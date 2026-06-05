# AstraNotes Customer Acceptance Document

**Purpose**: Provide a simplified, shareable Definition of Done (DoD) summary for each user story, plus a formal sign-off section for customer acceptance.

**Use**: Review this document at the end of each story or sprint to confirm that the work is complete, testable, and acceptable for release.

> **After Project Completion Note**: While each story has an acceptance sign-off, this was added to show how a Definition of Done document would be used.  Tracking of which user stories and requiremtns were complete was done in another document and used the DoD criteria here.

---

## Common Definition of Done

A story is considered done when all of the following are true:
- The implemented behavior matches the user story and acceptance criteria.
- Required tests pass and evidence is captured.
- No blocking defects remain for the story scope.
- Security, reliability, and performance requirements in scope are addressed.
- The behavior can be demonstrated to the customer.

---

## Story 1: Create and Edit Notes with Save Options

**Simplified DoD Summary**
- Notes can be created, edited, manually saved, and autosaved.
- Title and body validation is enforced.
- Save operations are transactional and errors are handled without crashes.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 2: Support Different Note Formats via Plugins

**Simplified DoD Summary**
- Plugins load dynamically and expose supported formats.
- Notes can be created in supported formats.
- Invalid or missing plugins are handled gracefully.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 3: Search Within Current Open Note

**Simplified DoD Summary**
- Users can search within the open note.
- Matches are highlighted in real time.
- Search does not interrupt editing or saving.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 4: Search Across All Saved Notes

**Simplified DoD Summary**
- Users can search all saved notes by title/header data.
- Search uses indexed storage and returns results quickly.
- Search respects private-note visibility rules.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 5: Persistent Storage of Notes

**Simplified DoD Summary**
- Notes persist across sessions in SQLite with WAL mode.
- Data remains consistent after normal shutdown or crash recovery.
- Repository access is transactional and reliable.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 6: Encrypt Private Notes

**Simplified DoD Summary**
- Private notes encrypt and decrypt correctly.
- Password is required before private-note display.
- Encryption failures are handled without data loss or crashes.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 7: Note Metadata and Versioning with Snapshots

**Simplified DoD Summary**
- Notes store creation and last-modified metadata.
- Snapshots are created and limited to 2 per note.
- Users can restore a prior snapshot safely.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 8: Delete One or More Notes

**Simplified DoD Summary**
- Single and bulk deletion work as expected.
- Deleted notes move to Trash and can be restored.
- Delete operations are transactional.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 9: Plugin Validation and Missing-Plugin Fallback

**Simplified DoD Summary**
- Plugins are validated before use.
- Missing or invalid plugins do not crash the app.
- Notes affected by missing plugins are handled read-only or unsupported.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 10: In-Note Search with Match Count and Navigation

**Simplified DoD Summary**
- Search shows match counts and supports next/previous navigation.
- No-match state is clear to the user.
- Search remains responsive while editing.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 11: Crash Recovery with WAL and Durable Save

**Simplified DoD Summary**
- WAL-based recovery restores consistency after crash or power loss.
- Save completion only occurs after the durable write is complete.
- No note data is lost across restart scenarios.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 12: Encryption with Argon2id Key Derivation and IV Management

**Simplified DoD Summary**
- Encryption uses the specified Argon2id parameters and AES-256-GCM.
- A unique IV is generated per encryption and stored with the ciphertext.
- Decryption only works with the correct password.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 13: Trash with 14-Day Retention and Automatic Purge

**Simplified DoD Summary**
- Deleted notes move to Trash and remain recoverable for 14 days.
- Trash items are purged automatically after the retention window.
- Purge and restore operations are transactional.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 14: Automatic Snapshot Creation on Save

**Simplified DoD Summary**
- Snapshots are created on save, not every keystroke.
- Only two snapshots are kept per note.
- Restoring a snapshot creates a safety snapshot first.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 15: Graceful Shutdown with Unsaved-Edit Recovery

**Simplified DoD Summary**
- Exit prompts handle Save, Discard, or Cancel.
- Unexpected shutdown preserves recoverable edits for restart.
- Recovery and shutdown flows do not corrupt note data.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 16: Immutable Audit Log for Note Operations

**Simplified DoD Summary**
- Note operations are written to an append-only audit log.
- Audit entries cannot be altered or deleted.
- Sensitive data is redacted from log content.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 17: Database Security and SQL Injection Prevention

**Simplified DoD Summary**
- Database access uses prepared statements and bound parameters.
- Input validation rejects unsafe content such as null bytes.
- Database storage and permissions are restricted appropriately.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Story 18: Consistent Error Handling and User Notification Contract

**Simplified DoD Summary**
- Errors are logged with full detail for developers.
- Users receive non-blocking, friendly notifications.
- The app preserves state and allows retry or continuation.

**Customer Acceptance Sign-Off**
- Accepted by: ____________________
- Date: ____________________
- Notes: ____________________
- Status: [ ] Accepted  [ ] Accepted with comments  [ ] Rejected

---

## Final Acceptance Summary

**Overall Release Status**
- Release candidate: ____________________
- Reviewer: ____________________
- Date: ____________________
- Decision: [ ] Go  [ ] No-Go
- Final notes: ____________________
