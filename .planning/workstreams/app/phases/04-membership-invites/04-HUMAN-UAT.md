---
status: partial
phase: 04-membership-invites
source: [04-VERIFICATION.md]
started: 2026-10-21T18:04:23Z
updated: 2026-10-21T18:04:23Z
---

## Current Test

[awaiting human testing]

## Tests

### 1. Invite link end-to-end (visual)
expected: Generate a space and a room invite; the invite view shows a gcs://invite/... link with a Copy action and copying preserves the URL.
result: [pending]

### 2. Scope isolation of the invite view
expected: Open scope A's invite view, close it, open scope B's; B never shows A's link.
result: [pending]

### 3. Join flow
expected: Paste a valid link: Join enables, the dialog closes, and the member appears. Paste an invalid shape: the inline error keeps the dialog open.
result: [pending]

### 4. Post-join room access (review WR-01)
expected: After redeeming a standalone-room invite and a space invite with autoJoinRooms, the joined room(s) become selectable and receive messages without restarting the app. This is expected to FAIL by inspection: kRedeem does not refresh derived joins or push RoomList, and kLeave does not recompute the derived projection. Confirm the user-visible impact and whether the warning still warrants a follow-up plan.
result: [pending]

### 5. Role badges and gating
expected: Five-tier badges, the 'You' marker, and role-gated affordances match the UI spec for each tier.
result: [pending]

### 6. Pending-op surface
expected: With 2+ admins, a removal shows a Pending badge and the Super Admin Approve affordance activates it; a target re-redeeming an invite does not clear the badge.
result: [pending]

### 7. Two live nodes (production config)
expected: Mint on A, redeem on B (not joined); B joins as Guest and both rosters converge.
result: [pending]

## Summary

total: 7
passed: 0
issues: 0
pending: 7
skipped: 0
blocked: 0

## Gaps
