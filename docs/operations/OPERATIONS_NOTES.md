# AstraNotes Operations Notes

**Purpose**: Capture security, deployment, and maintenance notes that support final submission and future maintenance.

---

## Security Notes

- Notes are stored locally in SQLite.
- Encryption is local and uses AES-256-GCM with Argon2id key derivation.
- Audit logging is append-only and stored locally in the user profile data directory.
- Prepared statements should remain the default pattern for repository access.

## Deployment Notes

- The project is a desktop Qt application.
- Build and packaging should remain cross-platform where possible.
- CI workflows should continue to validate Windows, macOS, and Linux compatibility.
- Runtime dependencies should be resolved through CMake and the platform package manager or Qt deployment tools.

## Maintenance Notes

- Keep the README, validation docs, and architecture docs aligned with implementation changes.
- Preserve legacy docs when they explain the evolution of requirements or design decisions.
- Update the traceability matrix when tests are renamed, added, or removed.