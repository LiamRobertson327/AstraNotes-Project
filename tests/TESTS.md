# AstraNotes Test Inventory

This document describes the automated test artifacts in `tests/` and the new tests added in this update.

## Smoke Tests

### `SmokeTest.cpp`
Small end-to-end checks for the core app plumbing.
- `testEncryptionRoundtrip` - encrypts and decrypts a payload through `EncryptionService`.
- `testRepositorySaveLoad` - saves a note to SQLite and loads it back by id.
- `testSnapshotAndTrash` - creates a snapshot, trashes the note, and purges trashed rows.

### `mainwindow_quick_tests.cpp` (new)
Quick UI smoke coverage for the split search bars.
- `searchBarsHaveIndependentState` - verifies the content search and saved-notes search have different placeholders and independent text state.
- `enterAdvancesToNextContentMatch` - verifies Enter moves the content search to the next highlighted match.

### `trash_feature_smoke.cpp`
Smoke coverage for the trash flow using the service layer.
- `create_trash_restore_purge_flow` - saves a note, trashes it, restores it, then purges it.

## Unit Tests

### `note_service_trash_unit_tests.cpp`
Verifies `NoteService` forwards trash requests correctly.
- `trashNote_callsRepository` - confirms the service passes the note id to the repository mock.

### `snapshot_service_tests.cpp`
Verifies snapshot lifecycle behavior at the service layer.
- `saveAndPruneSnapshots` - saves multiple snapshots and prunes them to the configured limit.

### `trash_service_tests.cpp`
Verifies trash lifecycle behavior at the service layer.
- `trashRestoreAndPurge` - trashes a note, restores it, then purges it.

### `note_service_password_unit_tests.cpp` (new)
Adds coverage for `NoteService::saveNote()` behavior.
- `saveNote_usesPasswordOnlyForSecuredNotes` - unsecured notes do not forward a password; secured notes do.
- `saveNote_reportsMissingRepository` - returns a clear error when the repository is unavailable.

## Integration Tests

### `note_trash_integration_tests.cpp`
Exercises the service and repository together.
- `trashNote_movesToTrash` - confirms trashing changes active-note counts and the trashed listing.

### `repository_roundtrip_integration_tests.cpp` (new)
Exercises repository behavior across counts, search, and snapshots.
- `titleSearchCountsAndTypes_roundTrip` - saves notes, checks active counts, type counts, title counts, and paged search results.
- `snapshotRoundTrip_preservesLatestContent` - saves two snapshots, verifies newest-first retrieval, and reloads a snapshot by id.

## Performance Tests

### `nfr_performance_tests.cpp` (new)
Benchmarks the documented NFR1/NFR2 targets against a 10,000-note dataset.
- `saveLatency_under50ms` - verifies save latency stays under 50 ms.
- `loadLatency_under100ms` - verifies load latency stays under 100 ms.
- `titleSearchLatency_under200ms` - verifies title search counting stays under 200 ms.
- `firstPageLoadLatency_under500ms` - verifies first-page pagination stays under 500 ms.

## Test Runner

### `scripts/run-all-tests.ps1`
Runs registered tests through CTest when available.
- Because the new integration tests are added with `add_test()` in `tests/CMakeLists.txt`, the script includes them automatically when it runs CTest.
- In fallback mode, it scans for test executables under the build tree, so executables like `repository_roundtrip_integration_tests.exe` are also picked up.

## Notes

- All tests are discovered through `tests/CMakeLists.txt`.
- Smoke, unit, and integration tests are compiled as separate executables to keep failure scope narrow.
- The test database work uses either `:memory:` or `QTemporaryDir` so the tests stay isolated and repeatable.