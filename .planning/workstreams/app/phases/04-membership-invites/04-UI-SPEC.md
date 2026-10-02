---
phase: 4
slug: membership-invites
status: draft
shadcn_initialized: false
preset: none
created: 2026-09-27
---

# Phase 4 — UI Design Contract

> Visual and interaction contract for membership & invites (MEMB-01…MEMB-05). The design system is **Material 3 via the `frontend_scaffold` widget library** (in-tree submodule `src/app/scaffold/`), not shadcn — this is a Flutter app; shadcn's registry model does not apply. **This contract inherits every Phase 1-3 token decision** (`01/02/03-UI-SPEC.md`, pinned against `scaffold_dimens.dart` + `scaffold_palette.dart`) and declares only Phase 4 deltas and new surface contracts. Do not invent values.
>
> Phase 4 UI scope (locked by `04-CONTEXT.md` D-01…D-08): a **member list** per scope (space or standalone room), a **five-tier role badge** on every member, an **invite flow** that mints and copies a `gcs://invite/...` link, a **join (redeem) flow** that pastes an invite link, and the **approval surface** for destructive ops (admin demotion, member removal, space/room deletion) when 2+ admins exist. The member roster, invite link, and approval state all reach Dart as **pushed** `GcsEvent` data — the UI never synthesizes membership (C++ owns state, D-04). No kick/ban/message-delete (Phase 5), no bot (Phase 6), no lobby (Phase 7).
>
> **Claude's Discretion resolved here (04-CONTEXT.md):** member-list + invite entry points, role display, approval surfacing, and invite-link clipboard/share mechanics are explicitly assigned to the UI phase. The decisions below (member list as a `ResponsiveDrawer` dialog, join as a rail-header affordance + paste dialog, role badges on the five tiers, pending ops rendered inline from pushed `approved_by`) are this phase's design contract, not deferrals.

---

## Design System

| Property | Value |
|----------|-------|
| Tool | none (Flutter Material 3 — scaffold is the design system) |
| Preset | not applicable |
| Component library | `frontend_scaffold` (in-tree git submodule, `src/app/scaffold/`, package `package:frontend_scaffold`) |
| Icon library | Flutter material `Icons` (unchanged from Phase 1) |
| Font | Material default (inherits scaffold `scaffold_theme.dart`; no custom font) |

**Source of truth (unchanged, D-12/D-22):** `src/app/scaffold/lib/theme/` (`scaffold_palette.dart`, `scaffold_dimens.dart`, `scaffold_colors.dart`). Light/dark palettes registered per Phase 1 in `src/app/lib/theme/gcs_theme.dart`. Scaffold `lib/` is read-only for this workstream (submodule contract) — Phase 4 composes atoms, never edits or forks them.

**Zero new packages** (Phase 2-3 discipline holds). Every new UI surface below is plain Dart under `src/app/lib/shell/` composed from scaffold atoms.

---

## Spacing Scale

Inherited unchanged from Phase 1 (`ScaffoldDimens.defaultDimens`; the scaffold scale is **2px-based**, token `spaceN` resolves to `2 × N` px). Phase 4 application of existing tokens:

| Token | Value | Phase 4 usage |
|-------|-------|---------------|
| `space2` | 4px | Gap between trailing icon affordances on a member row; label-to-control gap in dialogs; badge-to-text gap |
| `space3` | 6px | Chevron-to-name gap in space nodes (inherited); icon-to-address gap in member rows |
| `space4` | 8px | Gap between form fields / footer buttons in dialogs (inherited) |
| `space6` | 12px | Horizontal padding of member rows; horizontal padding of invite/join dialog fields |
| `space8` | 16px | Member-list dialog inner padding; section padding (inherited) |
| `space12` | 24px | Rail section break (inherited) |

**Radii (inherited):** dialog drawer corner 16px (atom-fixed by `ResponsiveDrawer`); member-row hover/selection tint `radiusMd` 12px (unused for selection — member rows are not selectable, see Interactions); footer confirm pills `borderRadiusButton` 48px; circular icon affordances `BoxShape.circle`.

**Sizes:** trailing icon affordances (members, change-role, remove, approve) are **40px circular** (`minTouchTarget − space4` = 48 − 8, the Phase 1 `_SendButton` idiom) lifted to 48px hit area by `ScaffoldPressable`'s internal `ScaffoldTouchTarget`. Member rows keep the rail row contract: 40px content + `space2 × 2` vertical padding = 48px minimum touch target.

**Exceptions:** none new. `minTouchTarget` 48px is reused, not changed. `space3` (6px) is the one non-multiple-of-4 token the phase consumes; it is pre-existing, not introduced here.

---

## Typography

Inherited unchanged from Phase 1 (`design_tokens.json typography.scale`): **exactly 4 sizes, 2 weights (400 regular, 500 medium)**. No new sizes or weights enter Phase 4.

| Role | Size | Weight | Line Height | Phase 4 usage |
|------|------|--------|-------------|---------------|
| Headline | 32px | 400 | 40px | Reserved (unused) |
| Title | 22px | 500 | 28px | Reserved for pane header (unused in dialogs — drawer owns its title) |
| Body | 14px | 400 | 20px | **Member addresses (truncated), role-selector radio labels, dialog helper lines, "Leave"/"Delete" confirm bodies, button labels** |
| Label | 11px | 500 | 16px | **Role badge text (atom-sized `labelSmall`); "Pending…" badge text (atom-sized); "You" self-marker; invite-link helper line** |

**Contract lines:**
- Member addresses render **truncated short-form** (same convention as the Phase 3 `senderName` sender label: leading prefix + ellipsis + trailing suffix). The full address is never rendered inline.
- The invite link (`gcs://invite/…`) renders in the invite view in a **monospace** face (the scaffold's code/reserved face, as used for code blocks) at Body size, `textPrimary`, so the URL is copy-scannable. No new font family is registered.
- Role badge text renders at `ScaffoldBadge`'s intrinsic `labelSmall` size — do not re-style (matches the Phase 2 "Private" badge rule).
- The "You" self-marker uses Label at `textSecondary`.

---

## Color

Inherited 60/30/10 split from Phase 1, both palettes, unchanged values. Phase 4 adds **new element mappings only**, all from existing tokens. The **dark palette** is the primary contract (`ScaffoldPalette.defaultPalette`); light mode mirrors via `lightPalette` with the same accents.

| Role | Token (light / dark) | Phase 4 usage |
|------|----------------------|---------------|
| Dominant 60% | `grayPrimary` (#F1F3F5 / #151E29) | Rail base (unchanged) |
| Secondary 30% | `deepBlueTertiary` (#E9EDF2 / #05090F) | **Member-list / invite / join dialog surface** (atom-fixed by `ResponsiveDrawer`) |
| Secondary 30% | `deepBlueCardColor` (#FFFFFF / #0A121F) | Rail card surfaces (unchanged) |
| Accent 10% | `lightGreenPrimary` (#00EAAE / #00EAAE) | See reserved-for list below |
| Destructive | `statusError` (#D13438 / #FF4D4D) | **Remove affordance, destructive confirm fills (Remove / Leave / Delete)** |
| Warning | `statusWarningText` (#9A6A00 / #FFC42E) | **"Pending removal" / "Pending role change" badges** |
| Success | `statusSuccess` (#0AA06B / #0AD89C) | **Reserved — not used for interactive affordances in Phase 4** (see note) |
| Text primary | `textPrimary` | Member addresses, invite-link text, role-selector labels |
| Text secondary | `textSecondary` | "You" marker, change-role + approve affordance tints, helper lines, empty/loading copy |

### Accent reserved for (Phase 4 cumulative list)

Accent (`lightGreenPrimary`) is reserved for **exactly these** elements:
1. Self-message bubble fill (Phase 1)
2. Selected-room indicator in the left rail (Phase 1)
3. Composer focus ring (Phase 1)
4. Send button fill (Phase 1)
5. Active typing indicator dot (Phase 1)
6. Rail-header "+" create affordance (Phase 2)
7. Dialog confirm button fill (Phase 2)
8. Empty-state action button fill (Phase 2)
9. **"Invite members" CTA** in the member-list dialog footer (this phase's primary CTA)
10. **"Copy link" action** in the invite view
11. **"Join" CTA** in the join dialog
12. **Super Admin role badge fill** (exactly one per scope — the singular creator)

Accent is explicitly **NOT** used for: the rail "Join" affordance, per-node "Members" affordances, per-row change-role / remove / approve affordances, non-Super-Admin role badges, or the "Pending…" badges.

### Phase 4 role-badge color contract (resolves role display)

`ScaffoldBadge(variant: text)` renders each tier with a `badgeColor` mapped by role; the atom's luminance-resolved on-status color keeps every badge WCAG-AA without new colors:

| Role | Badge text | badgeColor |
|------|-----------|-----------|
| Super Admin | "Super Admin" | `lightGreenPrimary` (accent — one per scope) |
| Admin | "Admin" | `textSecondary` |
| Moderator | "Moderator" | `textSecondary` |
| Member | "Member" | `textSecondary` |
| Guest | "Guest" | `textSecondary` |

Non-Super-Admin tiers are distinguished by badge **text**, not color — the roster stays calm and the singular protected role is the only color-coded one (mirrors Phase 2: "Private" is signaled by exception, badge noise stays zero for the common case).

### Phase 4 pending-state color contract (resolves approval surfacing)

- **"Pending removal" / "Pending role change"** render as `ScaffoldBadge(variant: text)` with `badgeColor: statusWarningText` — an attention state, not an error. A pending space/room deletion renders the same badge on the rail node (see Interactions).
- The **approve** affordance is `textSecondary` (neutral, per-row discipline) — see Interactions. `statusSuccess` is deliberately **not** used for interactive affordances: it is a near-hue of the accent green and would blur "this is the CTA" vs "this is a status". `statusSuccess` stays reserved for its existing status roles.

### Destructive contract (first real use in the workstream)

Phase 4 is the first phase that ships destructive actions (Phase 2 declared `statusError` "reserved only"). `statusError` paints:
1. The per-row **remove** affordance icon (`Icons.person_remove_outline`).
2. The **destructive confirm fill** of the Remove / Leave / Delete confirmation dialogs.

Destructive confirm buttons follow the Phase 2 confirm-pill composition (40px pill, `borderRadiusButton`, `ScaffoldSurface`) but fill `statusError` and use `ScaffoldColors.btnText`-on-status-resolved foreground (white on the dark `#FF4D4D`; the light-mode `#D13438` resolves dark text — executor uses `ScaffoldBadge._resolveOnStatusColor`'s luminance rule or the equivalent status-resolved foreground; the exact helper is a planner detail).

---

## Copywriting Contract

Voice (inherited): sentence case, no exclamation marks, no "Oops". Error copy always pairs a problem statement with a next step. All copy below is final — executor uses it verbatim.

| Element | Copy |
|---------|------|
| Primary CTA (member list footer) | **"Invite members"** (verb + noun) |
| Member-list affordance semantic label | "View members of \<name\>" (icon-only pressable — WCAG 4.1.2) |
| Join affordance semantic label | "Join via invite link" (icon-only pressable) |
| Member-list dialog title | "Members of \<name\>" |
| Self marker | "You" (Label, `textSecondary`, after own address) |
| Role badge text | **"Super Admin" / "Admin" / "Moderator" / "Member" / "Guest"** (exact five-tier names — success criterion 3) |
| Pending-removal badge | "Pending removal" |
| Pending-role-change badge | "Pending: \<role\>" (e.g. "Pending: Member") |
| Pending-deletion badge (rail) | "Pending deletion" |
| Change-role affordance semantic label | "Change role for \<address\>" |
| Remove affordance semantic label | "Remove \<address\>" |
| Approve affordance semantic label | "Approve removal of \<address\>" / "Approve role change for \<address\>" |
| Invite view title | "Invite to \<name\>" |
| Invite view helper | "Anyone with this link joins as a Guest." |
| Invite view action | **"Copy link"** |
| Invite view close | "Done" |
| Copy success toast | "Invite link copied" (compact pill — no title, `ToastType.success`) |
| Join dialog title | "Join" |
| Join field hint | "Paste invite link" |
| Join field inline error | "Enter a valid invite link." (dialog stays open) |
| Join CTA | **"Join"** |
| Join cancel | "Cancel" |
| Role-change dialog title | "Change role" |
| Role-selector labels | "Admin" / "Moderator" / "Member" / "Guest" (Super Admin is creator-only — never grantable, never listed) |
| Role-change confirm | "Save role" |
| Role-change cancel | "Cancel" |
| Remove confirmation title | "Remove member" |
| Remove confirmation body | "Remove \<address\> from \<name\>? They'll lose access." |
| Remove confirm (destructive) | **"Remove"** |
| Leave confirmation title | "Leave \<name\>?" |
| Leave confirmation body | "You'll lose access to this space and its rooms." |
| Leave confirm (destructive) | **"Leave"** |
| Delete confirmation title | "Delete \<name\>?" |
| Delete confirmation body | "This deletes it for everyone. This can't be undone." |
| Delete confirm (destructive) | **"Delete"** |
| Approve confirmation | **None** — tapping "Approve" is the Super Admin's explicit act and activates the pending op immediately (D-07: single-signer, no timeouts) |
| Empty state — member list, heading | "No members yet" |
| Empty state — member list, body | "Invite someone to this space to get started." |
| Error toast — invite mint failed, title | "Couldn't create invite" |
| Error toast — join failed, title | "Couldn't join" |
| Error toast — join failed, message | "The invite link wasn't accepted. Check it and try again." |
| Error toast — role change failed, title | "Couldn't change role" |
| Error toast — remove failed, title | "Couldn't remove member" |
| Error toast — approve failed, title | "Couldn't approve" |
| Error toast — leave failed, title | "Couldn't leave" |
| Error toast — publish failed, message | "The command didn't reach the chat core. Try again." (verbatim Phase 2 transport message) |
| Success feedback | **No toast** except the "Invite link copied" pill. Member-list / rail updates under pushed events ARE the confirmation (no local optimism, D-04/D-05) |

---

## Interaction Contracts

### Rail entry points (D-21 inherited)

```
┌─────────────────────────────┐
│ Spaces          [⧉] [＋]    │  ← toolbar: "Join" (person_add, textSecondary)
│─────────────────────────────│      + accent 40px circular "+"
│ ▾ Space A   [Private] [👥][✎][＋] │  ← space node: NEW members affordance
│     general                 │      (group_outlined, textSecondary)
│     random                  │  ← nested room rows: NO members affordance
│ ▾ Space B            [👥][✎][＋] │      (membership is derived from the space)
│                             │
│   Rooms                     │
│   lounge             [👥]   │  ← standalone room row: NEW members affordance
│   quiet              [👥]   │
└─────────────────────────────┘
```

- **Toolbar** gains a **"Join"** icon affordance (`Icons.person_add_outlined`, `textSecondary`, 40px circular, 48px hit area) to the LEFT of the accent "+". The "+" stays the single accent instance per rail (Phase 2 discipline); "Join" is `textSecondary`. Both persist in every rail state (the entry points are never a dead end).
- **Space nodes** gain a **"Members"** affordance (`Icons.group_outlined`, `textSecondary`) in the trailing cluster, to the LEFT of the edit "+" (order: members → edit → "+"). Opens the member list for that space.
- **Standalone room rows** gain a **"Members"** affordance (`Icons.group_outlined`, `textSecondary`) as a trailing icon (the row currently has no trailing affordances). Opens the member list for that standalone room.
- **Nested room rows** get **no** members/invite affordance — nested-room membership is derived from the parent space (D-01: `MemberRecord` scope is "a space or a standalone room", D-02: `joined(room) = parentSpace.autoJoinRooms && !member.explicitLeave`). Invite the space instead.
- The members affordance follows the `_NodeIconAffordance` idiom exactly (40px circular, `textSecondary`, `ScaffoldPressable` semantic label, innermost gesture wins).

### The member list (new — `members_dialog.dart`)

One `ResponsiveDrawer.show` dialog per scope, opened from the members affordance. Surface `deepBlueTertiary`, sheet corner 16px (atom-fixed). The dialog is a **dumb read of the pushed `MemberList`** for its scope — never synthesized.

**Roster row anatomy** (start → end):

```
[ truncated address (Body, textPrimary) ] [ "You" (Label) ] [ pending badge? ] [ role badge ] [ ✎ ] [ ⛔ ] [ ✓ ]
```

| Element | Presence |
|---------|----------|
| Address | Always — truncated short-form, ellipsized |
| "You" marker | Current user's own row only (Label, `textSecondary`) |
| Pending badge | Only when the member has a dormant destructive op (`approved_by` empty, D-07) — "Pending removal" or "Pending: \<role\>" |
| Role badge | Always — five-tier text badge per the color contract |
| Change-role affordance (`Icons.manage_accounts_outlined`) | Viewer has membership-grant capability (Super Admin/Admin) AND target is not the Super Admin AND target has no pending op |
| Remove affordance (`Icons.person_remove_outline`, `statusError`) | Viewer has membership capability AND target is not the Super Admin AND target is not the viewer AND target has no pending op |
| Approve affordance (`Icons.check_circle_outline`) | Viewer is the Super Admin AND target has a pending op |

**Affordance gating is presentation, not enforcement (D-08):** the row renders affordances from the **viewer's pushed role**; C++ write guards are the source of truth. A Super Admin row renders **no** change-role/remove affordances (D-03: "cannot be demoted" — the UI simply never offers it, the C++ guard backs it).

**Footer:** end-aligned row, `space4` gap — Cancel (transparent, Body 400, `textSecondary`) then **"Invite members"** (accent 40px pill, Body 500, `btnText` foreground). "Invite members" renders only when the viewer has invite capability (Super Admin/Admin always; Member when `membersCanInvite` — MEMB-05, D-08). "Leave \<name\>" (a `textSecondary` Body text action) renders start-aligned in the footer when the viewer is a non-Super-Admin member of the scope.

**Roster sub-states:**

| Condition | Rendered state |
|-----------|----------------|
| `MemberList` for this scope not yet pushed | `ScaffoldStateView(state: 'loading')` — skeleton |
| Pushed roster empty (edge — creator is always a member) | `ScaffoldStateView` empty variant, copy above |
| Pushed roster non-empty | Rows per anatomy above |

### The invite view (mode of the members dialog)

Tapping "Invite members" publishes the invite/mint command through the injected `GcsCommandTransport` and transitions the same drawer to the invite view (the link is C++-minted and pushed back — see State Contracts). The invite view renders:

- **Title** "Invite to \<name\>".
- The **invite link** in a read-only surface (monospace Body, `textPrimary`, ellipsized horizontally with the full value selectable/copyable).
- **Helper line** "Anyone with this link joins as a Guest." (Label, `textSecondary`).
- **"Copy link"** (accent pill) → `Clipboard.setData` + `showToast(context, 'Invite link copied')` (compact pill, success, no title).
- **"Done"** (transparent, `textSecondary`) closes.

**No local optimism:** the invite view shows whatever link the C++ pushed; until the push lands it renders a loading state (the same skeleton), and a `publishCommand == false` or a pushed rejection surfaces the "Couldn't create invite" error toast.

### The join (redeem) flow (new — `join_dialog.dart`)

The rail "Join" affordance opens a `ResponsiveDrawer` dialog with one field:

- **Field** "Paste invite link" (`TextEntryFieldWidget` + `TextFormFieldLogic`, autofocus, Enter submits).
- **Inline validation:** confirm with a value that does not parse as `gcs://invite/{space|room}/<id>?key=<key>` → "Enter a valid invite link.", dialog stays open.
- **Confirm "Join"** (accent pill): parses the link, publishes the redeem command through the transport, and **closes immediately** (Phase 2 D-05 behavior contract). `publishCommand == false` → "Couldn't join" error toast, dialog stays open. A C++-side rejection arrives as a pushed error → toast with the pushed reason. On success, the newly joined space/room appears in the rail and its member list via pushed events — **no success toast** (the rail/member-list update IS the confirmation).

### Role change (mode of the members dialog)

The change-role affordance opens the drawer in a **role-change mode**: title "Change role", a radio list of the **four grantable roles** ("Admin", "Moderator", "Member", "Guest" — Super Admin is creator-only and never listed), "Save role" confirm. Each role renders as a `ScaffoldSelectionIndicatorRadio` + Body label in a `ScaffoldPressable` row (the Phase 2 bool-radio-pair pattern, but a four-way single-select — the exact atom/coordination is a planner detail). **Demotion is destructive** (D-07): moving an Admin down requires Super Admin approval when 2+ admins exist, so the confirm writes dormant and the member row later shows "Pending: \<role\>" instead of updating immediately. Promotion (Member→Admin, Member→Moderator, Guest→Member) is not destructive and applies immediately.

### Remove member (destructive)

The remove affordance opens a confirmation dialog (`statusError` destructive confirm): title "Remove member", body "Remove \<address\> from \<name\>? They'll lose access.", confirm **"Remove"** (destructive fill), cancel "Cancel". When 2+ admins exist the removal is written dormant (D-07) — the member row then shows "Pending removal" and the Super Admin sees "Approve"; when a sole admin acts it applies immediately. No local optimism: the roster updates only via the pushed `MemberList`.

### Leave (self-removal → `explicit_leave`)

The "Leave \<name\>" footer action opens a confirmation (destructive): title "Leave \<name\>?", body "You'll lose access to this space and its rooms.", confirm **"Leave"**. It writes `explicit_leave` on the viewer's space-scope member record (D-02 — the derived-join override), never removes the record. Unavailable to the Super Admin (creator cannot self-remove; no guard exists to demote them).

### Delete space / room (destructive, D-07)

A **"Delete"** affordance joins the space-node trailing cluster and standalone room rows (rendered only to Super Admin/Admin, `statusError` tint, `Icons.delete_outline`, semantic label "Delete \<name\>"). Confirmation: title "Delete \<name\>?", body "This deletes it for everyone. This can't be undone.", confirm **"Delete"** (destructive fill). Same approval gate as removal: dormant + "Pending deletion" badge on the rail node + "Approve" for the Super Admin when 2+ admins exist. (Planner note: deletion UI may be scoped to its own plan — D-07 lists it as Phase 4 destructive scope, but the success criteria anchor on invites/roles/approval; the visual contract is shared with removal.)

### Pending approvals surfacing (D-07)

Pending destructive ops are **not** a separate list — they are embedded on the target record (`approved_by` empty) and rendered inline: a member row carries its "Pending removal" / "Pending: \<role\>" badge, a rail node carries "Pending deletion". The Super Admin's **Approve** affordance activates the op by overwriting `approved_by` with their id (single-signer, immediate, no confirmation). Non-Super-Admins see the pending badge as informational only (no approve affordance).

---

## Component Inventory (Phase 4)

All existing scaffold atoms; the new artifacts are plain-Dart shell widgets.

| Component | Source | Used for |
|-----------|--------|----------|
| `ResponsiveDrawer.show` | `components/bottom_drawer/responsive_drawer.dart` | Member-list / invite / join / role-change / confirm dialog shell (dialog ≥800px / bottom sheet below) |
| `TextEntryFieldWidget` + `TextFormFieldLogic` | `components/text_entry_field_widget.dart`, `text_form_field_logic.dart` | Join-dialog paste field (known light-mode contrast constraint, see Color note in 02-UI-SPEC) |
| `ScaffoldBadge` | `components/scaffold_badge.dart` | Five-tier role badges + "Pending…" badges (text variant, `badgeColor` mapped) |
| `ScaffoldSelectionIndicatorRadio` | `components/scaffold_selection_indicator_radio.dart` | Role-change selector (four-way single-select) |
| `ScaffoldPressable` | `components/scaffold_pressable.dart` | All rows and icon affordances; `semanticLabel` on every icon-only instance |
| `ScaffoldStateView` | `components/scaffold_state_view.dart` | Member-list loading skeleton / empty state |
| `ScaffoldSurface` | `components/scaffold_surface.dart` | Circular icon affordances + accent/destructive confirm pills (Phase 1 `_SendButton` idiom) |
| `showToast` | `components/toast/toast_manager.dart` | "Invite link copied" pill (no title), error toasts (title ⇒ card) |
| `Clipboard` | `package:flutter/services.dart` | Copy the invite link |
| **`RoomRail` (extended)** | `src/app/lib/shell/room_rail.dart` | Toolbar "Join" affordance; space-node + standalone-room "Members"/"Delete" affordances |
| **`MembersDialog` (new)** | `src/app/lib/shell/members_dialog.dart` | Member list + invite view + role-change + confirmations (modes) |
| **`JoinDialog` (new)** | `src/app/lib/shell/join_dialog.dart` | Paste-invite-link redeem flow |

---

## State Contracts (backing the visuals)

- **`GcsEvent` gains a `MemberList` arm** (D-04): a flat, full-replacement roster per scope (`scopeId` + ordered member entries carrying `address`, `role`, `explicit_leave`, `approved_by`/pending marker, and a `self` flag). Pushed on redeem, on any membership write, and at session init after CRDT replay (mirrors the `SpaceTree` push pattern).
- **`GcsEvent` gains an invite-link arm** (Claude's Discretion resolved): a dedicated pushed event carrying the minted `gcs://invite/...` URL that the invite view renders. This is **not** a toast reuse — the link is transient response data, not an error.
- **`GcsCommand` gains** invite/mint, redeem/join, change-role, remove, leave (`explicit_leave`), delete, and approve arms (append-only oneof discipline, D-24/D-26). Dart stays data-only (D-01): no id minting on the Dart side.
- **`MembersCubit` (new)** — or an extension of `RailState`; the visual contract is identical either way and the planner picks. Recommended: a dedicated `MembersCubit` holding `Map<String, List<MemberInfo>> byScope` (full replacement per scope, mirroring `RailCubit.setTree`), so the modal roster stays out of the always-visible rail tree. The members dialog reads `byScope[scopeId]`; pending state derives from each entry's `approved_by` emptiness (never locally computed).
- **`SessionCubit._dispatchEvent`** gains `hasMemberList` and `hasInviteLink` arms (the `MemberList` arm dispatches to the members cubit; the invite-link arm routes to the open invite view or queues it).
- **View-local state** (dialog modes, role-selector selection, paste field) never enters Cubits (Phase 2 D-05 discipline).

---

## Registry Safety

shadcn registry does not apply (Flutter app, no `components.json`, no npm). Equivalent supply-chain surface:

| Registry | Blocks Used | Safety Gate |
|----------|-------------|-------------|
| Dart pub (pub.dev) | **none** — zero new packages this phase | not required |
| Scaffold submodule | existing atoms only (in-tree, git-submodule-pinned) | APIs read directly this session — every atom named above verified in `src/app/scaffold/lib/components/` |
| Third-party blocks | none | not applicable |

No Jinja2 template regeneration, no new generated families, no scaffold `lib/` edits.

---

## Checker Sign-Off

- [ ] Dimension 1 Copywriting: PASS
- [ ] Dimension 2 Visuals: PASS
- [ ] Dimension 3 Color: PASS
- [ ] Dimension 4 Typography: PASS
- [ ] Dimension 5 Spacing: PASS
- [ ] Dimension 6 Registry Safety: PASS

**Approval:** pending

---

## Sources Pinned

All atom APIs and token values verified against live reads on 2026-09-27:

- `.planning/workstreams/app/phases/04-membership-invites/04-CONTEXT.md` — D-01..D-08 locked decisions; Claude's Discretion areas resolved here
- `.planning/workstreams/app/REQUIREMENTS.md` — MEMB-01..MEMB-05 (Phase 4 scope anchor)
- `.planning/workstreams/app/ROADMAP.md` — Phase 4 goal + success criteria 1-5
- `.planning/notes/gcs-chat-architecture.md` — §Invite Model (URL shapes), §Membership Types (five tiers), §Destructive Action Rules (approval table)
- `.planning/workstreams/app/phases/02-spaces-rooms/02-UI-SPEC.md` — inherited tokens, dialog/rail patterns, copy voice, accent reserved-for base
- `.planning/workstreams/app/phases/03-messaging/03-UI-SPEC.md` — sender short-form truncation, toast contract, status-color reservations
- `src/app/scaffold/lib/theme/scaffold_palette.dart` — `lightGreenPrimary`, `textSecondary`, `statusError`, `statusWarningText`, `statusSuccess` light/dark values
- `src/app/scaffold/lib/theme/scaffold_dimens.dart` — 2px-base `spaceN` scale, `minTouchTarget` 48, `radiusPill`/`radiusMd`/`borderRadiusButton`
- `src/app/scaffold/lib/theme/scaffold_colors.dart` — `btnText`, `btnFilterSelected`
- `src/app/scaffold/lib/components/scaffold_badge.dart` — text variant, `badgeColor`, luminance-resolved on-status color
- `src/app/scaffold/lib/components/toast/toast_manager.dart` — `showToast` title⇒card / no-title⇒pill density; `ToastType.success/error/warning`
- `src/app/lib/shell/room_rail.dart` — space-node/room-row anatomy, `_NodeIconAffordance` idiom this phase extends
- `src/app/lib/shell/space_room_dialog.dart` — `ResponsiveDrawer.show` + `_DialogForm` mode pattern, confirm-pill composition, transport-toast behavior
- `src/app/lib/shell/gcs_shell.dart` — shell layout, send-failure toast pattern
- `src/app/lib/cubits/session_cubit.dart` — `_dispatchEvent` arm this phase extends; `GcsCommandTransport.publishCommand`
- `src/app/lib/cubits/rail_cubit.dart` — `setTree` full-replacement pattern the members cubit mirrors
