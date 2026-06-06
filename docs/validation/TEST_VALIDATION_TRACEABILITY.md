# AstraNotes Test Validation Traceability
This document maps product requirements to the automated tests that validate them, and identifies remaining coverage gaps.
Test inventory source: `tests/TESTS.md` plus the active test targets under `tests/`.
Primary requirement source: `docs/requirements/INITIAL_REQUIREMENTS_REVISED.md`.

| Req ID | Requirement Summary | Automated Test Evidence | Coverage Status | Remaining Validation Target |

| FR1 | Create/edit notes, manual save, autosave behavior, save constraints | `tests/unit/note_service_password_unit_tests.cpp`, `tests/unit/repository_failure_tests.cpp`, `tests/integration/repository_roundtrip_integration_tests.cpp`, `tests/smoke/mainwindow_quick_tests.cpp` | Good | Add explicit autosave debounce and unsaved-indicator UI tests. |

| FR2 | Plugin-driven note formats and format-specific serialization | `tests/smoke/SmokeTest.cpp` (encryption/roundtrip plumbing), `tests/integration/repository_roundtrip_integration_tests.cpp` (type/count behavior) | Good | Add dedicated plugin discovery/validation fallback tests (missing plugin, disabled plugin, bad plugin metadata). |

| FR3 | In-note search/highlighting/navigation | `tests/smoke/mainwindow_quick_tests.cpp`, `tests/unit/search_edgecases_tests.cpp` | Good | Add wrap-around and empty-query highlight-clear tests at the UI level. |

| FR4 | Search all notes by title using indexed DB path | `tests/integration/repository_roundtrip_integration_tests.cpp`, `tests/performance/nfr_performance_tests.cpp`, `tests/unit/search_edgecases_tests.cpp` | Good | Add an index-presence assertion for schema migration safety. |

| FR5 | Durable SQLite persistence and recovery | `tests/smoke/SmokeTest.cpp`, `tests/integration/repository_roundtrip_integration_tests.cpp`, `tests/unit/repository_failure_tests.cpp` | Good | Add crash and restart recovery tests that verify WAL rollback and consistency after interruption. |

| FR6 | Encryption/decryption workflow with failure handling | `tests/smoke/SmokeTest.cpp`, `tests/unit/note_service_password_unit_tests.cpp`, `tests/unit/encryption_failure_tests.cpp` | Good | Add corruption-path tests for tampered ciphertext, salt, IV, and tag values. |

| FR7 | Metadata persistence/display (created, modified, type) | `tests/integration/repository_roundtrip_integration_tests.cpp`, `tests/smoke/mainwindow_quick_tests.cpp` | Good | Add explicit assertions for createdAt, lastModified, and display formatting invariants. |

| FR8 | Snapshot lifecycle, max-retention, and restore/delete flow | `tests/unit/snapshot_service_tests.cpp`, `tests/integration/repository_roundtrip_integration_tests.cpp`, `tests/smoke/SmokeTest.cpp` | Good | Add a UI-level revert workflow test, including safety snapshot creation before revert. |

| FR9 | Trash, restore, and purge lifecycle | `tests/unit/note_service_trash_unit_tests.cpp`, `tests/unit/trash_service_tests.cpp`, `tests/unit/note_deletion_lifecycle_tests.cpp`, `tests/integration/note_trash_integration_tests.cpp`, `tests/smoke/trash_feature_smoke.cpp` | Good | Add bulk-delete and retention-window boundary tests for 13, 14, and 15 day behavior. |

| NFR1 | Performance targets for save, load, search, and pagination | `tests/performance/nfr_performance_tests.cpp` | Good | Add regression guardrails that compare trends across CI runs. |

| NFR2 | 10k-note scalability with indexed search and on-demand loading | `tests/performance/nfr_performance_tests.cpp`, `tests/integration/repository_roundtrip_integration_tests.cpp` | Good | Add a memory-footprint assertion during 10k search and open workflows. |

| NFR3 | Reliability, crash recovery, and graceful shutdown | Indirect coverage only (normal flow tests) | Gap | Add fault-injection tests for interrupted writes and startup recovery prompts. |

## Unit Test File Purposes

- `tests/unit/repository_failure_tests.cpp`
  - Verifies service behavior when repository operations fail or are unavailable.
- `tests/unit/encryption_failure_tests.cpp`
  - Verifies the wrong-password path surfaces correctly.
- `tests/unit/concurrency_tests.cpp`
  - Verifies concurrent save behavior with a thread-safe fake repository.
- `tests/unit/search_edgecases_tests.cpp`
  - Verifies unicode, emoji, and long-content search paths.
- `tests/unit/encryption_failure_tests.cpp`
  - Verifies bad passwords are blocked and can't decrypt the note
- `tests/unit/note_deletion_lifecycle_tests.cpp`
  - Verifies `std::unique_ptr` ownership destroys note objects deterministically.
- `tests/unit/search_edgecases_tests.cpp`
  - Verifies unicode/emoji and long-content search paths.

## Notes 

- Use this document as the validation companion to `docs REQUIREMENT_TRACEABILITY_MATRIX.md` and `tests/TESTS.md`.
- Update matrix rows whenever requirements change or tests are added or removed.

## Features Targeted by Remaining Tests

The highest-value remaining automated tests should target:

1. Crash/recovery reliability (`FR5`, `NFR3`) under forced process interruption.
2. Plugin failure and fallback paths (`FR2`) for malformed/missing format handlers.
3. UI search navigation boundaries (`FR3`) including wrap-around and highlight clearing.
4. Retention boundaries and bulk trash operations (`FR9`) for compliance with 14-day policy.
5. Metadata invariants (`FR7`) including immutable create timestamp and modified timestamp updates.

## Usage Notes

- Use this document as the validation companion to `docs/validation/REQUIREMENT_TRACEABILITY_MATRIX.md`.
- Update matrix rows whenever requirements change or tests are added/removed.
- For release readiness, all rows should be at least "Good" coverage or include an accepted gap note.  Rows with "Good" coverage may still benefit from more testing and validation.
- While automated tests may not fully cover every requirement and edge case, manual testing by using the application GUI will cover these gaps.  Regardless it is still optimal to have all tests cases covered to ensure the application functions properly when code changes are made.