---
title: Define GCS bot participant identity mapping
date: 2026-08-15
priority: high
context: gs-explore --ws app
---

# Define GCS bot participant identity mapping

How does the GCS bot participant in a chat room map to the ELM router's specialist selection?

## Questions to resolve

- Is there one GCS bot per room, or does the bot multiplex across rooms?
- How does a `@gcs` mention route to the correct specialist (knowledge, code, creative)?
- Does the bot participant have a persistent identity (same ID across rooms) or ephemeral per-room?
- How does the bot's auto-answer policy interact with specialist selection?

## Dependencies

- ELM router from GNUS-NEO-SWARM (Phase 7)
- Chat room participant model

## Acceptance

Identity mapping documented, with clear flow from mention → specialist selection → response generation.
