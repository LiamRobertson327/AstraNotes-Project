# AstraNotes Test Plan

**Purpose**: Define the test coverage that validates the requirements, user stories, backlog items, and sprint work before release.

**Scope**: This plan covers unit tests, integration tests, UI tests, performance tests, and recovery tests for the current AstraNotes implementation path.

---

## 1. Test Objectives

- Verify that refined requirements are implemented correctly.
- Verify that user stories and acceptance criteria map to executable tests.
- Verify that sprint backlog items produce the expected behavior.
- Verify that core quality gates are satisfied before release.

---

## 2. Test Coverage Map

| Area | What Will Be Tested | Primary Source |
|------|---------------------|----------------|
| Requirements | Functional, non-functional, and security behavior | `docs/INITIAL_REQUIREMENTS_REVISED.md` |
| User Stories | Acceptance criteria and story behavior | `docs/User_stories.md` |
| Backlog | Priority-driven implementation scope | `docs/BACKLOG.md` |
| Sprint Plan | Work executed per sprint | `docs/SPRINTS_IMPLEMENTATION_PLAN.md` |
| UML Traceability | Requirement-to-design alignment | `docs/REQUIREMENT_TRACEABILITY_MATRIX.md` |

---

## 3. Test Types

### 3.1 Unit Tests
- Note model validation
- Repository operations
- Plugin interface behavior
- Encryption and decryption helpers
- Cache invalidation logic
- Trash state transitions

### 3.2 Integration Tests
- SQLite persistence and recovery
- Plugin load/save/open workflows
- Snapshot creation and restore behavior
- Trash retention and purge flows
- Crash recovery and graceful shutdown

### 3.3 UI Tests
- Create, edit, save, and autosave flows
- In-note search and global search UI
- Password prompt flow for private notes
- Delete, restore, and trash navigation
- Exit prompt for unsaved changes

### 3.4 Performance Tests
- Save latency
- Load latency
- Search latency for 10,000 notes
- Pagination response time
- Encryption latency

### 3.5 Security Tests
- SQL injection prevention
- Encryption round-trip validation
- IV generation and password-gated decryption
- Audit log redaction and immutability

---

## 4. Requirements-to-Test Trace Examples

| Requirement / Story | Example Test Coverage |
|---------------------|-----------------------|
| FR1 / Story 1 | Create note, edit note, manual save, autosave, save error handling |
| FR2 / Story 2, 9 | Plugin discovery, format selection, missing-plugin fallback |
| FR5 / Story 11 | WAL persistence, crash recovery, save completion durability |
| FR6 / Story 6, 12 | AES-256-GCM encryption, Argon2id, IV handling, password prompt |
| FR8 / Story 7, 14 | Snapshot rotation, restore safety snapshot, FIFO purge |
| FR9 / Story 8, 13 | Trash workflow, restore, retention, auto-purge |
| NFR1 / Story 1, 4, 5 | Save/load/search/pagination timing benchmarks |
| NFR3 / Story 11, 15 | Crash recovery, power-loss recovery, graceful shutdown |
| SR2 / Story 16 | Immutable audit log creation and redaction |
| SR3 / Story 17 | Prepared statements and database hardening |
| SR4 / Story 18 | Error logging, user notification, state preservation |

---

## 5. Exit Criteria

Release testing is considered complete when:

- All required unit tests pass.
- All integration tests for the current sprint pass.
- Performance targets are verified or documented with exceptions.
- Security-critical behaviors are confirmed.
- No blocking crashes or data-loss issues remain.
- Release gate criteria in `docs/RELEASE_GATES.md` are satisfied.

---

## 6. Evidence to Capture

During execution, capture:
- Test run output
- Build output
- Screenshots of UI flows where relevant
- Logs for crash recovery or error handling tests
- Benchmark results for performance tests

This evidence should be saved in `docs/EXECUTION_EVIDENCE.md` or attached to the implementation record.
