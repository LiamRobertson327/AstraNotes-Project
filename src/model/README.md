Domain models

Files:
- `Note.h` / `Note.cpp`: Domain representation of a note. Holds metadata (id, title, typeId, created/modified timestamps) and encryption metadata when note is secured.
- `Snapshot.h` / `Snapshot.cpp`: Represents a point-in-time snapshot of a note's content and metadata used by the snapshot/versioning subsystem.

Notes:
- Models are plain data holders and should not perform persistence or UI operations.
