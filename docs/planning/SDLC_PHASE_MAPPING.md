# AstraNotes Documentation to SDLC Phase Mapping

**Purpose**: Map each documentation artifact to its corresponding Software Development Life Cycle (SDLC) phase to help navigate the project and understand which documents are relevant at each stage of development.

---

## SDLC Phase Overview

This project follows a structured SDLC with the following phases:

1. **Requirements & Analysis** - Define what the system must do
2. **Design** - Define how the system will be built
3. **Planning** - Organize work into sprints and backlog
4. **Implementation** - Write the code
5. **Testing & Verification** - Validate behavior and collect evidence
6. **Release & Deployment** - Decide go/no-go and release
7. **Project Tracking & Meta** - Cross-cutting documents for visibility

---

## Requirements & Analysis Phase

**Purpose**: Capture, refine, and validate what the system must do.

| Document | File Path | Purpose |
|-----------|-----------|---------|
| **Initial Requirements (Refined)** | `docs/requirements/INITIAL_REQUIREMENTS_REVISED.md` | Authoritative refined requirements including 9 FRs, NFRs, and SRs with detailed acceptance criteria per requirement |
| **Product Requirements Document** | `docs/requirements/ASTRANOTES_PRD.md` | High-level product summary, goals, target users, in-scope/out-of-scope, success metrics, and principles |
| **Requirement Traceability Matrix** | `docs/validation/REQUIREMENT_TRACEABILITY_MATRIX.md` | Validates alignment between refined requirements and UML diagram evidence; includes gap analysis and status summary |

**How to use this phase**:
- Start with the PRD to understand the overall product direction and goals.
- Review INITIAL_REQUIREMENTS_REVISED.md to understand the detailed requirements.
- Use the Traceability Matrix to verify that each requirement is addressed in the UML design.
- Do NOT proceed to the next phase until all requirements are understood and documented.

---

## Design Phase

**Purpose**: Define the system architecture and structure that will satisfy the requirements.

| Document | File Path | Purpose |
|-----------|-----------|---------|
| **Architecture Document** | `docs/architecture/ARCHITECTURE.md` | 7-layer architecture design, 10K+ notes scale model, caching strategy, plugin system, security approach, and design decisions |
| **Class Responsibilities & Plugin Integration** | `docs/architecture/CLASS_RESPONSIBILITIES_AND_PLUGIN_INTEGRATION.md` | Detailed class roles for `Note`, `NoteService`, `INoteRepository`, `SQLiteNoteRepository`, `PluginManager`, `IPlugin`, `EncryptionService`, and `CacheManager` with integration flow diagrams |
| **UML Starter Diagrams** | `docs/legacy/UML_STARTER_DIAGRAMS.md` | Use case, class, sequence, state, component, deployment, and activity diagrams showing system structure and behavior |

**How to use this phase**:
- Review ARCHITECTURE.md to understand the overall design approach.
- Study CLASS_RESPONSIBILITIES_AND_PLUGIN_INTEGRATION.md to understand class boundaries and how plugins/encryption are integrated.
- Use the UML diagrams (use cases, class diagrams, sequence diagrams, activity diagrams) as the primary design reference.
- Validate that the design aligns with the Traceability Matrix from the Requirements phase.
- Confirm with the team that the design approach is acceptable before proceeding to Planning.

---

## Planning Phase

**Purpose**: Organize requirements and design decisions into executable work (sprints, backlog, and roadmap).

| Document | File Path | Purpose |
|-----------|-----------|---------|
| **User Stories** | `docs/requirements/USER_STORIES.md` | 18 user stories (Stories 1–8 core, Stories 9–18 gap coverage) with acceptance criteria, phase mapping, and DoD checklist |
| **Product Backlog** | `docs/planning/BACKLOG.md` | Stories prioritized by business value with effort estimates and initial scope |
| **Sprint Implementation Plan** | `docs/planning/SPRINTS_IMPLEMENTATION_PLAN.md` | 6 informal sprints organized by dependency order: (1) core lifecycle, (2) search/metadata, (3) security, (4) plugins, (5) trash, (6) hardening |
| **Implementation Roadmap** | `docs/planning/IMPLEMENTATION_ROADMAP.md` | Phase-based roadmap linking UI, note list, persistence, plugins, and search into implementation phases |

**How to use this phase**:
- Review User Stories to see the breakdown of each requirement into actionable stories with acceptance criteria.
- Check the Product Backlog to understand priority and effort estimates.
- Use the Sprint Implementation Plan to determine which stories to pull into the next sprint.
- Reference the Implementation Roadmap for phase-level guidance on sequencing and dependencies.
- Update the backlog as work progresses and priorities shift.

---

## Implementation Phase

**Purpose**: Write code that satisfies the design and implements the planned work.

| Document | File Path | Purpose |
|-----------|-----------|---------|
| **Development Checkpoint** | `docs/planning/DEVELOPMENT_CHECKPOINT.md` | Pre-development guide with recommended reading order, readiness checklist, first implementation milestones, and development guidance |
| **Actions Performed** | `docs/legacy/ACTIONS_PERFORMED.md` | Running log of all significant implementation actions, file changes, and milestones (to be updated as coding progresses) |

**How to use this phase**:
- Start with the Development Checkpoint to understand the recommended implementation order.
- Follow the suggested Sprint 1 sequence: Note model → SQLite → Manual/autosave → Error handling → Crash recovery.
- Update ACTIONS_PERFORMED.md after each major implementation milestone or file change.
- Reference the Design phase documents (ARCHITECTURE.md, CLASS_RESPONSIBILITIES.md, UML diagrams) as you write code.
- Trace each code change back to a user story and requirement ID for traceability.

---

## Testing & Verification Phase

**Purpose**: Validate that the implementation meets requirements and collect evidence of correctness.

| Document | File Path | Purpose |
|-----------|-----------|---------|
| **Test Plan** | `docs/validation/TEST_PLAN.md` | Test strategy covering unit tests, integration tests, UI tests, performance tests, and security tests; includes requirement-to-test trace examples |

**How to use this phase**:
- Use the Test Plan to design test cases for each user story and requirement.
- Link test results back to user stories and requirements using the trace examples.
- Verify that performance targets, security requirements, and crash recovery behavior are all validated before moving to Release.

---

## Release & Deployment Phase

**Purpose**: Validate that the system is ready to release, make a go/no-go decision, and prepare for deployment.

| Document | File Path | Purpose |
|-----------|-----------|---------|
| **Release Gates** | `docs/planning/RELEASE_GATES.md` | Go/no-go checklist, gate categories (Functional, Quality, Security, Reliability, Traceability), and example release decision record |
| **Customer Acceptance and DoD** | `docs/requirements/CUSTOMER_ACCEPTANCE_AND_DOD.md` | Simplified Definition of Done (DoD) per user story and formal sign-off section for customer acceptance on each story and final release |

**How to use this phase**:
- Verify that all Release Gate criteria are satisfied before declaring a sprint or release ready.
- Use the Customer Acceptance document to walk through each story with the customer and obtain sign-off.
- Record the go/no-go decision and date in the Release Gates document.
- If any gates are not met, identify the gaps and defer to the next sprint or handle as exceptions.

---

## Project Tracking & Meta Phase

**Purpose**: Provide cross-cutting visibility and navigation across all phases.

| Document | File Path | Purpose |
|-----------|-----------|---------|
| **Traceability Chain** | `docs/validation/TRACEABILITY_CHAIN.md` | End-to-end chain from requirements → user stories → backlog → sprints → tests → release gates; shows how all phases connect |
| **Development Checkpoint** | `docs/planning/DEVELOPMENT_CHECKPOINT.md` | Pre-development navigation guide; recommended reading order and checkpoint decision criteria (also listed under Implementation) |

**How to use this phase**:
- Refer to TRACEABILITY_CHAIN.md whenever you need to understand how a specific artifact (e.g., a user story or test case) connects to the larger project flow.
- Use the Checkpoint document to audit the project before moving into Implementation.
- Keep both documents current as the project evolves.

---

## Document Flow Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│ Requirements & Analysis                                         │
├─────────────────────────────────────────────────────────────────┤
│ • INITIAL_REQUIREMENTS_REVISED.md                               │
│ • ASTRANOTES_PRD.md                                             │
│ • REQUIREMENT_TRACEABILITY_MATRIX.md                            │
└──────────────────────────────┬──────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│ Design                                                          │
├─────────────────────────────────────────────────────────────────┤
│ • ARCHITECTURE.md                                               │
│ • CLASS_RESPONSIBILITIES_AND_PLUGIN_INTEGRATION.md              │
│ • UML_STARTER_DIAGRAMS.md                                       │
└──────────────────────────────┬──────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│ Planning                                                        │
├─────────────────────────────────────────────────────────────────┤
│ • User_stories.md                                               │
│ • BACKLOG.md                                                    │
│ • SPRINTS_IMPLEMENTATION_PLAN.md                                │
│ • IMPLEMENTATION_ROADMAP.md                                     │
│ • DEVELOPMENT_CHECKPOINT.md                                     │
└──────────────────────────────┬──────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│ Implementation                                                  │
├─────────────────────────────────────────────────────────────────┤
│ • DEVELOPMENT_CHECKPOINT.md (reference)                         │
│ • ACTIONS_PERFORMED.md (update as you code)                     │
│ • Source code in src/ directory                                 │
└──────────────────────────────┬──────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│ Testing & Verification                                          │
├─────────────────────────────────────────────────────────────────┤
│ • TEST_PLAN.md                                                  │
│                                                                 │
└──────────────────────────────┬──────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│ Release & Deployment                                            │
├─────────────────────────────────────────────────────────────────┤
│ • RELEASE_GATES.md                                              │
│ • CUSTOMER_ACCEPTANCE_AND_DOD.md                                │
└──────────────────────────────┬──────────────────────────────────┘
                               │
                               ▼
        ┌──────────────────────────────────────────┐
        │ Project Tracking & Meta                  │
        ├──────────────────────────────────────────┤
        │ • TRACEABILITY_CHAIN.md (all phases)    │
        │ • DEVELOPMENT_CHECKPOINT.md (all phases)│
        └──────────────────────────────────────────┘
```

---

## Quick Reference Table

| Phase | Primary Documents | Key Activities |
|-------|------------------|-----------------|
| Requirements & Analysis | PRD, Requirements, Traceability Matrix | Capture and refine requirements; validate UML alignment |
| Design | Architecture, Class Responsibilities, UML Diagrams | Define system structure and behavior |
| Planning | User Stories, Backlog, Sprint Plan, Roadmap | Break requirements into sprints; organize work |
| Implementation | Development Checkpoint, Actions Log, Source Code | Write code; update progress tracking |
| Testing & Verification | Test Plan | Design and run tests; capture results |
| Release & Deployment | Release Gates, Customer Acceptance | Validate readiness; obtain sign-off; release |
| Meta / Cross-Cutting | Traceability Chain, Development Checkpoint | Navigate across phases; audit completeness |

---

## Recommended Checkpoint Sequence

If you want to audit the entire project from the beginning, review documents in this order:

1. **ASTRANOTES_PRD.md** - Understand product vision
2. **INITIAL_REQUIREMENTS_REVISED.md** - Review detailed requirements
3. **REQUIREMENT_TRACEABILITY_MATRIX.md** - Verify design covers requirements
4. **ARCHITECTURE.md** - Understand design approach
5. **CLASS_RESPONSIBILITIES_AND_PLUGIN_INTEGRATION.md** - Review class roles
6. **User_stories.md** - See requirement-to-story breakdown
7. **SPRINTS_IMPLEMENTATION_PLAN.md** - Understand implementation order
8. **DEVELOPMENT_CHECKPOINT.md** - Validate readiness to start coding
9. **TEST_PLAN.md** - Review testing strategy
10. **RELEASE_GATES.md** - Understand release criteria

---

## Notes

- The TRACEABILITY_CHAIN.md and DEVELOPMENT_CHECKPOINT.md documents serve as navigation aids and should be consulted throughout all phases.
- All new code changes should be traced back to at least one user story and requirement ID using the IDs from REQUIREMENT_TRACEABILITY_MATRIX.md.
- This document should be updated if new documentation artifacts are created or if the phase structure changes.