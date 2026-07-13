# Dependency Evaluation

Evaluate an external library before adoption.
Run this skill before adding a new dependency to the project.

---

## Step 1 — Evaluation

Assess the candidate library across the following criteria and present the findings to the user.

**Alternatives**
- List at least 2 alternative libraries that serve the same purpose.
- Compare each on: feature coverage, community size, maintenance activity.

**Maintenance Status**
- Last release date, open issue count, whether the repository is actively maintained.

**License**
- Confirm the license is compatible with the project's usage.

**Risks**
- Known vulnerabilities, breaking change history, dependency chain complexity.

---

## Step 2 — Wrapper Design Draft

Propose an internal wrapper class design for the library.

- Define the class name and the methods it will expose.
- Show which parts of the library will be encapsulated.
- Confirm that no other module will reference the library directly.

Present the wrapper design to the user and request approval before proceeding.

---

## Step 3 — Verification Status

Record the verification status of the library's key features.

Each feature is marked with one of the following statuses.

| Status | Meaning |
|---|---|
| `[ ] Unverified` | Feature has not been tested in this project. |
| `[~] In Progress` | Verification task has been delegated via task_delegation. |
| `[x] Verified` | Feature has been confirmed to work correctly in this project. |

List the key features to verify based on the project's intended usage.

Example:

```
- [ ] Unverified — Parse input from file
- [ ] Unverified — Handle malformed input gracefully
- [ ] Unverified — Performance under large dataset
```

**Verification of each feature is carried out as a separate task via task_delegation.**
Update the status to `[~] In Progress` when a verification task is delegated.
Update to `[x] Verified` when the task is complete and the feature is confirmed.

---

## Output

Save the evaluation result as `docs/dependency/library-name_YYYYMMDD.md`.
