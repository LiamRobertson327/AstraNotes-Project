# AstraNotes Test Validation Traceability

## Purpose
This document maps product requirements to the automated tests that validate them, and identifies remaining coverage gaps.

Primary requirement source: `docs/INITIAL_REQUIREMENTS_REVISED.md`.
Test inventory source: `tests/TESTS.md` plus active test targets in `tests/`.

## Requirement-to-Test Matrix

| Req ID | Requirement Summary | Automated Test Evidence | Coverage Status | Remaining Validation Target |
|---|---|---|---|---|
| FR1 | Create/edit notes, manual save, autosave behavior, save constraints | `tests/unit/note_service_password_unit_tests.cpp`, `tests/unit/repository_failure_tests.cpp`, `tests/integration/repository_roundtrip_integration_tests.cpp` | Partial | Add explicit autosave debounce and unsaved-indicator UI tests. |
| FR2 | Plugin-driven note formats and format-specific serialization | `tests/smoke/SmokeTest.cpp` (encryption/roundtrip plumbing), `tests/integration/repository_roundtrip_integration_tests.cpp` (type/count behavior) | Partial | Add dedicated plugin discovery/validation fallback tests (missing plugin, disabled plugin, bad plugin metadata). |
| FR3 | In-note search/highlighting/navigation | `tests/smoke/mainwindow_quick_tests.cpp`, `tests/unit/search_edgecases_tests.cpp` | Partial | Add explicit next/previous wrap-around and empty-query highlight-clear tests at UI level. |
| FR4 | Search all notes by title using indexed DB path | `tests/integration/repository_roundtrip_integration_tests.cpp`, `tests/performance/nfr_performance_tests.cpp`, `tests/unit/search_edgecases_tests.cpp` | Good | Add index-presence/assertion test for schema migration safety. |
| FR5 | Durable SQLite persistence and recovery | `tests/smoke/SmokeTest.cpp`, `tests/integration/repository_roundtrip_integration_tests.cpp` | Partial | Add crash/restart recovery tests that verify WAL rollback/consistency after forced interruption. |
| FR6 | Encryption/decryption workflow and failure handling | `tests/smoke/SmokeTest.cpp`, `tests/unit/note_service_password_unit_tests.cpp`, `tests/unit/encryption_failure_tests.cpp` | Good | Add corruption-path tests for tampered ciphertext/salt/IV/tag. |
| FR7 | Metadata persistence/display (created/modified/type) | `tests/integration/repository_roundtrip_integration_tests.cpp`, `tests/smoke/mainwindow_quick_tests.cpp` (indirect UI path) | Partial | Add explicit metadata assertions for createdAt/lastModified and display formatting invariants. |
| FR8 | Snapshot lifecycle, max-retention, restore/delete flow | `tests/unit/snapshot_service_tests.cpp`, `tests/integration/repository_roundtrip_integration_tests.cpp`, `tests/smoke/SmokeTest.cpp` | Good | Add UI-level revert workflow test (including safety snapshot creation before revert). |
| FR9 | Trash/delete/restore/purge lifecycle | `tests/unit/note_service_trash_unit_tests.cpp`, `tests/unit/trash_service_tests.cpp`, `tests/unit/note_deletion_lifecycle_tests.cpp`, `tests/integration/note_trash_integration_tests.cpp`, `tests/smoke/trash_feature_smoke.cpp` | Good | Add bulk-delete and retention-window boundary tests (13/14/15 day behavior). |
| NFR1 | Performance targets for save/load/search/pagination | `tests/performance/nfr_performance_tests.cpp` | Good | Add trend regression guardrails (baseline comparison across CI runs). |
| NFR2 | 10k-note scalability with indexed search/on-demand loading | `tests/performance/nfr_performance_tests.cpp`, `tests/integration/repository_roundtrip_integration_tests.cpp` | Good | Add memory-footprint assertion during 10k search/open workflows. |
| NFR3 | Reliability: crash recovery and graceful shutdown | Indirect coverage only (normal flow tests) | Gap | Add fault-injection tests for interrupted writes and startup recovery prompts. |

## New Edge-Case Tests Added

- `tests/unit/repository_failure_tests.cpp`
  - Verifies service behavior when repository operations fail/unavailable.
- `tests/unit/encryption_failure_tests.cpp`
  - Verifies wrong-password path surfaces correctly.
- `tests/unit/concurrency_tests.cpp`
  - Verifies concurrent save path with thread-safe fake repository.
- `tests/unit/note_deletion_lifecycle_tests.cpp`
  - Verifies `std::unique_ptr` ownership destroys note objects deterministically.
- `tests/unit/search_edgecases_tests.cpp`
  - Verifies unicode/emoji and long-content search paths.

## Features Targeted by Remaining Tests

The highest-value remaining automated tests should target:

1. Crash/recovery reliability (`FR5`, `NFR3`) under forced process interruption.
2. Plugin failure and fallback paths (`FR2`) for malformed/missing format handlers.
3. UI search navigation boundaries (`FR3`) including wrap-around and highlight clearing.
4. Retention boundaries and bulk trash operations (`FR9`) for compliance with 14-day policy.
5. Metadata invariants (`FR7`) including immutable create timestamp and modified timestamp updates.

## Usage Notes

- Use this document as the validation companion to `docs/REQUIREMENT_TRACEABILITY_MATRIX.md`.
- Update matrix rows whenever requirements change or tests are added/removed.
- For release readiness, all rows should be at least "Good" coverage or include an accepted gap note.
