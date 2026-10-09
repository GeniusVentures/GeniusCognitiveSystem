# Genius Cognitive System

## Product & Technical Design Specification

The **Genius Cognitive System (GCS)** is an integrated distributed cognitive platform. The **Genius Expert Language Model (Genius ELM)** provides the semantic inference core within a broader architecture for orchestration, verification, memory, specialized agents, distributed execution, swarm cognition, and coordinated cognitive evolution.

This documentation is a combined product requirements document, technical design document, and system architecture blueprint.

## Start here: Two ways to use GNUS.ai

**GNUS is both distributed compute and a cognitive application platform.** Low-level clients can submit compute jobs; planned OpenAI-compatible clients can request inference and GCS cognitive services with familiar API request and response shapes. GCS selects the Semantic Core, specialist models, GAML memory, and checks, then executes locally, privately, or through SuperGenius nodes when appropriate.

Read the [Developer API and Distributed Compute Bridge](developer-api-and-compute-bridge.md) for the short architecture, commercial opportunity, and **pricing-status distinctions**. The [OpenAI-compatible API specification](openai-compatible-api-router-and-gcs-job-queue.md) is detailed architecture; the public gateway is **planned v1.1 Phase 6**, not a live service.

**Cost references:** The SuperGenius owner resolved **$0.0003 per funded processing-hour** for planned **ELM jobs** on 2026-08-26 ([Phase 13 / issue #369](https://github.com/GeniusVentures/SuperGenius/issues/369)); the rate is an agreed design choice, **not yet deployed ELM-hour metering or finalized OpenAI API retail pricing**. Existing general-processing jobs already fetch the GNUS/USD price and reserve GNUS in escrow using a separate USD-per-estimated-FLOP calculation; its `dimensions.block_len` input is not always measured bytes/FLOPs. Earlier $0.005/node-hour documents and throughput-per-dollar comparisons use different units. See [GNUS pricing methodology](https://docs.gnus.ai/about-gnus.ai/features-and-benefits/pricing-methodology/).

## Reviewer and investor technical entry points

Start with [Architecture at a Glance](ARCHITECTURE-AT-A-GLANCE.md), then [Implementation Status](IMPLEMENTATION-STATUS.md), [Technical Diligence FAQ](TECHNICAL-FAQ.md), [Performance Benchmark Evidence](PERFORMANCE-BENCHMARKS.md) and [Document Index](DOCUMENT-INDEX.md). They separate architecture from **working source, test coverage, public deployment and independent audit**. GCS Chat, NEO-SWARM and the full GCS system are different workstreams with different completion states.

## Architecture documentation

Use the left navigation to browse the generated architecture index and source-reference documentation.

A good starting path is the executive summary and system overview, followed by the model, routing, consensus, grounding, memory, distributed swarm thinking, the Context Lifecycle, Caching, and Governance contract, secure-agent architecture, the retraining layers, cognitive evolution coordination, epistemic arbitration, SGFP4, Objective Memory / VTG, speculative decoding, Frozen Micro-MTP, the OpenAI-compatible API router, Local Cognitive Second Brain Mode, Forecast-Driven Cognition, the Execution Integrity System, the GCS Capability System, and the Agent and Module Development Inventory.
