# AstraNotes Release Gates

**Purpose**: Define the go/no-go criteria that must be satisfied before a sprint or release can be considered ready.

Release gates connect the planned work to the actual evidence collected during implementation.

---

## 1. Gate Categories

### Functional Gate
- Core user-facing stories in the current sprint are implemented.
- Acceptance criteria for those stories are met.
- No open critical defects remain for the sprint scope.

### Quality Gate
- Required unit and integration tests pass.
- Required UI tests pass, if applicable.
- Performance checks are within expected limits or documented with exceptions.

### Security Gate
- Encryption, audit logging, and input validation requirements are satisfied where in scope.
- No high-risk security issues remain open.

### Reliability Gate
- Crash recovery and graceful shutdown behavior are validated where in scope.
- No data-loss defects remain open.

### Traceability Gate
- Work can be traced from requirement to user story to backlog to sprint to test evidence.
- Execution evidence is recorded in `docs/EXECUTION_EVIDENCE.md` or equivalent artifacts.

---

## 2. Go / No-Go Checklist

A release is a **Go** only when all of the following are true:

- [ ] Required stories are complete for the release scope
- [ ] Acceptance criteria are met
- [ ] Test plan execution is complete
- [ ] Evidence is captured
- [ ] No blocking defects remain
- [ ] Security-sensitive requirements are verified
- [ ] Performance requirements are acceptable or documented
- [ ] Traceability chain is intact

If any required item is not satisfied, the release is **No-Go** until the issue is resolved or explicitly deferred.

---

## 3. Example Release Decision Record

| Release | Date | Decision | Reason |
|---------|------|----------|--------|
| Sprint 1 | YYYY-MM-DD | Go / No-Go | Summary of gate status |

---

## 4. Recommended Release Process

1. Confirm the sprint scope.
2. Review completed stories and acceptance criteria.
3. Review test results and execution evidence.
4. Check for unresolved critical defects.
5. Record the final release decision.

---

## 5. Notes

This document should be updated at the end of each sprint or before any release decision is made.
