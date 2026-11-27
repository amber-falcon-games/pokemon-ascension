#!/usr/bin/env python3
"""Talent trees (F015, D027): generate, check and compile.

  talent_trees.py generate   writes data/ascension/talent_trees.json from upstream level-up learnsets
  talent_trees.py check      validates the JSON (owner edits welcome)
  talent_trees.py compile    writes src/data/ascension/talent_trees.h (ROM tables) from the JSON

Model: each species has up to 8 branches. A branch is a chain of nodes; a node unlocks at a level
and holds one move, or 2-4 moves to choose from (a branch point). A later node replaces the
earlier one. "global" chains (one per type and category) hold every move, for taught moves.
"""
import json, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
LEARNSETS = os.path.join(ROOT, "src/data/pokemon/level_up_learnsets/gen_9.h")
SPECIES_DIR = os.path.join(ROOT, "src/data/pokemon/species_info")
MOVES = os.path.join(ROOT, "src/data/moves_info.h")
JSON_OUT = os.path.join(ROOT, "data/ascension/talent_trees.json")
H_OUT = os.path.join(ROOT, "src/data/ascension/talent_trees.h")
MAX_BRANCHES = 8
MAX_NODES = 6
MAX_CHOICES = 4
CHOICE_WINDOW = 3   # moves of one chain unlocking within this many levels become a choice


def parse_learnsets():
    text = open(LEARNSETS).read()
    sets = {}
    for m in re.finditer(r"static const struct LevelUpMove (\w+)\[\] = \{(.*?)LEVEL_UP_END", text, re.S):
        sets[m.group(1)] = [(int(l), mv) for l, mv in re.findall(r"LEVEL_UP_MOVE\(\s*(\d+),\s*(MOVE_\w+)\)", m.group(2))]
    return sets


def parse_species():
    species = {}
    for fn in sorted(os.listdir(SPECIES_DIR)):
        if not fn.endswith(".h"):
            continue
        text = open(os.path.join(SPECIES_DIR, fn)).read()
        blocks = re.split(r"\n    \[(SPECIES_\w+)\]\s*=", text)
        for i in range(1, len(blocks), 2):
            name, body = blocks[i], blocks[i + 1]
            ls = re.search(r"\.levelUpLearnset\s*=\s*(\w+)", body)
            evo = re.search(r"\.evolutions\s*=\s*EVOLUTION\((.*?)\),\n", body, re.S)
            evos = []   # (target, level or None)
            if evo:
                for method, param, target in re.findall(r"\{\s*(EVO_\w+)\s*,\s*([^,]+?)\s*,\s*(SPECIES_\w+)", evo.group(1)):
                    lvl = int(param) if method == "EVO_LEVEL" and param.strip().isdigit() else None
                    evos.append((target, lvl))
            if ls:
                species[name] = {"learnset": ls.group(1), "evolves_to": evos}
    return species


def parse_moves():
    text = open(MOVES).read()
    moves = {}
    for m in re.finditer(r"\n    \[(MOVE_\w+)\] =\s*\{(.*?)\n    \},", text, re.S):
        body = m.group(2)
        t = re.search(r"\.type\s*=\s*(TYPE_\w+)", body)
        c = re.search(r"\.category\s*=\s*(DAMAGE_CATEGORY_\w+)", body)
        p = re.search(r"\.power\s*=\s*([^,\n]+)", body)
        power = 0
        if p:
            nums = re.findall(r"\d+", p.group(1).split("?")[1] if "?" in p.group(1) else p.group(1))
            power = int(nums[0]) if nums else 0
        if t and c:
            moves[m.group(1)] = {"type": t.group(1), "category": c.group(1), "power": power}
    return moves


def chain_key(move, moves):
    info = moves[move]
    cat = {"DAMAGE_CATEGORY_PHYSICAL": "PHYSICAL", "DAMAGE_CATEGORY_SPECIAL": "SPECIAL"}.get(info["category"], "STATUS")
    return info["type"][5:] + "_" + cat


def build_branches(learned, moves):
    """learned: [(level, move)] for one species including its pre-evolutions (earliest level kept)."""
    first = {}
    for lvl, mv in learned:
        if mv in moves and mv != "MOVE_NONE":
            first[mv] = min(first.get(mv, 999), max(lvl, 1))
    groups = {}
    for mv, lvl in first.items():
        groups.setdefault(chain_key(mv, moves), []).append((lvl, mv))
    branches = []
    for key, items in groups.items():
        items.sort(key=lambda x: (x[0], moves[x[1]]["power"]))
        nodes = []
        for lvl, mv in items:
            if nodes and lvl - nodes[-1]["level"] <= CHOICE_WINDOW and len(nodes[-1]["moves"]) < MAX_CHOICES:
                nodes[-1]["moves"].append(mv)
            else:
                nodes.append({"level": lvl, "moves": [mv]})
        nodes = nodes[-MAX_NODES:] if len(nodes) > MAX_NODES else nodes
        nodes[0]["level"] = min(nodes[0]["level"], items[0][0])
        damaging = not key.endswith("STATUS")
        score = (damaging, len(nodes), max(moves[m]["power"] for n in nodes for m in n["moves"]))
        branches.append((score, {"name": key.replace("_", " ").title(), "nodes": nodes}))
    branches.sort(key=lambda b: b[0], reverse=True)
    # keep at most 2 status branches, then the best damaging ones, up to MAX_BRANCHES
    status = [b for s, b in branches if not s[0]][:2]
    damaging = [b for s, b in branches if s[0]][:MAX_BRANCHES - len(status)]
    return damaging + status


def write_json(data, path):
    """Readable layout for hand edits: one line per branch and per global chain."""
    out = ['{', f' "version": {data["version"]},', f' "note": {json.dumps(data["note"])},', ' "global": {']
    g = list(data["global"].items())
    for i, (k, v) in enumerate(g):
        out.append(f'  {json.dumps(k)}: {json.dumps(v)}' + (',' if i < len(g) - 1 else ''))
    out += [' },', ' "species": {']
    sp = list(data["species"].items())
    for i, (k, tree) in enumerate(sp):
        out.append(f'  {json.dumps(k)}: {{"branches": [')
        br = tree["branches"]
        for j, b in enumerate(br):
            out.append('   ' + json.dumps(b, separators=(", ", ": ")) + (',' if j < len(br) - 1 else ''))
        out.append('  ]}' + (',' if i < len(sp) - 1 else ''))
    out += [' }', '}']
    with open(path, "w") as f:
        f.write("\n".join(out) + "\n")


def generate():
    sets, species, moves = parse_learnsets(), parse_species(), parse_moves()
    pre = {}   # species -> (pre-evolution, evolution level or None)
    for sp, info in species.items():
        for evo, lvl in info["evolves_to"]:
            pre.setdefault(evo, (sp, lvl))
    lines = {}
    for sp, info in species.items():
        chain, cur, seen = [], sp, set()
        while cur and cur not in seen and cur in species:
            seen.add(cur); chain.append(cur); cur = pre.get(cur, (None, None))[0]
        chain.reverse()   # root first
        learned, known = [], set()
        for idx, member in enumerate(chain):
            evo_level = pre.get(member, (None, None))[1] if idx > 0 else None
            for lvl, mv in sets.get(species[member]["learnset"], []):
                # An evolved stage's level-0/1 moves are its relearn list: new ones unlock when it
                # evolves (at the evolution level, where there is one), not from level 1.
                if idx > 0 and lvl <= 1 and mv not in known:
                    lvl = evo_level or 1
                learned.append((lvl, mv))
            known |= {mv for _, mv in sets.get(species[member]["learnset"], [])}
        if learned:
            lines[sp] = {"branches": build_branches(learned, moves)}
    glob = {}
    for mv, info in moves.items():
        if mv == "MOVE_NONE" or (info["power"] == 0 and info["category"] != "DAMAGE_CATEGORY_STATUS"):
            continue
        glob.setdefault(chain_key(mv, moves), []).append(mv)
    for key in glob:
        glob[key].sort(key=lambda m: (moves[m]["power"], m))
    data = {"version": 1,
            "note": "Talent trees (F015, D027). Generated by tools/ascension/talent_trees.py generate; edit freely, then run check.",
            "global": dict(sorted(glob.items())), "species": dict(sorted(lines.items()))}
    os.makedirs(os.path.dirname(JSON_OUT), exist_ok=True)
    write_json(data, JSON_OUT)
    print(f"wrote {JSON_OUT}: {len(lines)} species, {len(glob)} global chains")


def check(path=JSON_OUT):
    moves = parse_moves()
    data = json.load(open(path))
    errors = []
    for sp, tree in data["species"].items():
        br = tree["branches"]
        if len(br) > MAX_BRANCHES:
            errors.append(f"{sp}: {len(br)} branches (max {MAX_BRANCHES})")
        for b in br:
            last = 0
            if len(b["nodes"]) > MAX_NODES:
                errors.append(f"{sp} {b['name']}: {len(b['nodes'])} nodes (max {MAX_NODES})")
            for n in b["nodes"]:
                if not 1 <= n["level"] <= 100 or n["level"] < last:
                    errors.append(f"{sp} {b['name']}: level {n['level']} out of order")
                last = n["level"]
                if not 1 <= len(n["moves"]) <= MAX_CHOICES:
                    errors.append(f"{sp} {b['name']}: a node needs 1-{MAX_CHOICES} moves")
                for mv in n["moves"]:
                    if mv not in moves:
                        errors.append(f"{sp} {b['name']}: unknown move {mv}")
    for key, chain in data["global"].items():
        for mv in chain:
            if mv not in moves:
                errors.append(f"global {key}: unknown move {mv}")
    for e in errors[:50]:
        print("error:", e)
    print(f"{'FAIL' if errors else 'ok'}: {len(data['species'])} species, {len(errors)} error(s)")
    return not errors


def compile_header(path=JSON_OUT, out=H_OUT):
    """ROM tables: per species, its branches; per branch, its nodes (level, up to 4 moves)."""
    data = json.load(open(path))
    lines = ["// Generated by tools/ascension/talent_trees.py compile from data/ascension/talent_trees.json.",
             "// Do not edit: edit the JSON and rebuild (F015, D027).", ""]
    tables = []
    for sp, tree in data["species"].items():
        sym = "sTalent_" + sp[len("SPECIES_"):]
        for bi, b in enumerate(tree["branches"]):
            nodes = ", ".join("{%d, %d, {%s}}" % (n["level"], len(n["moves"]), ", ".join(n["moves"])) for n in b["nodes"])
            lines.append("static const struct TalentNode %s_%d[] = {%s};" % (sym, bi, nodes))
        branches = ", ".join("{%d, %s_%d}" % (len(b["nodes"]), sym, bi) for bi, b in enumerate(tree["branches"]))
        lines.append("static const struct TalentBranch %s[] = {%s};" % (sym, branches))
        tables.append("    [%s] = {%d, %s}," % (sp, len(tree["branches"]), sym))
    lines += ["", "const struct TalentTree gTalentTrees[NUM_SPECIES] =", "{"] + tables + ["};", ""]
    for key, chain in data["global"].items():
        lines.append("static const u16 sTalentGlobal_%s[] = {%s, MOVE_NONE};" % (key, ", ".join(chain)))
    lines += ["", "const u16 *const gTalentGlobalChains[] =", "{"]
    lines += ["    sTalentGlobal_%s," % key for key in data["global"]]
    lines += ["    NULL,", "};", ""]
    os.makedirs(os.path.dirname(out), exist_ok=True)
    text = "\n".join(lines)
    if not os.path.exists(out) or open(out).read() != text:
        open(out, "w").write(text)
    print(f"wrote {out}: {len(tables)} species, {len(data['global'])} global chains")


if __name__ == "__main__":
    cmd = sys.argv[1] if len(sys.argv) > 1 else "check"
    if cmd == "generate":
        generate()
    elif cmd == "compile":
        compile_header(*(sys.argv[2:4]))
    elif cmd == "check":
        sys.exit(0 if check(sys.argv[2] if len(sys.argv) > 2 else JSON_OUT) else 1)
    else:
        sys.exit("usage: talent_trees.py generate|check [file]|compile [json] [header]")
