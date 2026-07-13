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

# Shared Workflow & Collaboration Rules

Collaboration principles, the Task Delegation workflow, custom commands, writing rules, and core development principles are defined once in `agent_harness` (linked below as a submodule) and apply here as-is.

@agent_harness/CLAUDE.md
