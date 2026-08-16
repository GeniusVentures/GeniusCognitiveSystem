# Genius Cognitive System — App Workstream

## What This Is

Multi-party group chat application built on the Genius Cognitive System (GCS). Users create spaces and rooms, invite participants, and chat with both humans and AI specialists. The GCS system participates as a bot that can auto-answer questions or respond to mentions.

## Core Value

Users can create a space, invite others, and have a group conversation where an AI participant (GCS) responds to questions — all synchronized via CRDT without central servers.

## Current Milestone: v1.0 GCS Chat

**Goal:** Build a working multi-party chat with spaces, rooms, membership, and GCS bot integration.

**Target features:**
- Space and room creation with configurable inheritance
- CRDT-based message sync (append-only, tombstone moderation)
- Membership roles (Super Admin, Admin, Moderator, Member, Guest)
- Invite system with capability tokens
- GCS bot participant with auto-answer policies
- Public lobby for space discovery

## Requirements

### Validated

(None yet — ship to validate)

### Active

- [ ] **CHAT-01**: User can create a space (public or private)
- [ ] **CHAT-02**: User can create rooms within a space or standalone
- [ ] **CHAT-03**: User can invite others via capability tokens
- [ ] **CHAT-04**: User can send and receive messages in real-time
- [ ] **CHAT-05**: User can @mention the GCS bot for AI responses
- [ ] **CHAT-06**: Moderators can delete messages and manage members
- [ ] **CHAT-07**: Public spaces appear in lobby for discovery

### Out of Scope

| Feature | Reason |
|---------|--------|
| Video/audio streaming | Deferred to genius-tube extension |
| End-to-end encryption | MVP uses app-layer symmetric keys only |
| Message editing | Append-only CRDT for MVP |
| File attachments | Text-only for MVP |
| Mobile push notifications | Desktop-first |

## Context

- **Architecture:** See `.planning/notes/gcs-chat-architecture.md`
- **C++ Core:** `src/lib/gcs_core.cpp` (GCS integration, FFI to Flutter)
- **Flutter UI:** `src/app/` (Cubit state management, scaffold widgets)
- **FFI Bridge:** `src/ffi/` (Dart → C++ bindings)
- **Widget Library:** `src/app/scaffold/` (composite widgets, Jinja2 templates)
- **Extension:** genius-tube will consume this as `gcs-chat-core` dependency

## Constraints

- **Tech stack:** C++17 (no C++20 features), Flutter/Dart, libp2p GossipSub, GlobalDB CRDT
- **No central servers:** All state sync via CRDT over IPFS pubsub
- **Platform:** macOS first, then Linux, Windows, iOS, Android

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Single unified room model with `autoAnswer` policy | Simplifies state; Solo/Multi/Q&A are just config | — Pending |
| App-layer encryption (not libp2p PSK) | Per-room keys, easier rotation, existence visible | — Pending |
| Spaces as containers with `autoJoinRooms` config | Flexible: boundary or loose grouping | — Pending |
| Multiple Admins, no single Owner | Avoids "who owns this" problem, enables succession | — Pending |
| Super Admin = creator, cannot be demoted | Prevents rogue admin lockout | — Pending |
| Destructive actions need Super Admin approval when 2+ admins | Balance between agility and safety | — Pending |

## Evolution

This document evolves at phase transitions and milestone boundaries.

**After each phase transition** (via `/gsd-transition`):
1. Requirements invalidated? → Move to Out of Scope with reason
2. Requirements validated? → Move to Validated with phase reference
3. New requirements emerged? → Add to Active
4. Decisions to log? → Add to Key Decisions
5. "What This Is" still accurate? → Update if drifted

**After each milestone** (via `/gsd:complete-milestone`):
1. Full review of all sections
2. Core Value check — still the right priority?
3. Audit Out of Scope — reasons still valid?
4. Update Context with current state

---
*Last updated: 2026-08-15 after milestone v1.0 creation*
