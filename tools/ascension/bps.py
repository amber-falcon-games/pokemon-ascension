#!/usr/bin/env python3
"""Create, apply and verify BPS patches (byuu/Near's "BPS1" format).

Pokémon Ascension is distributed as a .bps patch against the player's own
Pokémon Emerald dump (decision D017). This tool has no dependencies beyond
the Python standard library so it runs on a stock Mac.

Usage:
  bps.py create SOURCE TARGET PATCH [--metadata TEXT]
  bps.py apply  SOURCE PATCH OUTPUT
  bps.py verify SOURCE TARGET PATCH       # apply and compare byte-for-byte

Format summary (all integers are BPS varints):
  "BPS1", source size, target size, metadata size, metadata,
  actions until the last 12 bytes, then CRC32 of source, target and patch
  (little-endian). Each action is ((length - 1) << 2) | kind:
    0 SourceRead  copy source[outputOffset:outputOffset+length]
    1 TargetRead  literal bytes follow
    2 SourceCopy  signed relative offset follows; copy from source
    3 TargetCopy  signed relative offset follows; copy from output so far
                  (may overlap the bytes being written, which encodes runs)
"""
import argparse
import sys
import zlib

MAGIC = b"BPS1"
SOURCE_READ, TARGET_READ, SOURCE_COPY, TARGET_COPY = range(4)
K = 16            # match key length in bytes
STEP = 2          # index every STEP-th source position (Thumb code is 2-byte aligned)
MIN_MATCH = 24    # shortest SourceCopy worth emitting
MIN_RUN = 16      # shortest run of one repeated byte worth a TargetCopy


class PatchError(Exception):
    pass


# ---------------------------------------------------------------- varints

def put_varint(out, value):
    while True:
        x = value & 0x7F
        value >>= 7
        if value == 0:
            out.append(0x80 | x)
            return
        out.append(x)
        value -= 1


def get_varint(buf, pos):
    value, shift = 0, 1
    while True:
        if pos >= len(buf):
            raise PatchError("truncated varint")
        x = buf[pos]
        pos += 1
        value += (x & 0x7F) * shift
        if x & 0x80:
            return value, pos
        shift <<= 7
        value += shift


def put_signed(out, delta):
    put_varint(out, (abs(delta) << 1) | (1 if delta < 0 else 0))


# ---------------------------------------------------------------- apply

def apply_patch(source, patch):
    if len(patch) < len(MAGIC) + 12 or patch[:4] != MAGIC:
        raise PatchError("not a BPS1 patch")
    body, footer = patch[:-12], patch[-12:]
    src_crc = int.from_bytes(footer[0:4], "little")
    tgt_crc = int.from_bytes(footer[4:8], "little")
    patch_crc = int.from_bytes(footer[8:12], "little")
    if zlib.crc32(patch[:-4]) != patch_crc:
        raise PatchError("patch is corrupt (patch checksum mismatch)")
    if zlib.crc32(source) != src_crc:
        raise PatchError("this patch is for a different base ROM (source checksum mismatch)")
    pos = 4
    src_size, pos = get_varint(body, pos)
    tgt_size, pos = get_varint(body, pos)
    meta_size, pos = get_varint(body, pos)
    pos += meta_size
    if len(source) != src_size:
        raise PatchError(f"source is {len(source)} bytes, patch expects {src_size}")
    out = bytearray(tgt_size)
    o = 0
    src_rel = 0
    tgt_rel = 0
    while pos < len(body):
        data, pos = get_varint(body, pos)
        kind, length = data & 3, (data >> 2) + 1
        if o + length > tgt_size:
            raise PatchError("action writes past the end of the target")
        if kind == SOURCE_READ:
            out[o:o + length] = source[o:o + length]
        elif kind == TARGET_READ:
            out[o:o + length] = body[pos:pos + length]
            pos += length
        else:
            raw, pos = get_varint(body, pos)
            delta = -(raw >> 1) if raw & 1 else (raw >> 1)
            if kind == SOURCE_COPY:
                src_rel += delta
                out[o:o + length] = source[src_rel:src_rel + length]
                src_rel += length
            else:
                tgt_rel += delta
                if tgt_rel + length <= o:
                    out[o:o + length] = out[tgt_rel:tgt_rel + length]
                else:                      # overlapping copy: byte by byte
                    for i in range(length):
                        out[o + i] = out[tgt_rel + i]
                tgt_rel += length
        o += length
    if o != tgt_size:
        raise PatchError("patch ended before the target was complete")
    if zlib.crc32(out) != tgt_crc:
        raise PatchError("result does not match the expected target checksum")
    return bytes(out)


# ---------------------------------------------------------------- create

def _match_len(a, ai, b, bi, limit):
    """Length of the common prefix of a[ai:] and b[bi:], at most limit."""
    n, step = 0, 64
    while n < limit:
        c = min(step, limit - n)
        if a[ai + n:ai + n + c] == b[bi + n:bi + n + c]:
            n += c
            step = min(step * 2, 1 << 20)
        elif c == 1:
            break
        else:
            step = c // 2
    return n


def create_patch(source, target, metadata=b""):
    src, tgt = memoryview(source), memoryview(target)
    index = {}
    for i in range(0, len(source) - K + 1, STEP):
        index.setdefault(bytes(src[i:i + K]), i)

    out = bytearray(MAGIC)
    put_varint(out, len(source))
    put_varint(out, len(target))
    put_varint(out, len(metadata))
    out += metadata

    pending = bytearray()           # literal bytes waiting for a TargetRead
    src_rel = 0
    tgt_rel = 0
    o = 0                            # output offset

    def flush():
        if pending:
            put_varint(out, ((len(pending) - 1) << 2) | TARGET_READ)
            out.extend(pending)
            pending.clear()

    n = len(target)
    while o < n:
        remaining = n - o
        # 1. Same bytes at the same offset in the source: SourceRead.
        if o < len(source):
            m = _match_len(tgt, o, src, o, min(remaining, len(source) - o))
            if m >= MIN_MATCH or (m == remaining and m > 0):
                flush()
                put_varint(out, ((m - 1) << 2) | SOURCE_READ)
                o += m
                continue
        # 2. A run of one repeated byte: one literal, then an overlapping TargetCopy.
        if remaining >= MIN_RUN and o > 0 and target[o] == target[o - 1]:
            r = _match_len(tgt, o, tgt, o - 1, remaining)
            if r >= MIN_RUN:
                flush()
                put_varint(out, ((r - 1) << 2) | TARGET_COPY)
                put_signed(out, (o - 1) - tgt_rel)
                tgt_rel = o - 1 + r
                o += r
                continue
        # 3. The same bytes somewhere else in the source: SourceCopy.
        if remaining >= K:
            s = index.get(bytes(tgt[o:o + K]))
            if s is not None:
                m = _match_len(tgt, o, src, s, min(remaining, len(source) - s))
                if m >= MIN_MATCH:
                    flush()
                    put_varint(out, ((m - 1) << 2) | SOURCE_COPY)
                    put_signed(out, s - src_rel)
                    src_rel = s + m
                    o += m
                    continue
        pending.append(target[o])
        o += 1
    flush()

    out += zlib.crc32(source).to_bytes(4, "little")
    out += zlib.crc32(target).to_bytes(4, "little")
    out += zlib.crc32(out).to_bytes(4, "little")
    return bytes(out)


# ---------------------------------------------------------------- CLI

def _read(path):
    with open(path, "rb") as f:
        return f.read()


def main(argv=None):
    p = argparse.ArgumentParser(description="Create, apply and verify BPS patches.")
    sub = p.add_subparsers(dest="cmd", required=True)
    c = sub.add_parser("create"); c.add_argument("source"); c.add_argument("target"); c.add_argument("patch"); c.add_argument("--metadata", default="")
    a = sub.add_parser("apply"); a.add_argument("source"); a.add_argument("patch"); a.add_argument("output")
    v = sub.add_parser("verify"); v.add_argument("source"); v.add_argument("target"); v.add_argument("patch")
    args = p.parse_args(argv)
    try:
        if args.cmd == "create":
            patch = create_patch(_read(args.source), _read(args.target), args.metadata.encode("utf-8"))
            with open(args.patch, "wb") as f:
                f.write(patch)
            print(f"wrote {args.patch} ({len(patch):,} bytes)")
        elif args.cmd == "apply":
            result = apply_patch(_read(args.source), _read(args.patch))
            with open(args.output, "wb") as f:
                f.write(result)
            print(f"wrote {args.output} ({len(result):,} bytes)")
        else:
            result = apply_patch(_read(args.source), _read(args.patch))
            if result != _read(args.target):
                raise PatchError("applying the patch does not reproduce the target")
            print("ok: patch reproduces the target exactly")
    except (PatchError, OSError) as e:
        print(f"error: {e}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
