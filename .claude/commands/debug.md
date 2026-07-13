# Debug

A lightweight workflow for reproducing and resolving issues identified in the commit report.
This is distinct from unit testing — the goal is the fastest possible reproduction of the specific failure.

---

## Step 1 — Identify the Issue

Read the target issue from the commit report in `docs/commit/`.
Confirm the issue grade and description with the user before proceeding.

---

## Step 2 — Quick Reproduction

Write a minimal reproduction script that isolates and triggers the issue.

- Use the smallest possible input and setup to reproduce the failure.
- Do not rely on the full test suite — focus only on reproducing this specific symptom.
- Run the reproduction script and confirm the failure is observable.

If the issue cannot be reproduced, report this to the user immediately and stop.

---

## Step 3 — Root Cause Analysis

Trace the failure back to its root cause.

- Identify the exact line, function, or module where the failure originates.
- Distinguish the root cause from symptoms.
- Note any side effects or related areas that may be affected by a fix.

Present the root cause analysis to the user and request confirmation before proceeding.

---

## Step 4 — Fix

Implement the fix based on the confirmed root cause.
Follow the Core Development Principles in CLAUDE.md.

---

## Step 5 — Verify

Re-run the reproduction script from Step 2 and confirm the failure no longer occurs.
Run the existing unit tests and confirm nothing is broken.

---

## Step 6 — Update Issue Status

Update the resolved issue in the commit report to `[RESOLVED]`.
Add the relative path reference as defined in the Session Recovery section of CLAUDE.md.
