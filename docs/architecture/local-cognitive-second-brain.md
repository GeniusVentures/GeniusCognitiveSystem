# **25. Local Cognitive Second Brain Mode**

## **25.1 Purpose**

This document defines **Local Cognitive Second Brain Mode** for **GeniusCognitiveSystem (GCS)**.

Local Cognitive Second Brain Mode allows GCS to operate as a private, user-owned memory and reasoning system on a local device, workstation, SMB appliance, enterprise node, or private GNUS subnet.

This mode supports personal, SMB, and enterprise workflows where the system needs to remember evolving context, prepare the user for meetings, track commitments, maintain project state, detect contradictions, and adapt over time while keeping private memory inside the configured privacy boundary.

The core architectural rule is:

> The local second brain is a **GCS agent mode** backed by **GAML**, executed by **local or private ELMs**, coordinated by the **orchestration layer**, and improved over time through **EGGROLL adaptation signals**.

It is a local cognitive operating mode inside the broader GCS architecture.

---

## **25.2 Architectural Position**

Local Cognitive Second Brain Mode uses five GCS components:

* **Orchestration Layer:** control plane
* **GAML:** structured memory substrate
* **Local or Private ELMs:** reasoning engine
* **Second Brain Agent:** behavior layer
* **EGGROLL:** adaptation loop

Together, these components provide private memory, local reasoning, tool execution, verification, writeback, and adaptation.

---

## **25.3 Orchestration Layer Role**

The orchestration layer supervises second-brain execution.

It decides:

* whether a request stays local, uses private enterprise resources, or escalates to the public swarm
* which memory scope is allowed
* which ELM, agent chain, tool, grounding source, and validation path should be used
* whether the task requires arbitration, consensus, secure execution, or writeback
* whether EGGROLL adaptation signals may be emitted

The orchestration layer controls how the second-brain workflow runs.

---

## **25.4 GAML Role**

**GAML** is the memory substrate for Local Cognitive Second Brain Mode.

GAML stores structured long-term memory, including:

* facts
* claims
* commitments
* decisions
* deadlines
* tasks
* preferences
* user profile signals
* project state
* source references
* contradictions
* reasoning traces

GAML supports:

* structured memory objects
* multi-hop reasoning over memory objects
* temporal coherence
* confidence scoring
* source grounding
* version-aware writeback
* contradiction tracking
* private and local memory scopes

This allows the local second brain to track state across conversations, documents, meetings, and workflows.

---

## **25.5 Local and Private ELM Role**

Local or private **Expert Language Models (ELMs)** perform reasoning over private memory.

They may run on:

* a Mac or PC
* a Jetson or edge device
* a local GNUS node
* a workstation
* an SMB AI appliance
* an enterprise private subnet

These ELMs handle tasks such as:

* meeting preparation
* daily briefs
* project recall
* document drafting
* personal search
* task planning
* workflow assistance
* private decision support

The local ELM receives compact, structured context packets assembled from GAML.

---

## **25.6 Second Brain Agent Role**

The **Second Brain Agent** provides the user-facing workflow layer for local memory.

It handles requests such as:

* “Prep me for my next meeting.”
* “What changed on this project?”
* “Who owes me what?”
* “What did we decide last week?”
* “What deadlines moved?”
* “Summarize my day.”
* “Draft the follow-up using what we already know.”

The Second Brain Agent is responsible for:

* reading the user request
* asking orchestration for allowed memory scope and execution mode
* requesting structured retrieval from GAML
* building a compact context packet
* invoking local or private ELM reasoning
* calling permitted tools
* returning a grounded answer, draft, action, or brief
* writing confirmed updates back into GAML
* emitting adaptation signals to EGGROLL when appropriate

---

## **25.7 EGGROLL Role**

In this mode, **EGGROLL** supports private and local adaptation in addition to swarm retraining.

EGGROLL can learn from:

* user corrections
* repeated phrasing preferences
* successful meeting prep patterns
* failed retrievals
* missed commitments
* stale or wrong deadlines
* tool-use outcomes
* accepted drafts
* rejected drafts
* verified task completions
* local evaluation results

EGGROLL outputs may include:

* adapter updates
* retrieval policy updates
* router weight updates
* prompt policy updates
* memory scoring policy updates
* local ELM fine-tuning jobs
* private enterprise model adaptation jobs

Private EGGROLL signals remain scoped to the user or enterprise unless broader sharing is explicitly enabled.

---

## **25.8 High-Level Flow**

```text
Local Sources
  -> Observer and Ingestion Agents
  -> GAML Structured Memory
  -> Personal Memory / Second Brain Agent
  -> Local or Private ELM Reasoning
  -> User Answer, Action, Brief, or Draft
  -> GAML Writeback
  -> EGGROLL Adaptation Signals
  -> Improved Private ELMs, Adapters, Routing, and Memory Policies
```

---

## **25.9 Local Data Sources**

The local second brain may ingest private or local context from:

* email
* calendar
* meeting transcripts
* voice notes
* local notes
* Markdown vaults
* documents and PDFs
* browser research captures
* project management systems
* source code repositories
* CRM exports
* support tickets
* internal wikis
* filesystem folders
* user corrections and feedback

By default, second-brain memory belongs to the user, local device, enterprise account, or private subnet that generated it.

---

## **25.10 Structured Memory Objects**

Local Cognitive Second Brain Mode stores memory as typed GAML objects.

Core memory object classes include:

* **Person:** name, role, organization, contact handles, relationship to user, interaction history, preferences, commitments, and trust signals
* **Organization:** company, customer, partner, school, hospital, government agency, vendor, or internal group
* **Project:** objective, stakeholders, status, open questions, milestones, blockers, risks, and related artifacts
* **Decision:** what was decided, by whom, when, why, source references, and whether it has been superseded
* **Commitment:** who promised what, to whom, by when, current status, source evidence, and follow-up history
* **Deadline:** due date, owner, linked project, source event, confidence, and change history
* **Task:** action item, owner, priority, dependency, due date, and completion state
* **Fact:** stable claim with source, timestamp, confidence, and validation status
* **Claim:** unverified or contested assertion awaiting confirmation
* **Contradiction:** conflict between facts, claims, dates, owners, assumptions, or sources
* **Preference:** user, team, or organization preference learned from behavior or explicit instruction
* **Style Signal:** writing style, communication tone, formatting preference, or decision style
* **Memory Trace:** retrieval path, reasoning dependency, arbitration decision, correction, or writeback event

---

## **25.11 Memory Lifecycle**

Each memory item moves through a lifecycle before it becomes trusted operational state.

```text
Observe -> Extract -> Normalize -> Link -> Score -> Store -> Retrieve -> Reason -> Verify -> Write Back -> Adapt
```

### **25.11.1 Observe**

Observer agents monitor permitted local sources and identify new or changed information.

### **25.11.2 Extract**

Ingestion agents extract typed entities, facts, claims, commitments, decisions, deadlines, tasks, and preferences.

### **25.11.3 Normalize**

The system resolves names, aliases, duplicate entities, date formats, references, and source metadata.

### **25.11.4 Link**

New memory objects are linked to existing people, projects, organizations, topics, prior decisions, and related tasks.

### **25.11.5 Score**

GAML assigns or updates confidence using source reliability, recency, repetition, user confirmation, contradiction status, and trust weight.

### **25.11.6 Store**

Memory is stored in the local or private GAML store using version-aware records and immutable source references where possible.

### **25.11.7 Retrieve**

The Second Brain Agent retrieves the memory needed for the current task.

Retrieval is structured and reasoning-driven.

### **25.11.8 Reason**

The selected local or private ELM reasons over the assembled memory packet, current user request, available tools, and relevant constraints.

### **25.11.9 Verify**

For higher-risk outputs, the system checks source grounding, contradictions, stale assumptions, and possible missing context before responding.

### **25.11.10 Write Back**

The result may update GAML with new tasks, decisions, confirmations, corrections, changed deadlines, user preferences, or reasoning traces.

### **25.11.11 Adapt**

EGGROLL converts repeated corrections, successful outcomes, failed retrievals, preferred phrasing, and workflow patterns into adaptation signals.

---

## **25.12 Context Packet Assembly**

The Second Brain Agent assembles compact context packets for local or private ELM execution.

A context packet may include:

* current user request
* relevant active project state
* recent timeline
* key people and organizations
* open decisions
* open commitments
* deadlines
* contradictions or uncertainty
* user preferences
* source references
* permitted tools
* privacy and execution constraints

The goal is to give small local ELMs enough structured context to act intelligently with bounded token use and low latency.

---

## **25.13 Human-Readable Memory Mirror**

GAML is the structured memory substrate.

Users and enterprises also need inspectability.

Local Cognitive Second Brain Mode should support a human-readable mirror such as:

```text
memory/
  Today.md
  People/
  Organizations/
  Projects/
  Decisions/
  Commitments/
  Deadlines/
  Tasks/
  Contradictions/
  Preferences/
  Sources/
```

This mirror may be Markdown, Obsidian-compatible files, HTML, or another portable representation.

It is an inspectable view over GAML so users can see what the system stores, where it came from, and what changed.

---

## **25.14 Privacy Modes**

Local Cognitive Second Brain Mode supports explicit privacy boundaries.

### **25.14.1 Local-Only Mode**

All memory, inference, retrieval, writeback, and adaptation remain on the device.

### **25.14.2 Private Enterprise Mode**

Memory may be shared inside a controlled enterprise subnet according to permissions, roles, and organizational policy.

### **25.14.3 Hybrid Mode**

Private memory stays local or enterprise-contained, while non-private tasks may use public GNUS compute or public knowledge grounding.

### **25.14.4 Explicit Swarm Contribution Mode**

Approved, filtered, anonymized, or deliberately shared adaptation signals may contribute to broader swarm learning.

---

## **25.15 Example Workflows**

### **25.15.1 Meeting Prep**

```text
User: Prep me for my 2pm meeting with Sarah.

Second Brain Agent:
  -> Reads calendar event
  -> Resolves Sarah as a person entity
  -> Retrieves project links, recent emails, commitments, and open questions
  -> Builds context packet
  -> Invokes local ELM
  -> Produces meeting brief with sources and open action items
  -> Writes any user corrections back into GAML
```

### **25.15.2 Project Drift Detection**

```text
User: What changed on the Nexlogic rollout this week?

Second Brain Agent:
  -> Retrieves project timeline
  -> Compares new emails, notes, deadlines, and decisions
  -> Detects changed assumptions or contradictions
  -> Summarizes material changes
  -> Flags stale commitments
```

### **25.15.3 Personal Daily Brief**

```text
Scheduled agent:
  -> Reads Today context, calendar, inbox, tasks, and active projects
  -> Retrieves relevant people and commitments
  -> Produces daily brief
  -> Optionally generates voice summary locally
```

### **25.15.4 Private ELM Adaptation**

```text
Repeated user corrections:
  -> Captured as preference and correction memory
  -> Converted into EGGROLL adaptation signals
  -> Used to tune local adapter, routing, or memory retrieval policy
  -> Improves future briefs and drafts
```

---

## **25.16 Implementation Requirements**

A first implementation should include:

1. Local source connectors for files, notes, email, calendar, and meeting transcripts.
2. GAML object schemas for people, organizations, projects, decisions, commitments, deadlines, tasks, facts, claims, preferences, style signals, contradictions, and memory traces.
3. A Second Brain Agent with retrieval, context-packet assembly, local ELM invocation, permitted tool use, and writeback.
4. A human-readable memory mirror.
5. Privacy modes for local-only, private enterprise, hybrid, and explicit swarm contribution.
6. EGGROLL signal emission for corrections, retrieval failures, accepted outputs, rejected outputs, and repeated preferences.
7. Validation logic for stale memory, conflicting commitments, source-grounding checks, and permission boundaries.

---

## **25.17 Design Principle**

Local Cognitive Second Brain Mode should be private, inspectable, and operationally useful.

It stores structured state in GAML.

It reasons through local and private ELMs using compact context packets.

It uses permitted tools through the Second Brain Agent.

It improves through EGGROLL adaptation signals.

It preserves user and enterprise control through explicit privacy boundaries and a human-readable memory mirror.

---

## **25.18 Summary**

Local Cognitive Second Brain Mode turns GCS into a private memory and reasoning system for individuals, teams, SMBs, and enterprises.

The architecture can be summarized as:

```text
Orchestration = control plane
GAML = memory substrate
Local ELM = reasoning engine
Second Brain Agent = behavior layer
EGGROLL = adaptation loop
Human-readable mirror = inspectability layer
```

Together, these components form a local-first second brain that can scale upward into private enterprise cognition and, when permitted, outward into the distributed GNUS cognitive network.

---
