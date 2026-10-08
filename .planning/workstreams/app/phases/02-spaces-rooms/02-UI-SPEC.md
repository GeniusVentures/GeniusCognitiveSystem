---
phase: 2
slug: spaces-rooms
status: approved
shadcn_initialized: false
preset: none
created: 2026-09-19
reviewed_at: 2026-09-19
---

# Phase 2 — UI Design Contract

> Visual and interaction contract for the spaces/rooms tree rail + create/edit dialog (Flutter/Dart). The design system is **Material 3 via the `frontend_scaffold` widget library** (in-tree submodule `src/app/scaffold/`), not shadcn — this is a Flutter app; shadcn's registry model does not apply. **This contract inherits every Phase 1 token decision** (`01-UI-SPEC.md`, pinned against `design_tokens.json` + `scaffold_dimens.dart` + `scaffold_palette.dart`) and declares only Phase 2 deltas and new surface contracts. Do not invent values.
>
> Phase 2 UI scope (locked by 02-CONTEXT D-05): the rail grows from a flat joined-room list to a **Spaces tree + standalone Rooms sections**, a **"+" affordance** in the rail header and on each space node, and **one reusable create/edit dialog** composed entirely from existing scaffold atoms. No messaging UI changes (Phase 3), no membership UI (Phase 4), no delete UI (D-03 lands tombstone *fields* only).

---

## Design System

| Property | Value |
|----------|-------|
| Tool | none (Flutter Material 3 — scaffold is the design system) |
| Preset | not applicable |
| Component library | `frontend_scaffold` (in-tree git submodule, `src/app/scaffold/`, package `package:frontend_scaffold`) |
| Icon library | Flutter material `Icons` (unchanged from Phase 1) |
| Font | Material default (inherits scaffold `scaffold_theme.dart`; no custom font) |

**Source of truth (unchanged, D-12/D-22):** `src/app/scaffold/design_tokens.json` + `src/app/scaffold/lib/theme/`. Light/dark palettes registered per Phase 1. Scaffold `lib/` is read-only for this workstream (submodule contract) — Phase 2 composes atoms, never edits or forks them.

**Zero new packages** (verified in 02-RESEARCH Standard Stack). Every new UI surface below is plain Dart under `src/app/lib/shell/` composed from scaffold atoms.

---

## Spacing Scale

Inherited unchanged from Phase 1 (`ScaffoldDimens.defaultDimens`; resolved values are the contract). Phase 2 application of existing tokens:

| Token | Value | Phase 2 usage |
|-------|-------|---------------|
| `space2` | 4px | Badge-to-label gap in space rows; gap between trailing icon affordances on a space node; label-to-control gap in dialog |
| `space3` | 6px | Chevron-to-space-name gap in the space node row |
| `space4` | 8px | Gap between form fields in the dialog; gap between footer buttons |
| `space6` | 12px | Horizontal padding of every rail row (unchanged); **nesting indent** — child room rows add one `space6` (12px) leading inset beyond the space row's own `space6` |
| `space8` | 16px | Rail toolbar padding; section padding (unchanged) |
| `space12` | 24px | Section break between the Spaces block and the standalone Rooms block in the rail (Phase 1 section-break pattern) |

**Radii (inherited):** dialog drawer corner 16px (atom-fixed by `ResponsiveDrawer`'s sheet shape); rail row selection tint `radiusMd` 12px (unchanged); footer confirm/cancel pills `borderRadiusButton` 48px; circular icon affordances are `BoxShape.circle` (matching the Phase 1 send-button composition).

**Sizes:** trailing icon affordances (rail "+", space-node "+", space-node edit) are **40px circular** (`minTouchTarget − space4` = 48 − 8, the Phase 1 `_SendButton` sizing idiom) lifted to 48px hit area by `ScaffoldPressable`'s internal `ScaffoldTouchTarget`. Rail rows keep the Phase 1 contract: 40px content + `space2 × 2` vertical padding = 48px minimum touch target.

**Exceptions:** none new. `disabledOverlayOpacity` 0.40 is reused (see Color) — not a new value.

---

## Typography

Inherited unchanged from Phase 1 (`design_tokens.json typography.scale`): **exactly 4 sizes, 2 weights (400 regular, 500 medium)**. No new sizes or weights enter Phase 2.

| Role | Size | Weight | Line Height | Phase 2 usage |
|------|------|--------|-------------|---------------|
| Headline | 32px | 400 | 40px | Reserved (unused in Phase 2, as in Phase 1) |
| Title | 22px | 500 | 28px | Reserved for pane header (unused in rail — see note) |
| Body | 14px | 400 | 20px | **Space names, room names, dialog field labels, radio labels, toggle label, button labels, empty-state body, "No rooms yet" inline label** |
| Label | 11px | 500 | 16px | **autoJoin toggle description line; badge text is atom-sized** |

**Contract lines:**
- Section headers ("Spaces" toolbar label, "Rooms" section header) render **exactly like the shipped Phase 1 "Rooms" header** in `room_rail.dart`: `textTheme.titleSmall`, `textSecondary`, letterSpacing 0.5 — same style, two instances. Do not restyle one independently.
- Space name and room name use Body, ellipsized (`TextOverflow.ellipsis`), exactly as the shipped room row.
- Private badge text ("Private") renders at the `ScaffoldBadge` atom's intrinsic text-pill size — do not re-style.
- The "No rooms yet" inline row under an expanded, empty space uses Body at `textSecondary`, indented one nesting level, non-interactive.

---

## Color

Inherited 60/30/10 split from Phase 1, both palettes, unchanged values. Phase 2 adds **new element mappings only**, all from existing tokens:

| Role | Token (light / dark) | Phase 2 usage |
|------|----------------------|---------------|
| Dominant 60% | `grayPrimary` (#F1F3F5 / #151E29) | Rail base (unchanged) |
| Secondary 30% | `deepBlueTertiary` (#E9EDF2 / #05090F) | **Create/edit dialog surface** (atom-fixed by `ResponsiveDrawer`; matches Phase 1 "drawers, sheets" mapping) |
| Secondary 30% | `deepBlueCardColor` (#FFFFFF / #0A121F) | Rail card surfaces (unchanged) |
| Accent 10% | `lightGreenPrimary` (#00EAAE / #00EAAE) | See reserved-for list below |
| Accent 10% (secondary) | `lightGreenSecondary` (#01CC95 / #01CC95) | Not used by Phase 2 new elements |
| Destructive | `statusError` (#D13438 / #FF4D4D) | **Reserved only — no destructive actions ship in Phase 2** (no delete UI; D-03 is fields-only). Error-toast type color, unchanged |
| Text primary | `textPrimary` | Selected room label; space names |
| Text secondary | `textSecondary` | Unselected joined room labels; **private badge fill**; per-node "+"/edit icon tint; chevron tint; empty-state body |

### Accent reserved for (Phase 2 cumulative list)

Accent (`lightGreenPrimary`) is reserved for **exactly these** elements:
1. Self-message bubble fill (Phase 1)
2. Selected-room indicator in the left rail (Phase 1)
3. Composer focus ring (Phase 1)
4. Send button fill (Phase 1)
5. Active typing indicator dot (Phase 1)
6. **Rail-header "+" affordance icon** (the phase's primary CTA entry point — one instance per rail)
7. **Dialog confirm button fill** ("Create space" / "Create room" / "Save changes")
8. **Empty-state action button fill** ("Create space" in the no-spaces state view)

Accent is explicitly **NOT** used for: per-space-node "+" or edit icons (they render `textSecondary` — accent on every space row would violate the 10% discipline as the catalog grows), the private badge, chevrons, unjoined room rows, section headers, the cancel button.

### Phase 2 state-color contract lines

- **Joined vs unjoined room rows** (resolves research Open Question 1): a room row whose topic is present in the pushed `RoomList` renders at full strength (Phase 1 mapping: `textPrimary` when selected, else `textSecondary`). A catalog-but-not-joined room row renders via `ScaffoldPressable(disabled: true)` — `ScaffoldDisabledOverlay` at `disabledOverlayOpacity` 0.40 + `Semantics(enabled: false)`, non-tappable. **This dim/un-dim transition IS the observable for success criterion 3**: toggling `autoJoinRooms` off dims the space's child room rows (rooms remain visible per D-04); toggling on restores them. Standalone rooms are permanently dimmed in Phase 2 (no join UI until Phase 4).
- **Private badge**: `ScaffoldBadge(variant: text, text: 'Private', badgeColor: palette.textSecondary)`. The atom's luminance-resolved on-status color (white on `textSecondary` in both palettes) keeps it WCAG-AA without new colors. Private spaces only — **public spaces render no badge** (privacy is signaled by exception; badge noise stays zero for the common case). **Rooms render no privacy badge in Phase 2** — `RoomRecord` carries no `is_public` (public/private is a space attribute; CORE-01).
- **Confirm button foreground**: `ScaffoldColors.btnText` #000B18 on the accent fill (Phase 1 dark-on-green idiom). Cancel button: transparent, `textSecondary` Body label.

### Known constraint (documented, not fixed here)

`TextEntryFieldWidget` hardcodes `Colors.white` input text on a `grayPrimary` fill (`text_entry_field_widget.dart:18-25`). On the dark palette this is correct; on the light palette the fill is #F1F3F5 and typed text renders white-on-near-white (contrast failure). Scaffold `lib/` is a read-only submodule for this workstream — **do not fork or wrap-fix the atom**. Ship as-is, file the contrast fix against the scaffold workstream (`src/app/scaffold/.planning/`), and note it in the plan. The hint text (`palette.gray500`) is unaffected; the limitation applies to entered text in light mode only.

---

## Copywriting Contract

Voice (inherited): sentence case, no exclamation marks, no "Oops". Error copy always pairs a problem statement with a next step. All copy below is final — executor uses it verbatim.

| Element | Copy |
|---------|------|
| Primary CTA (dialog confirm) | **"Create space"** (verb + noun). Context variants, same button: "Create room" (room modes), "Save changes" (edit mode) |
| Dialog cancel | "Cancel" |
| Rail-header "+" semantic label | "Create space or room" (icon-only pressable — WCAG 4.1.2) |
| Space-node "+" semantic label | "New room in \<space name\>" |
| Space-node edit semantic label | "Edit space \<space name\>" |
| Dialog title — create from header (either type) | "New" (type-neutral — the drawer atom takes a static title per open; owner-approved IN-03 fix, supersedes the per-type "New space"/"New room" rows) |
| Dialog title — create from space node | "New room in \<space name\>" |
| Dialog title — edit | "Edit space" |
| Dialog type selector labels | "Space" / "Standalone room" (header-launch create mode only) |
| Name field hint | "Space name" or "Room name" per mode |
| Visibility radio labels | "Public" / "Private" (space modes only) |
| autoJoinRooms toggle label | "Auto-join rooms" |
| autoJoinRooms description (Label, `textSecondary`) | "You'll automatically join every room in this space. Rooms stay visible either way." |
| Name validation error (inline, on confirm with empty field) | "Enter a name." — dialog stays open, field shows error |
| Empty state — no spaces, heading | "No spaces yet" |
| Empty state — no spaces, body | "Spaces organize your rooms. Create your first space to get started." |
| Empty state — no spaces, action (accent button) | "Create space" |
| Empty state — expanded space with no rooms (inline row) | "No rooms yet" |
| Error toast — command publish failed (transport down), title | "Couldn't create space" / "Couldn't create room" / "Couldn't update space" |
| Error toast — publish failed, message | "The command didn't reach the chat core. Try again." |
| Error toast — C++ rejected the command (pushed ErrorNotice), title | Same title as above; message shows the pushed reason string |
| Success feedback | **No toast.** The dialog closes on confirm; the rail row appearing/updating under the pushed `SpaceTree` IS the confirmation (no local optimism, D-05) |
| Startup rail state | Loading skeleton — no copy |
| Destructive confirmation | **None in Phase 2** — no destructive actions ship. `statusError` token stays reserved |

**Superseded Phase 1 copy:** the Phase 1 "No spaces" empty state ("You're not a member of any spaces…") is replaced by the copy above — Phase 2 ships creation, so the empty state now carries the CTA instead of a dead end. Phase 1's "No rooms yet" rail-body empty state (no-creation wording) applies only when the entire catalog is empty and is superseded by the no-spaces state above; per-space "No rooms yet" is the inline row.

---

## Interaction Contracts

### Rail structure (D-05, D-21 inherited)

```
┌─────────────────────────────┐
│ Spaces            [＋]      │  ← toolbar: section-header style label +
│─────────────────────────────│    accent 40px circular "+" (48px hit area)
│ ▾ Space A          [✎][＋] │  ← space node row (48px): chevron, name,
│     general                  │    "Private" badge when private, edit, "+"
│     random                   │  ← child room rows: +12px indent, joined=
│     No rooms yet             │    tappable / unjoined=dimmed (40%)
│                             │
│   Rooms                     │  ← "Rooms" section header + space12 break;
│   lounge                    │    ENTIRE SECTION HIDDEN when no standalone
│   quiet                     │    rooms exist (no empty-section noise)
└─────────────────────────────┘
```

- **Toolbar** persists in every rail state (loading, empty, populated) — the creation affordance is never a dead end. Padding `space8`; label start-aligned, "+" end-aligned.
- **Rail width, background, divider** — unchanged Phase 1 contracts (280px expanded, `grayPrimary`, `borderSubtle` hairline).
- **"Rooms" section hidden when empty** — the section header renders only when at least one standalone room exists.

### Space node anatomy (resolves research Open Question 3)

Row layout, start→end: `chevron` → space name (Body, ellipsized) → `ScaffoldBadge` "Private" (only when `!isPublic`, gap `space2`) → spacer → edit icon (`Icons.edit_outlined`, `textSecondary`) → "+" icon (`Icons.add`, `textSecondary`), icons 40px circular with `space2` gap.

- **Whole-row tap toggles expansion** (spaces are not selectable in Phase 2 — no space-level view exists). Nested `ScaffoldPressable`s: the innermost gesture wins the arena, so trailing icon taps do not collapse the node.
- **Expansion animation uses the `ScaffoldDisclosure` motion vocabulary** (`AnimatedSize` at `ScaffoldMotionDurations.medium`, chevron `AnimatedRotation` at `ScaffoldMotionDurations.short`, zero-duration under `ScaffoldMotion.reducedMotion`). `ScaffoldDisclosure` itself is NOT used — its `title` is a `String` with no slot for badge/trailing actions — so the node composes the same primitives. Chevron tint `textSecondary` (no accent — expansion is navigation, not CTA).
- **Expansion state is view-local** (a `StatefulWidget` map of space id → bool), **never in `RailState`** — `RailState` mirrors pushed C++ truth only (D-04 Phase 1). New spaces default **expanded**. Expansion survives `setTree` full replacements (keyed by space id).

### Room row states (resolves research Open Question 1)

| State | Visual | Interaction |
|-------|--------|-------------|
| Joined + selected | `ScaffoldColors.btnFilterSelected` tint (accent @10%), `radiusMd`, `textPrimary` + icon | Taps are redundant-reselection no-ops (existing `selectRoom`) |
| Joined, not selected | No tint, `textSecondary` | Tap selects (existing `selectRoom` path — zero cubit change) |
| Catalog, not joined | `ScaffoldPressable(disabled: true)`: `textSecondary` + `ScaffoldDisabledOverlay` 0.40 | Non-tappable; `Semantics(enabled: false)` |

Join status derives from the pushed `RoomList` (topic membership), never local computation — the row simply looks up `rooms.contains(topic)`. When a toggle or `setRooms` push clears the active selection, the composer hint falls back to "Select a room to start messaging" (existing Phase 1 behavior).

### Rail state during startup (resolves research Open Question 4)

| Rail condition | Rendered state |
|----------------|----------------|
| Before first `SpaceTree` push (CRDT replay) | `ScaffoldStateView(state: 'loading')` — skeleton; toolbar still visible |
| First `SpaceTree` arrived, catalog empty | `ScaffoldStateView` empty variant with the no-spaces copy + "Create space" `emptyAction` |
| First `SpaceTree` arrived, catalog populated | Tree per structure above |
| Session error before readiness | Rail stays in loading skeleton; the error surfaces via the composer hint (Phase 1 WR-03) — **no separate rail error variant in Phase 2** |

The loading-vs-empty distinction requires a "tree received" flag alongside the tree in `RailState` (planner detail; the visual contract is: skeleton until the first push, never the empty state).

### The create/edit dialog (D-05)

One `StatefulWidget` (suggested `src/app/lib/shell/space_room_dialog.dart`), shown via `ResponsiveDrawer.show(context, title: …, children: […], footer: …)` — dialog ≥800px width, modal bottom sheet below (atom-fixed). Surface `deepBlueTertiary`, sheet corner 16px (atom-fixed).

**Modes** (same widget, fields adapt):

| Mode | Entry | Type selector | Fields |
|------|-------|---------------|--------|
| Create — header launch | Toolbar "+" | Shown: "Space" (default) / "Standalone room" radio pair | Name; when Space: visibility radios + autoJoin toggle |
| Create — space-node launch | Space row "+" | Hidden (fixed: room in that space) | Name only |
| Edit space | Space row edit icon | Hidden | Name (prefilled), visibility (prefilled), autoJoin (prefilled) |

**Field composition (verified atom APIs):**
- Name: `TextEntryFieldWidget(logic: TextFormFieldLogic(context, controller: …, hintText: 'Space name'/'Room name', autofocus: true, onFieldSubmitted: …))` — Enter submits. `maxLength: 64` trims runaway names.
- Visibility: two `ScaffoldSelectionIndicatorRadio` (bool-based atom — coordinated pair: `onChanged(true)` on one sets the other false in dialog state). Each radio + its Body label wrapped in one tappable `ScaffoldPressable` row (the whole label row toggles, not just the 48px indicator).
- autoJoinRooms: `ScaffoldSelectionIndicatorToggle(value:, onChanged:)` + Body label + Label description at `textSecondary`.
- Type selector: same bool-radio-pair pattern as visibility, space modes only.
- Field spacing `space4`; label-to-control `space2`; dialog chrome (title, padding) is `BottomDrawer`-owned — do not re-pad.

**Footer:** end-aligned row, `space4` gap — Cancel (transparent `ScaffoldPressable`, Body 400, `textSecondary`) then Confirm (accent-filled 40px-high pill, Body 500, `ScaffoldColors.btnText` foreground; `disabled: true` while the name field is empty).

**Behavior contract (D-05 locked):**
1. Confirm validates: empty name → inline "Enter a name.", dialog stays open.
2. Valid confirm publishes the `GcsCommand` (`createSpace` / `createRoom` / `updateSpace`) through the injected `GcsCommandTransport` and **closes immediately** — no spinner, no waiting for `SpaceTree`.
3. `publishCommand == false` → error toast (card density, `ToastType.error`), dialog stays open.
4. C++ rejection arrives later as a pushed ErrorNotice → error toast with the pushed reason.
5. **No local optimism**: the rail renders nothing new until the `SpaceTree` push lands; no success toast fires.
6. Toast density follows the atom's title semantics — errors carry a title (card), the phase defines no compact-pill successes.

**No room-edit affordance**: `RoomRecord` has no mutable config and the phase defines no `UpdateRoomCommand` — room rows carry no edit affordance. Room rename arrives in a later phase.

---

## Component Inventory (Phase 2)

All are existing scaffold atoms; the two new artifacts are plain-Dart shell widgets.

| Component | Source | Used for |
|-----------|--------|----------|
| `ResponsiveDrawer.show` | `components/bottom_drawer/responsive_drawer.dart` | Create/edit dialog shell (dialog ≥800px / bottom sheet below) |
| `TextEntryFieldWidget` + `TextFormFieldLogic` | `components/text_entry_field_widget.dart`, `text_form_field_logic.dart` | Dialog name field (known light-mode contrast constraint, see Color) |
| `ScaffoldSelectionIndicatorRadio` | `components/scaffold_selection_indicator_radio.dart` | Dialog type pair + visibility pair (bool-coordinated pairs; 48px touch targets; `inMutuallyExclusiveGroup` semantics in-atom) |
| `ScaffoldSelectionIndicatorToggle` | `components/scaffold_selection_indicator_toggle.dart` | autoJoinRooms |
| `ScaffoldBadge` | `components/scaffold_badge.dart` | "Private" text pill on private space nodes (`badgeColor: textSecondary`) |
| `ScaffoldPressable` | `components/scaffold_pressable.dart` | All rows and icon affordances; `disabled: true` for unjoined rooms; `semanticLabel` on every icon-only instance |
| `ScaffoldStateView` | `components/scaffold_state_view.dart` | Rail loading skeleton / no-spaces empty state (+ `emptyAction`) |
| `showToast` | `components/toast/toast_manager.dart` | Error feedback (card density, title, `ToastType.error`) |
| `ScaffoldSurface` | `components/scaffold_surface.dart` | Circular icon affordances + accent confirm pill (Phase 1 `_SendButton` composition idiom) |
| `ScaffoldMotionDurations` / `ScaffoldMotion` | `components/scaffold_motion.dart` | Space-node expansion animation vocabulary + reduced-motion gating |
| **`RoomRail` (extended)** | `src/app/lib/shell/room_rail.dart` | Toolbar + Spaces tree + standalone Rooms sections |
| **`SpaceRoomDialog` (new)** | `src/app/lib/shell/space_room_dialog.dart` | The one create/edit dialog (D-05) |

---

## State Contracts (backing the visuals)

- `RailState` evolves flat → tree per 02-RESEARCH Pattern 6: `spaces` (catalog, `SpaceTree` pushes) + `standaloneRooms` (catalog) + `rooms` (joined topics, `RoomList` pushes — unchanged) + `activeRoom` (topic string — unchanged). `RailCubit.setTree` is a full replacement mirroring `setRooms`; `setRooms`/`selectRoom` semantics are untouched (the disabled-row rendering makes the joined-only `selectRoom` guard visually honest instead of a silent no-op).
- `SessionCubit._dispatchEvent` gains the `hasSpaceTree` arm before existing arms (02-RESEARCH Pattern 6).
- View-local state (dialog form fields, space-node expansion) never enters Cubits.

---

## Registry Safety

shadcn registry does not apply (Flutter app, no `components.json`, no npm). Equivalent supply-chain surface:

| Registry | Blocks Used | Safety Gate |
|----------|-------------|-------------|
| Dart pub (pub.dev) | **none** — zero new packages this phase (02-RESEARCH Package Legitimacy Audit) | not required |
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

All atom APIs and token values verified against live reads on 2026-09-19:

- `.planning/workstreams/app/phases/01-foundation/01-UI-SPEC.md` — inherited spacing/typography/color/copy contracts
- `.planning/workstreams/app/phases/02-spaces-rooms/02-CONTEXT.md` — D-01..D-05 locked decisions
- `.planning/workstreams/app/phases/02-spaces-rooms/02-RESEARCH.md` — open questions 1/3/4 + Patterns 6/7; Standard Stack (zero new packages)
- `src/app/scaffold/design_tokens.json` — typography scale, shape corners
- `src/app/scaffold/lib/components/bottom_drawer/responsive_drawer.dart` — 800px breakpoint, `deepBlueTertiary` surface, 16px sheet corner, `footer` slot
- `src/app/scaffold/lib/components/text_entry_field_widget.dart` + `text_form_field_logic.dart` — field API; hardcoded-white light-mode constraint (lines 18-25)
- `src/app/scaffold/lib/components/scaffold_selection_indicator_radio.dart` / `_toggle.dart` — bool-based value/onChanged/disabled; 48px touch targets; 0.40 disabled opacity
- `src/app/scaffold/lib/components/scaffold_badge.dart` — text variant, `badgeColor`, luminance-resolved on-status colors
- `src/app/scaffold/lib/components/scaffold_state_view.dart` — loading/empty variants, `emptyAction`
- `src/app/scaffold/lib/components/toast/toast_manager.dart` — title ⇒ card / no-title ⇒ pill density; `ToastType.error`
- `src/app/scaffold/lib/components/scaffold_pressable.dart` — `disabled` (overlay + semantics), `semanticLabel` requirement for icon-only
- `src/app/scaffold/lib/components/scaffold_disclosure.dart` — motion vocabulary (AnimatedSize/AnimatedRotation, `ScaffoldMotionDurations`, reduced-motion); String-title limitation
- `src/app/lib/shell/room_rail.dart`, `gcs_shell.dart`, `src/app/lib/cubits/rail_cubit.dart` — shipped Phase 1 row/section/selection patterns these contracts extend
