Crypto utilities

Purpose: provide encryption/decryption helpers used by the repository and snapshot services.

Files:
- `EncryptionService.h` / `EncryptionService.cpp` — stateless helper functions for symmetric encryption used by snapshots and secured notes. Implements Argon2id-based key derivation and AES-256-GCM encryption/decryption using OpenSSL.

Notes:
- Keep cryptographic operations isolated to this folder to make audits and updates easier.
- Unit tests should exercise decryption failure modes (wrong password) and ensure backwards compatibility with stored formats.

