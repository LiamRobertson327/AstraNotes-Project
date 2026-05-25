Crypto utilities

Purpose
 - Provide a small, auditable set of cryptographic utilities used by the repository and snapshot services to encrypt and decrypt note contents and snapshots.

Files and responsibilities
 - `EncryptionService.h` / `EncryptionService.cpp` — stateless helper functions that provide: key derivation (Argon2id), authenticated encryption (AES-256-GCM) and helper functions for serializing the encrypted blob (version tags, salt, nonce, tag). This module hides OpenSSL details from higher layers.

Design considerations and guarantees
 - Keep crypto operations isolated to this folder to enable security audits and algorithm upgrades without touching business logic.
 - The service exposes a clear wire-format for encrypted payloads (version byte, salt, nonce, ciphertext, auth-tag). The repository stores the serialized blob along with metadata (algorithm version) so upgrades can be implemented safely.

Testing and migration notes
 - Unit tests should exercise happy-path encryption/decryption, wrong-password failures, and detect changes in the wire-format.
 - When rotating algorithms or parameters, provide migration helpers and ensure old payloads remain readable unless deliberately retired.

Where used
 - `src/repository/SqliteNoteRepository` calls into `EncryptionService` when persisting or reading secured notes and snapshots.

