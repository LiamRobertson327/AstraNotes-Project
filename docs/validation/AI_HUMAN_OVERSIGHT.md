# AstraNotes AI and Human Oversight

**Purpose**: Record how AI assistance was used during the project and which decisions were refined or rejected by human judgment.

---

## AI-Assisted Work

- Documentation restructuring and cleanup suggestions.
- Test coverage mapping and traceability review.
- CMake, CI, and platform-compatibility debugging assistance.
- README drafting.
- Code creation aligning to documentation and user guidance.

## Accepted Suggestions

- Using a clearer docs index in the README.
- Tightening the validation traceability matrix to match the current test names.
- Documenting the audit log storage path explicitly.
- Making the project a single-user desktop application.
- Clearly separating MVC functions via a project refactor.

## Refined Suggestions

- Sub-folders within the documentation folder to ensure all documents are organized by their respective phase throughout the project.
- Older documentation was kept as legacy context instead of being deleted, so the defense packet preserves the project history.
- The README was shortened and focused on setup, usage, requirements, and defense-ready navigation rather than a full source tree dump.

## Rejected Suggestions

- Any suggestion to hide missing validation or skip important runtime coverage was rejected.
- Any suggestion to over-compress the docs at the expense of reviewer clarity was rejected.
- Any suggestion involving large code changes or refactors in one go.  These workflows were broken up into smaller pieces focusing on one class, method, or file at a time.

## Human Decisions

- Preserve historical artifacts in `docs/legacy/` for AI tracking and review continuity.
- Keep requirements, planning, architecture, validation, and operations artifacts easy to find.
- Prefer honest documentation of gaps over pretending every artifact is fully finalized.
- Use the Qt framework as the backbone of the project for easy cross-compatibility.
- Adopt the Liskov Substitution Principle and the Dependency Inversion Principle involving a code refactor to ensure the application is portable ie. a database other than SQLite can be used as long as it inherits INoteRepository.

## Lessons Learned
While AI-Driven Software Development allows for accelerated coding, it is still important to take care in not letting AI run the project.  It is capable of producing large amounts of code and documentation from a single prompt, and the project can drift even if the AI claims to be following the documentation.  While AstraNotes has ample documentation and they all contain useful information, I believe that the amount of documentation can be overwhelming for a new developer coming into the project.  I created the sub-folders for the documents at the end of the project to help organize them, and so it is less overwhelming.  In the future, when I am orchestrating AI, I plan to create document sub-folders at the start of the project, with each sub-folder relating to a clear phase in the SDLC.  I believe that this type of organization will allow for greater traceability and a more consistent, smooth project flow.