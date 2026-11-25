# Save compatibility

No engine or save schema yet. F001/F006 establish upstream layout, capacity and baseline. Before any persistent change record old/new version, affected data, compatibility, migration/rollback and tests. Never silently overwrite incompatible saves. Preserve backups and distinguish emulator save states from game saves. Use isolated debug/Lab saves; Lab battles must not mutate campaign. Version scenario schemas separately. If compatibility cannot be retained, request approval and clearly document restart requirements before merging. No save binaries committed.

