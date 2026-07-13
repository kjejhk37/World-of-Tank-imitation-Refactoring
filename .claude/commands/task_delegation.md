# Task Delegation

When a task is delegated by the user, follow the steps below in order.

---

## Step 1 — Write the Task Document

Request answers to the following items from the user and save the completed content as `docs/task/summary_YYYYMMDD_HHMM.md`.

- Purpose (required)
- Author (optional)
- User draft (optional) — ideas or direction the user has already conceived
- Requirements (optional)
- Checklist (optional)

---

## Step 2 — Brainstorming (Bidirectional)

Before writing the brainstorming draft, move the task document from `docs/task/` into `docs/brainstorming/`.
This keeps `docs/task/` as a queue of only unstarted tasks.

Write a brainstorming draft and save it as `docs/brainstorming/summary_YYYYMMDD_HHMM.md`.

- Write the relative path of the task .md at the top of the brainstorming document.
- The task .md is now located in `docs/brainstorming/`, so reference it as a sibling file.

The purpose of brainstorming is as follows.

- Clarify the user's goals and refine the implementation strategy.
- Identify risks in the implementation process in advance and discuss how to resolve them.
- Explore solutions to achieve the user's goals.

The brainstorming document must include the following sections.

- Summary — Summarize the key conclusions drawn from brainstorming in a top-heavy structure.
- Trade-offs — State the pros and cons of each solution and the risks that must be accepted.

**Brainstorming is conducted bidirectionally with the user.**

- After writing the draft, share it with the user and request feedback.
- Revise the brainstorming .md based on the user's feedback.
- Repeat share → feedback → revise until the user approves.
- Proceed to Step 3 only after user approval.

---

## Step 3 — Write the Strategy Document (Bidirectional)

Write a strategy document based on the brainstorming results and save it as `docs/strategy/summary_YYYYMMDD_HHMM.md`.

- Write the relative path of the brainstorming .md at the top of the strategy document.

The purpose of the strategy document is as follows.

- Specify "how" to apply the brainstorming results to the code.
- Manage a checklist to track whether results are applied correctly.
- Define the final approval and verification criteria for code implementation.

The strategy document must include an implementation checklist.

- Write each item in `[ ]` format.
- Break the checklist down into the smallest implementable units.

**The strategy document is reviewed bidirectionally with the user.**

- After writing the draft, share it with the user and request feedback.
- Revise the strategy .md based on the user's feedback.
- Repeat share → feedback → revise until the user approves.
- Proceed to Step 4 only after user approval. Do not begin implementation before approval.

---

## Step 4 — Implementation

Implement according to the checklist in the user-approved strategy document.
Each time an item is completed, update the corresponding checklist item in the strategy document to `[x]`.

When all checklist items are marked `[x]`, ask the user the following before proceeding to Step 5.

> Implementation is complete. Would you like to run a **Code Review**?
> (Stage 1: quality inspection against SOLID, library isolation, test coverage, comment rules.
>  Stage 2: ASCII or HTML diagrams — class diagram, sequence diagram.)

- If yes: run the code review per `.claude/commands/code_review.md`, then ask about refactor inspection (below).
- If no: proceed to Step 5.

After the code review (or if the user skipped it), ask:

> Would you like to run a **Refactor Inspection**?
> (Scans for SRP violations, coupling issues, missing tests, and naming problems. Reports issues only — no code is changed. Any fixes are handled via a new task delegation.)

- If yes: run the refactor inspection per `.claude/commands/refactor.md`, then proceed to Step 5.
- If no: proceed to Step 5.

---

## Step 5 — Implementation Result and Issue Report

After implementation is complete, write a result report as `docs/commit/summary_YYYYMMDD_HHMM.md`.

- Report only "errors identified in the current state," regardless of whether they relate to newly added features.
- Write the report in a top-heavy structure, one sentence per bullet point.
- Assign a severity level to each reported issue using the grades below.

### Issue Severity Grades

| Grade | Label | Criteria |
|---|---|---|
| 1 | `CRITICAL` | Blocks execution or causes data loss. Must be resolved before any further work. |
| 2 | `HIGH` | Major functional failure. A workaround exists but the issue cannot be left unresolved. |
| 3 | `MEDIUM` | Non-blocking functional issue. Does not prevent operation but requires resolution. |
| 4 | `LOW` | Minor issue with no functional impact. Can be deferred. |

Write each issue in the following format.

```
- [GRADE] Description of the issue
```

Example:

```
- [CRITICAL] Application crashes on startup when config file is missing.
- [MEDIUM] Return value is incorrect when an empty list is passed as input.
- [LOW] Variable naming does not follow the naming convention.
```

After writing the commit report, if any open issues exist, ask the user the following.

> Issues were identified in the commit report. Would you like to run **Debug** on any of them?
> (Writes a minimal reproduction script, traces the root cause, and resolves the issue.)

- If yes: confirm which issue to debug, then run per `.claude/commands/debug.md`.
- If no: hand off to the user for review.
