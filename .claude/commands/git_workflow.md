# Git Workflow

Written with solo development as the baseline.
When transitioning to team development, refer to the team considerations in each section.

---

## Branch Strategy

- `main` — Maintains only stable, deployable state.
- `develop` — Integration branch for development. All feature branches diverge from here.
- `feature/feature-name` — Created per feature unit. Merged into `develop` and deleted upon completion.
- `hotfix/description` — Branched directly from `main` for urgent fixes. Merged into both `main` and `develop`.

> **Team consideration**: Add `release/version` branches and configure branch protection rules with required reviewers.

---

## Commit Rules

Refer to the implementation result reports in the `docs/commit/` folder for commit content.

---

## Merge Rules

- `feature` → `develop`: **Squash merge** (consolidate commit history into one)
- `develop` → `main`: **Merge commit** (preserve history)
- `hotfix` → `main` / `develop`: **Merge commit**

> **Team consideration**: Require PRs, enforce at least one reviewer approval before merging, and agree on a Squash/Rebase policy as a team.

---

## Tags / Releases

Version tags follow Semantic Versioning: `vMAJOR.MINOR.PATCH`

- `MAJOR` — Breaking changes that are not backwards compatible
- `MINOR` — New features added while maintaining backwards compatibility
- `PATCH` — Bug fixes

Create a tag at the point of merging into `main`.

> **Team consideration**: Automate release note generation and integrate with a CI/CD pipeline.

---

## .gitignore Management

- Apply a base template matching the development language and IDE at initial setup.
- Always include secret keys and environment variable files (`.env`).
- Do not commit build artifacts or dependency folders.
