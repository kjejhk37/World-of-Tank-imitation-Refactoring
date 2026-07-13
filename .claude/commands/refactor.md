# Refactor

An inspection-only workflow that identifies refactoring candidates and surfaces them as issues.
No code is modified during this skill. Actual refactoring is carried out via task_delegation.

---

## Step 1 — Inspection

Scan the target code and check for the following.

**Structure**
- Classes or functions with more than one responsibility (SRP violation).
- High coupling between modules that should be independent.
- Duplicated logic that can be consolidated.

**Library Isolation**
- External libraries referenced directly outside their wrapper class.

**Readability**
- Functions or classes that are too large to understand at a glance.
- Naming that does not clearly convey intent.
- Missing or incomplete comments (per the Comment Writing Rules in CLAUDE.md).

**Test Coverage**
- Features that lack a corresponding unit test.

---

## Step 2 — Issue Report

Report each finding as an issue using the severity grades defined in CLAUDE.md.

```
- [HIGH] UserService handles both authentication and profile updates — SRP violation.
- [MEDIUM] DatabaseClient is imported directly in ReportGenerator — library isolation violated.
- [LOW] calculateTotal() has no comment block.
```

Present the report to the user.
Do not modify any code.

---

## Step 3 — Handoff

The user decides which issues to address.
Each selected issue is handled as a new task via task_delegation.
