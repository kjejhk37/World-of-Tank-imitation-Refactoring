# Project Overview

Refactor the `WOT-master` (`D3DX_2209`) tank-battle game codebase and migrate it from DirectX 11 to a later DirectX version.

- Purpose: learn C++/DirectX by directing the refactor, not by writing code directly.
- Also serves as a review of previously studied C++/DirectX/math concepts, and as a portfolio piece for a job search.
- The user does not write C++ code themselves; all refactoring and implementation goes through the agent_harness (Task Delegation workflow) below.
- Target codebase: `../WOT-master/D3DX_2209`.
- Progress is task-by-task — no full upfront plan. Each change goes through the workflow individually.

## Refactoring Scope

- Directory/project structure cleanup and overall architecture improvement, down to the basic directory layout.
- Code and mathematical mechanisms (e.g. physics, rendering math) are updated/modernized as part of the refactor, not just structure and build system.
- Apply multithreading where the current single-threaded design allows it.
- DirectX 11 → later DirectX version migration.
- Migrate the build system from `.sln`/`.vcxproj` to CMake.
- Apply the full range of the user's known software engineering knowledge, not just the items above.
- Final goal: a deployable, packaged `.exe` build environment.

## Development Environment

- Editor: Visual Studio → VS Code.
- Build system: `.sln`/`.vcxproj` → CMake.
- Reason: reproducible clone-and-build, CI compatibility, and headless build/verify for agent-driven implementation (Claude builds and verifies each change; Visual Studio's GUI-only build does not fit that loop).
- Sequencing: migrate alongside the directory/structure cleanup task, not as a blocking prerequisite.

---

# Collaboration Principles

Do not unconditionally agree with the user's opinions.
Actively propose alternatives or improvements when a better approach exists.
If the user's direction carries risks or trade-offs, state them clearly before proceeding.

All implementation work requires user approval obtained through the Task Delegation workflow before proceeding.
Do not take any action beyond the scope approved in the strategy document without explicit user consent.

If an unexpected situation arises during implementation — such as the need to add or remove functionality not covered in the strategy — stop immediately, notify the user, update the strategy .md to reflect the change, and resume only after re-approval.

---

# Custom Commands

- Task Delegation Workflow: [`.claude/commands/task_delegation.md`](.claude/commands/task_delegation.md)
- Git Workflow: [`.claude/commands/git_workflow.md`](.claude/commands/git_workflow.md) — Reference only if the file has been written.
- Dependency Evaluation: [`.claude/commands/dependency_eval.md`](.claude/commands/dependency_eval.md)

---

# .md Writing Rules

Write with the conclusion first (top-heavy structure).
Break each sentence onto its own line.
Use bullet points when listing examples or related items.

---

# Comment Writing Rules

Every function and class comment must include the following.

- Author
- Function/Class description
- Input description
- Output description
- Notes (trade-offs and risks)
- Date written

---

# Document Archiving

When a full work cycle is complete — task, brainstorming, strategy, implementation, and commit report all finalized — move the related documents into an archive folder to prevent Claude from loading unnecessary context during session recovery.

Archive by preserving the folder structure as a group so that relative paths between documents remain intact.

Archive destination: `docs/archive/summary_YYYYMMDD/`

Maintain the internal structure as follows.

```
docs/archive/summary_YYYYMMDD/
  brainstorming/task_YYYYMMDD_HHMM.md
  brainstorming/summary_YYYYMMDD_HHMM.md
  strategy/summary_YYYYMMDD_HHMM.md
  commit/summary_YYYYMMDD_HHMM.md
```

A cycle is considered complete when all of the following conditions are met.

- All checklist items in the strategy document are marked `[x]`.
- All issues in the commit report are marked `[RESOLVED]` or `[DEFERRED]`.

Mark an issue as `[DEFERRED]` when the user explicitly decides to postpone it to a future cycle.

During session recovery, read only the active documents in `docs/`.
Documents in `docs/archive/` are referenced only when the user explicitly requests it.

---

# Session Recovery

By default, the user reviews the implementation report in `docs/commit/` and delegates the next task.

Issues in the commit report follow a 3-stage lifecycle.

- Open — reported but no work delegated yet. Displayed with the original severity grade (e.g. `[CRITICAL]`).
- In Progress — a task delegation has been started to address the issue. Update the entry as follows.
- Resolved — the fixing work is complete. Update the entry as follows.

```
- [IN PROGRESS] Application crashes on startup when config file is missing.
  → task: ../brainstorming/task_summary_YYYYMMDD_HHMM.md

- [RESOLVED] Application crashes on startup when config file is missing.
  → resolved in: ../strategy/summary_YYYYMMDD_HHMM.md
```

If the user requests a session recovery, read the following documents in order and present recommendations before proceeding.

- `docs/brainstorming/` — Review the goals and trade-offs that were established. The task .md is also located here.
- `docs/strategy/` — Identify which checklist items were completed and which remain.
- `docs/commit/` — Review the implementation results and any outstanding issues by severity.

Based on the above, recommend the next action to the user.
Do not begin any work until the user confirms the direction.

---

# Project Initial Setup

If a project initial setup .md does not exist, confirm the following items, write the document, and reference it by relative path.

- User language (e.g. Korean, English, Japanese)
- Development language (e.g. Python, C++, C#, Rust)
- Base architecture (if none exists, recommend one based on project purpose and scalability)

---

# Core Development Principles

**Follow SOLID principles strictly. Apply the Single Responsibility Principle (SRP) with particular rigor.**
This ensures that developers can clearly understand AI-generated code after the fact.

**Always wrap external libraries in internal wrapper classes.**
This minimizes the scope of changes when an unverified library causes errors.
Define an internal class that owns and encapsulates all interactions with the external library.
Other modules must not reference the external library directly.

**Write unit tests for every developed feature.**
Whenever a feature is added or modified, run the build and unit tests without exception.
