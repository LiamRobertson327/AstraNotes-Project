## **Functional Requirements**
- The system must allow the user to create and edit notes with a manual save and auto-save feature, with all notes containing a title and body.
- The system must allow the user to create notes in different formats according to which plugins are currently installed that support those special formats.
- The system must allow the user to search the content in the note they currently have open.
- The system must allow the user to search the content or title for all the notes they have saved.
- The system must implement persistent storage so the user’s notes are not lost.
- The system must offer encryption of notes that the user marked as private.
- The system must attach metadata to each note, such as creation date and last modified date.
- The system must implement versioning, so the user can undo changes they made and optionally change a note to match a previously saved version if they want to revert multiple changes they made.  This system shall create a snapshot of a note when it is saved or edited to implement this.
- The system must allow the user to delete one or more notes at a time.

## **Non-Functional Requirements**
- The system must achieve fast performance, regardless of how many notes the user has.
- The system must be scalable and support at least 10,000 notes per user.
- The system must be reliable by implementing crash recovery and graceful shutdowns, so user notes are not lost.

## **Security, Privacy, Reliability, and Governance Requirements**
- Notes that the user marked as private shall be encrypted with AES-256-GCM.
- The system shall have immutable audit logs to track system operations and ensure compliance.
- The database shall implement methods to prevent SQL injection attacks.
- Errors when loading, saving, or creating notes shall be logged and reported to the user instead of the application crashing.
