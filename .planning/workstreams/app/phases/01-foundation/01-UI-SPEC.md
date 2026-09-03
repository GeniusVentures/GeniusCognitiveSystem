---
phase: 1
slug: foundation
status: draft
shadcn_initialized: false
preset: none
created: 2026-08-21
---

# Phase 1 — UI Design Contract

> Visual and interaction contract for the GCS chat app shell (Flutter/Dart). The design system is **Material 3 via the `frontend_scaffold` widget library** (in-tree submodule at `src/app/scaffold/`), not shadcn. shadcn's registry/web model does not apply here — this is a Flutter app and D-10..D-22 lock the scaffold as the sole UI kit.
>
> All token values below are pinned against `src/app/scaffold/design_tokens.json`, `src/app/scaffold/lib/theme/scaffold_dimens.dart` (`ScaffoldDimens.defaultDimens`), and `src/app/scaffold/lib/theme/scaffold_palette.dart` (`ScaffoldPalette.lightPalette` / `ScaffoldPalette.defaultPalette`). **Do not invent values.** Where a chat-domain mapping is required and not already in scaffold, it is stated explicitly as a contract line.

---

## Design System

| Property | Value |
|----------|-------|
| Tool | none (Flutter Material 3 — scaffold is the design system) |
| Preset | not applicable |
| Component library | `frontend_scaffold` (in-tree git submodule, `src/app/scaffold/`, package `package:frontend_scaffold`) — pinned at `ef16a0c` per D-13 |
| Icon library | Flutter material `Icons` (default; no separate icon package added in Phase 1) |
| Font | Material default (Roboto on Android/desktop, SF on iOS/macOS) — scaffold's `scaffold_theme.dart` does not override font; we inherit M3 defaults. No custom font shipped in Phase 1. |

**Source of truth (D-12):** `src/app/scaffold/design_tokens.json` + `src/app/scaffold/lib/theme/`. The host app (`src/app/lib/`) registers `ScaffoldPalette.lightPalette` on the light `ThemeData` and `ScaffoldPalette.defaultPalette` on the dark `ThemeData`, with `ScaffoldDimens.defaultDimens` on both, per `scaffold_theme.dart`'s `scaffoldThemeExtensions` pattern. The legacy hardcoded purple `ColorScheme.fromSeed` in `src/app/lib/main.dart` is deleted (D-12).

**App shell (D-11, D-21):**
- Screens follow scaffold's `AppScreenView` pattern (`src/app/scaffold/lib/components/app_screen_view.dart`).
- Per-screen Cubits per D-11; a session Cubit owns the FFI session handle (D-01/D-04).
- One-off screens (chat shell, space/room rail, message pane) live in `src/app/lib/` as plain Dart — ai-boss consumer pattern.
- Multi-variant chat composites (message bubble family) are **generated from our own Jinja2 templates + `_vars.json`** via scaffold `engine.py`, not hand-switched Dart (D-17). Templates and vars live in `src/app/templates/`; generated output lands under `src/app/lib/generated/` and is **never hand-edited**.

---

## Spacing Scale

Source: `ScaffoldDimens.defaultDimens` (`src/app/scaffold/lib/theme/scaffold_dimens.dart:115-137`). Tokens are named by **2px-base step index** (so `space8` = 16pt, not 8pt). The resolved values, not the subscripts, are the contract.

| Token | Value | Usage in chat shell |
|-------|-------|---------------------|
| `space2` | 4px | Inline padding inside bubbles; gap between avatar and bubble edge; badge-to-text gap in `ScaffoldComposer.badgeRow` |
| `space3` | 6px | Tight in-row gaps in rail items (icon to space/room label) |
| `space4` | 8px | Standard intra-component gap; `ScaffoldComposer` row separation; bubble vertical stacking gap inside a message group |
| `space6` | 12px | Message-flow horizontal inset (gutter between bubble and pane edge); rail item horizontal padding |
| `space8` | 16px | `itemSpacing` — standard spacing between stacked messages from different senders; rail section padding; pane padding around `ScaffoldComposer` |
| `space12` | 24px | Section breaks inside the rail (between spaces); gap between message groups when timestamp separators appear |

**Layout paddings (from `ScaffoldDimens.defaultDimens`):**
- `horizontalPadding` 20px — phone pane padding
- `horizontalDesktopPadding` 40px — desktop pane padding
- `verticalDesktopPadding` 40px — desktop vertical padding
- `itemSpacing` 16px — vertical gap between top-level message items in the flow

**Radii (from `ScaffoldDimens.defaultDimens` + `design_tokens.json shape.corner`):**
- `radiusMd` 12px — message bubble corner radius; `ScaffoldComposer` surface corner radius (matches M3 `medium`)
- `borderRadiusCard` 15px — rail space-card corner radius (legacy scaffold default; do NOT round to 16)
- `radiusPill` 48px — send button / pill actions in the composer action row (matches M3 `extraLarge`-adjacent pill)
- `borderRadiusButton` 48px — pill-shaped buttons
- `skeletonCornerRadius` 4px — message skeleton (pending) corner

**Touch & a11y:**
- `minTouchTarget` 48px — every rail item, send button, and bubble action must clear this
- `touchTargetPadding` 12px — internal padding to lift small content up to the touch target
- `focusRingWidth` 2px — focus ring stroke, driven by `ScaffoldFocusOutline` (used by `ScaffoldComposer`)
- `dragHandleSize` 24px — only if a rail drag handle appears (not in Phase 1 scope)

**Exceptions:**
- `appBarHeight` 65px (scaffold default) — only used if a top app bar is rendered; Phase 1 shell does not add one beyond what `AppScreenView` provides.
- `disabledOverlayOpacity` 0.40 — applied via `ScaffoldDisabledOverlay` when composer is disabled (e.g. session not yet initialized).

---

## Typography

Source: `design_tokens.json typography.scale`. The scaffold does not redeclare these in Dart — the host `ThemeData.textTheme` is built from these token values. We use **4 sizes** total for the chat shell, drawn directly from the M3 scale:

| Role | Size | Weight | Line Height | Letter Spacing | Usage |
|------|------|--------|-------------|----------------|-------|
| Display | 57px | 400 (regular) | 64px (≈1.12) | -0.25 | Reserved — not used in Phase 1 shell |
| Headline | 32px | 400 (regular) | 40px (1.25) | 0 | Space name at the top of the rail |
| Title | 22px | 500 (medium) | 28px (≈1.27) | 0 | Room name in the active-pane header; section titles inside the rail |
| Body | 14px | 400 (regular) | 20px (≈1.43) | 0.25 | **Message bubble text; composer input; rail item labels; empty/error state body copy** |
| Label | 11px | 500 (medium) | 16px (≈1.45) | 0.5 | Bubble timestamp; sender name above peer bubbles; composer hint text |

**Declared weights:** exactly two — `400` (regular) and `500` (medium). No semibold/bold anywhere in Phase 1.

**Contract lines:**
- Message bubble body uses **Body** (14/400/20px lh).
- Peer sender label above a bubble uses **Label** (11/500/16px lh), tinted `textSecondary`.
- Self bubbles render no sender label (already visually owned by alignment + color).
- Composer hint text uses **Label** at `textSecondary` color.
- Timestamps use **Label** at `textSecondary`.
- Rail item (room name) uses **Body**; rail space header uses **Title**.

---

## Color

Source: `ScaffoldPalette.lightPalette` (light theme) + `ScaffoldPalette.defaultPalette` (dark theme), both registered per D-22 via the scaffold light/dark toggle. **60/30/10 split** below is enforced against the light palette; the dark palette mirrors it via the same token names.

### Light palette (60/30/10)

| Role | Token | Value | Usage |
|------|-------|-------|-------|
| Dominant 60% | `lightPalette.grayPrimary` (rail/pane base) → `#F1F3F5` | Background of the rail + main pane (the bulk surface) |
| Dominant 60% (top) | `lightPalette.surfaceElevated` → `#FFFFFF` | `ScaffoldComposer` surface; sheet-level surfaces; rail space cards |
| Secondary 30% | `lightPalette.deepBlueTertiary` → `#E9EDF2` | Peer message bubble fill; secondary rail rows; inactive state views |
| Secondary 30% (cards) | `lightPalette.deepBlueCardColor` → `#FFFFFF` | Rail space card surfaces (matches `surfaceElevated` in light) |
| Accent 10% | `lightPalette.lightGreenPrimary` → `#00EAAE` | Self-bubble fill; composer focus ring; selected-room indicator in rail; primary CTA |
| Accent 10% (secondary) | `lightPalette.lightGreenSecondary` → `#01CC95` | Send icon tint; composer action-row icon tint |
| Destructive | `lightPalette.statusError` → `#D13438` | Message error state accent; error toast; destructive confirm |
| Success | `lightPalette.statusSuccess` → `#0AA06B` | "Sent" indicator next to self bubbles (optional in Phase 1) |
| Warning | `lightPalette.statusWarningText` → `#9A6A00` | "Pending" indicator text next to self bubbles (foreground-purposed) |
| Text primary | `lightPalette.textPrimary` → `#17191E` | Bubble body, room names, composer input text |
| Text secondary | `lightPalette.textSecondary` → `#5A6070` | Timestamps, peer sender labels, hint text, empty-state body |
| Border subtle | `lightPalette.borderSubtle` → `#1F000000` (12% black) | Composer surface 1px border; hairline between rail and pane |
| Border grey | `lightPalette.borderGrey` → `#1F000000` (12% black) | Enabled input borders (same value as borderSubtle in light) |

### Dark palette (60/30/10)

| Role | Token | Value | Usage |
|------|-------|-------|-------|
| Dominant 60% | `defaultPalette.grayPrimary` → `#151E29` | Rail + main pane base |
| Dominant 60% (top) | `defaultPalette.surfaceElevated` → `#0C0E14` | `ScaffoldComposer` surface |
| Secondary 30% | `defaultPalette.deepBlueTertiary` → `#05090F` | Peer bubble fill; deepest surfaces (drawers, sheets) |
| Secondary 30% (cards) | `defaultPalette.deepBlueCardColor` → `#0A121F` | Rail space card surfaces |
| Accent 10% | `defaultPalette.lightGreenPrimary` → `#00EAAE` | Self-bubble fill; focus ring; selected-room indicator |
| Accent 10% (secondary) | `defaultPalette.lightGreenSecondary` → `#01CC95` | Send icon tint |
| Destructive | `defaultPalette.statusError` → `#FF4D4D` | Error accents |
| Success | `defaultPalette.statusSuccess` → `#0AD89C` | Sent indicator |
| Warning | `defaultPalette.statusWarningText` → `#FFC42E` | Pending indicator |
| Text primary | `defaultPalette.textPrimary` → `#FFFFFF` | Bubble body, room names |
| Text secondary | `defaultPalette.textSecondary` → `#8A8F9D` | Timestamps, peer labels, hint |
| Border subtle | `defaultPalette.borderSubtle` → `#1FFFFFFF` (12% white) | Composer surface border; rail/pane divider |
| Border grey | `defaultPalette.borderGrey` → `#4DFFFFFF` (30% white) | Enabled input borders |

### Chat-domain color mapping (contract lines — not in scaffold, declared here)

- **Self bubble fill** = `lightGreenPrimary` (accent). Body text on self bubble uses `ScaffoldColors.btnText` `#000B18` (the dark-on-green readable foreground scaffold already defines).
- **Peer bubble fill** = `deepBlueTertiary` (secondary). Body text = `textPrimary`.
- **System bubble fill** = transparent with `borderSubtle` 1px outline. Body text = `textSecondary`. Centered horizontally.
- **Error bubble overlay** = `statusError` at 12% alpha (match `dropZoneRejected` alpha pattern `#33D13438` / `#33FF4D4D`) painted over the role fill; error text/icon = `statusError`.
- **Pending bubble** = role fill at 40% opacity (`disabledOverlayOpacity` 0.40 from `ScaffoldDimens`) — matches the existing scaffold "disabled/dim" pattern.
- **Thinking bubble** = peer fill (`deepBlueTertiary`) containing a 3-dot typing indicator; dots tint `textSecondary`; dot color uses `lightGreenPrimary` for the leading/active dot, `blue500`, `lightGreenSecondary` per `defaultPalette` token names. (No scaffold typing-indicator atom exists yet — expected Phase 9+ per scaffold roadmap; colors are existing palette tokens.)
- **Streaming bubble** = peer fill, with text appended as it arrives; no additional chrome — the cursor is the trailing token boundary.
- **Selected room in rail** = `lightGreenPrimary` at 10% alpha background (`ScaffoldColors.btnFilterSelected` already computes this) + `textPrimary` label.
- **Hover rail row** = `borderSubtle` 6% alpha overlay (uses existing token, no new value).

### Accent reserved for

Accent (`lightGreenPrimary`) is reserved for **exactly these** elements in Phase 1:
1. Self-message bubble fill
2. Selected-room indicator in the left rail
3. Composer focus ring (via `ScaffoldFocusOutline` bound to `focusRingColor`, which is seeded from `lightGreenPrimary`)
4. Send button background (composer action row)
5. Active typing indicator dot (leading dot in thinking state)

Accent is **not** used for: peer bubbles, rail row default text, system bubbles, error states, link text (no links in Phase 1), timestamps.

### Destructive reserved for

`statusError` is reserved for: message-send failure indicator on a bubble, error toast background accent, destructive confirmation button in any "leave room" or "delete message" flow. **No destructive actions ship in Phase 1** (room/space management is Phase 2+), but the token is declared here so future phases cannot drift.

---

## Copywriting Contract

Phase 1 ships a **skeleton shell** — the user can navigate spaces/rooms and see message history once Phase 2/3 lands. Phase 1's UI exercises the shell and composer against the FFI smoke path.

| Element | Copy |
|---------|------|
| Primary CTA (composer) | **"Send"** — verb only (no noun; the input itself is the noun). Send button uses an `Icons.send` icon; accessible label `"Send message"`. |
| Composer hint text | `"Message #<room-name>"` — Slack-style, room-interpolated. Fallback when no room is active: `"Select a room to start messaging"`. |
| Empty state (no rooms in space) heading | `"No rooms yet"` |
| Empty state (no rooms in space) body | `"This space doesn't have any rooms. Rooms will appear here once they're created."` (Phase 1 does not expose room creation; do not promise a CTA that doesn't exist.) |
| Empty state (no messages in room) heading | `"No messages yet"` |
| Empty state (no messages in room) body | `"Send the first message to get the conversation started."` |
| Empty state (no spaces) heading | `"No spaces"` |
| Empty state (no spaces) body | `"You're not a member of any spaces. Spaces you join will appear here."` |
| Error state (FFI session init failed) | `"Couldn't start the chat session. Restart the app to try again."` |
| Error state (message send failed) | Bubble-level: `"Not sent. Tap to retry."` — inline, not a toast. |
| Error state (room load failed) | `"Couldn't load this room. Pull to refresh or pick a different room."` |
| Destructive confirmation | None in Phase 1 — no destructive actions ship. Token reserved (see Color). |
| Pending send indicator | No copy — visual state only (40% opacity bubble). |
| Streaming indicator | No copy — text appends visibly. |
| Thinking indicator | No copy — typing-dots animation only. |

**Voice:** sentence case, no exclamation marks, no "Oops". Error copy always pairs a problem statement with a next step.

---

## Component Inventory (Phase 1)

All components below are **scaffold atoms** (`package:frontend_scaffold/...`) except the multi-variant chat composites, which are **generated from our own Jinja2 templates** per D-17.

| Component | Source | Used for |
|-----------|--------|----------|
| `AppScreenView` | `package:frontend_scaffold/components/app_screen_view.dart` | Top-level screen scaffold for the chat shell |
| `ScaffoldComposer` | `package:frontend_scaffold/components/scaffold_composer.dart` | Message input row at the bottom of the active pane; `onSubmit(String)` forwards to the FFI session Cubit |
| `ScaffoldSurface` | `package:frontend_scaffold/components/scaffold_surface.dart` | Card surfaces in the rail (space cards) |
| `ScaffoldStateViewCubit` | `package:frontend_scaffold/components/scaffold_state_view_cubit.dart` (family) | Empty/error states for rail and message pane |
| `ScaffoldFocusOutline` | `package:frontend_scaffold/components/scaffold_focus_outline.dart` | Composer focus ring (consumed internally by `ScaffoldComposer`) |
| `ScaffoldDisabledOverlay` | `package:frontend_scaffold/components/scaffold_disabled_overlay.dart` | Composer disabled state (session not initialized) |
| **Message bubble composite** | generated: `src/app/lib/generated/chat_message_bubble.dart` | Text-only bubble; variants = roles × states |
| **Message code-block composite** | generated: `src/app/lib/generated/chat_message_code_block.dart` | Wraps `ScaffoldCodeBlock` when it lands (deferred — Phase 1 has no code content; template skeleton only) |
| **Message media composite** | generated: `src/app/lib/generated/chat_message_media.dart` | Wraps media payload (deferred — Phase 1 has no media content; template skeleton only) |
| **Message flow composite** | generated: `src/app/lib/generated/chat_message_flow.dart` | Interleave-capable list rendering the three message composites via per-item variant data |

**Generated composite variant axes** (D-17..D-20, drives `_vars.json`):

- **Role axis** (D-18): `user-self` | `user-peer` | `assistant` | `system`
  - `user-self` and `user-peer` share role `user`; `self` is a variant flag, not a separate role.
  - `assistant` is the GCS bot.
  - `system` is join/leave/moderation notices.
- **State axis** (D-19): `pending` | `streaming` | `thinking` | `complete` | `error`
  - `thinking` is a distinct visual state — typing-dots indicator, no tokens yet.
  - `error` is a state, not a role.
- **Payload axis** (D-20): text bubble only in Phase 1. Code block and media composites are separate top-level items in the flow envelope, never wrapped in bubble chrome.

**Generated composite contract lines:**

- Bubble alignment: `user-self` aligns **end** (right on LTR), `user-peer` aligns **start** (left on LTR), `assistant` aligns **start**, `system` aligns **center**.
- Bubble max width: 78% of pane width on phone, 560px on desktop. (Not a scaffold token — chat-domain constant, declared here.)
- Bubble corner radius: `dimens.radiusMd` (12px). **Tail behavior:** on a run of consecutive bubbles from the same sender, only the last bubble in the run carries the full radius on the sender side; earlier bubbles in the run use `skeletonCornerRadius` (4px) on the sender-side bottom corner (iMessage-style tail). This is a chat-domain rule, not a scaffold token.
- Bubble internal padding: `space4` (8px) vertical, `space6` (12px) horizontal.
- Inter-bubble gap within a sender run: `space2` (4px). Between sender runs: `space8` (16px) (uses `itemSpacing`).
- Peer sender label appears above the **first** bubble of a peer run only; spacing from label to bubble = `space2` (4px).
- Timestamp appears below the **last** bubble of a run; spacing = `space2` (4px).

**Composer contract lines:**

- `ScaffoldComposer.hintText` = `"Message #<room-name>"` (see Copywriting).
- `ScaffoldComposer.onSubmit` forwards the raw string to the session Cubit's `sendMessage(String)`; the composer itself holds no submission logic (D-07 in scaffold).
- `ScaffoldComposer.disabled` is bound to the session Cubit's `isReady` flag — when FFI session is not yet initialized, the composer is dimmed via `ScaffoldDisabledOverlay` at `disabledOverlayOpacity` 0.40.
- `ScaffoldComposer.actionRow` contains exactly one item in Phase 1: the send button (circular, `radiusPill`, fill `lightGreenPrimary`, icon `Icons.send` tinted with `ScaffoldColors.btnText` `#000B18`).
- `ScaffoldComposer.badgeRow` is empty in Phase 1 (no attachments).
- Composer horizontal inset inside the pane: `space8` (16px) on phone, `space12` (24px) on desktop. Bottom inset: `space6` (12px).

**Rail contract lines:**

- Rail width: 240px collapsed, 280px expanded (desktop only). Phone layout replaces the rail with a drawer. (Not scaffold tokens — chat-domain constants.)
- Rail background: `grayPrimary`.
- Rail item height: 40px (above `minTouchTarget` 48px when combined with internal padding — internal vertical padding is `space2` + `space2`, total 40 + 8 = 48px).
- Space section header inside rail uses **Title** typography at `textSecondary`, uppercase, letterSpacing 0.5.
- Room row: `space6` horizontal padding, `space2` vertical padding, Body typography, `textPrimary` when active / `textSecondary` when inactive.
- Selected room: see Color section.

**Message flow contract lines:**

- The flow is a `CustomScrollView` anchored to the bottom (latest message visible without scrolling).
- Flow padding: horizontal `space8` (16px) phone / `space12` (24px) desktop; top `space8`, bottom `space6` (so the last bubble clears the composer by 12px).
- The flow renders heterogeneous items per D-20: `[text bubble][code block][text bubble][media]…`. Item-type dispatch is driven by per-item variant data delivered via FFI; the flow does not sniff payloads.
- Scroll-to-bottom button appears when the user scrolls up more than 200px and new messages arrive; positioned `space6` above the composer, end-aligned. (Contract line; uses a 40px circular `ScaffoldSurface`.)

---

## Cubit Architecture (D-11)

- **Session Cubit** (`SessionCubit`): owns the FFI session handle (D-01/D-04). Lifecycle: `init()` on app start → `shutdown()` on dispose. Exposes `isReady`, room list stream, active-room stream, `sendMessage(String)`, `selectRoom(roomId)`. Lives at app root.
- **Per-screen Cubits** (D-11):
  - `RailCubit` — spaces/rooms tree state; subscribes to `SessionCubit.rooms`.
  - `MessageFlowCubit` — active room's message list; subscribes to `SessionCubit.activeRoomMessages`.
  - `ComposerCubit` — composer disabled state; subscribes to `SessionCubit.isReady`.
- **State ownership rule (D-04):** C++ owns the source of truth. Cubits are projections of FFI-pushed state (D-05: `Dart_PostCObject` → `ReceivePort`). Cubits never synthesize message state locally — pending/streaming/error transitions are driven by FFI events, with the Cubit only mapping FFI payloads to the variant axes above.

---

## Registry Safety

shadcn registry does not apply (this is a Flutter app, no `components.json`, no npm registry). The equivalent supply-chain surface is **Dart pub packages** and the **scaffold submodule pin**.

| Registry | Blocks Used | Safety Gate |
|----------|-------------|-------------|
| Dart pub (pub.dev) | `ffi: ^2.2.0` (already in `src/app/pubspec.yaml`); `frontend_scaffold` via `path: scaffold` (in-tree submodule, not a pub package) | not required — both are in-tree or first-party Dart |
| Scaffold submodule | pinned at `ef16a0c` per D-13 (53-commit bump from `1cd3759` on 2026-08-20) | pin reviewed in CONTEXT.md; tracked via git submodule, not registry |
| `flutter_chat_ui` / `flutter_chat_core` | **removed** per D-10 | removal executed in the 01-05 Dart rewrite |

No third-party Jinja2 templates or external codegen registries are consumed. The scaffold `engine.py` runs from the in-tree submodule against in-tree templates (`src/app/templates/` and `src/app/scaffold/templates/`).

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

All token values verified against live reads on 2026-08-21:

- `src/app/scaffold/design_tokens.json` — M3 color roles, typography scale (display/headline/title/body/label), shape.corner, elevation levels
- `src/app/scaffold/lib/theme/scaffold_dimens.dart:115-137` — `ScaffoldDimens.defaultDimens` (all spacing, radii, touch-target, overlay-opacity values quoted above)
- `src/app/scaffold/lib/theme/scaffold_palette.dart:103-126` — `ScaffoldPalette.defaultPalette` (dark)
- `src/app/scaffold/lib/theme/scaffold_palette.dart:132-155` — `ScaffoldPalette.lightPalette` (light)
- `src/app/scaffold/lib/theme/scaffold_colors.dart` — raw `ScaffoldColors.*` constants (incl. `btnText #000B18`, `btnFilterSelected`)
- `src/app/scaffold/lib/theme/scaffold_theme.dart` — `scaffoldThemeExtensions` registration pattern
- `src/app/scaffold/lib/components/scaffold_composer.dart` — `ScaffoldComposer` API surface (hint/badge/action/onSubmit/disabled/focusNode)
- `src/app/scaffold/lib/components/app_screen_view.dart` — `AppScreenView` body/footer shell
- `.planning/workstreams/app/phases/01-foundation/01-CONTEXT.md` — D-10..D-22 (locked decisions)
- `.planning/workstreams/app/phases/01-foundation/01-RESEARCH.md` — verified scaffold pin, consumer pattern, Cubit-pattern atoms
