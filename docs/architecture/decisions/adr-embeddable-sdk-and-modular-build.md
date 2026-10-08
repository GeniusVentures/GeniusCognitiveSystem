# ADR: Embeddable GCS SDK and Modular Native Build

- **Status:** accepted
- **Date:** 2026-07-28
- **Scope:** GeniusCognitiveSystem parent repository, native SDK, Flutter integration, CMake structure
- **Decision type:** architecture, SDK boundary, build system, application integration

## Context

GeniusCognitiveSystem currently serves primarily as the parent architecture and documentation repository, with GNUS-NEO-SWARM included as a submodule. The product will also provide a reusable chat interface that can be embedded into Flutter applications and other application shells.

The public application boundary must not depend directly on GNUS-NEO-SWARM internal classes. At the same time, the native code should follow the modular CMake conventions already used by GNUS-NEO-SWARM: each independently buildable module owns its source list, target, include paths, compile features, and direct link dependencies in its own `CMakeLists.txt`.

## Decision

### 1. GeniusCognitiveSystem becomes the stable SDK and embedding boundary

The parent repository will contain native source code and public interfaces in addition to architecture documentation.

The parent owns:

- public runtime request, event, status, result, and artifact contracts;
- the abstract `RuntimeCoordinator` interface;
- the public `GeniusCognitiveSystem` facade;
- chat session and streaming client contracts;
- a stable C ABI for Flutter and other foreign-function consumers;
- platform-neutral Dart client contracts;
- reusable Flutter chat UI components.

GNUS-NEO-SWARM remains the primary concrete runtime implementation behind those contracts.

### 2. Runtime implementation is injected behind the public interface

The parent defines a generic runtime lifecycle abstraction. The initial concrete implementation is supplied by GNUS-NEO-SWARM.

```text
GeniusCognitiveSystem facade
        |
RuntimeCoordinator interface
        ^
NeoSwarmRuntimeCoordinator implementation
```

The parent API must not expose NeoSwarm internal classes, MNN objects, SGProcessingManager objects, or SuperGenius client objects to applications.

### 3. Flutter consumes backend-neutral contracts

The Flutter chat package depends on a backend-neutral Dart interface capable of:

- submitting chat requests;
- receiving streamed runtime events;
- cancelling requests;
- querying request status;
- presenting tool approvals, artifacts, usage, provenance, and errors.

Backend implementations may include:

- native FFI into the GCS C ABI;
- OpenAI-compatible HTTP and server-sent events;
- gRPC where appropriate;
- mock backends for tests and previews.

The reusable Flutter UI must not depend directly on NeoSwarm, MNN, SGProcessingManager, or SuperGenius.

### 4. Native modules use per-directory CMake ownership

Every independently buildable or independently testable native module owns a `CMakeLists.txt` in its directory.

The root `CMakeLists.txt` configures the project and delegates with `add_subdirectory()`. It does not collect implementation source files from every module.

The initial structure is:

```text
GeniusCognitiveSystem/
|-- CMakeLists.txt
|-- cmake/
|   |-- CommonBuildParameters.cmake
|   |-- GCSOptions.cmake
|   |-- GCSInstall.cmake
|   `-- GCSPlatform.cmake
|-- include/gcs/
|   |-- runtime/
|   |-- chat/
|   |-- api/
|   `-- ffi/
|-- src/
|   |-- CMakeLists.txt
|   |-- common/
|   |   `-- CMakeLists.txt
|   |-- runtime/
|   |   `-- CMakeLists.txt
|   |-- chat/
|   |   `-- CMakeLists.txt
|   |-- api/
|   |   `-- CMakeLists.txt
|   `-- ffi/
|       `-- CMakeLists.txt
|-- test/
|   |-- CMakeLists.txt
|   |-- common/CMakeLists.txt
|   |-- runtime/CMakeLists.txt
|   |-- chat/CMakeLists.txt
|   |-- api/CMakeLists.txt
|   |-- ffi/CMakeLists.txt
|   `-- integration/CMakeLists.txt
|-- packages/
|   |-- gcs_client/
|   |-- gcs_chat/
|   `-- gcs_native/
`-- GNUS-NEO-SWARM/
```

Subdirectories may be split further when they become meaningful build or test modules. A directory containing only private headers does not require a target solely to satisfy a mechanical rule.

### 5. Initial native target boundaries

The expected initial targets are:

- `gcs_common` -- shared errors, identifiers, serialization primitives, and base types;
- `gcs_runtime` -- runtime contracts, handles, events, status, registry, and cancellation interfaces;
- `gcs_chat_core` -- chat sessions and event-stream adaptation independent of Flutter widgets;
- `gcs_api` -- the public `GeniusCognitiveSystem` facade;
- `gcs_ffi` -- the stable shared-library C ABI;
- `neoswarm_runtime` -- the concrete runtime coordinator supplied by GNUS-NEO-SWARM.

Each target declares only direct dependencies. Heavy dependencies remain in the owning implementation module.

### 6. Native dependencies do not leak through the SDK

MNN, Vulkan/MoltenVK, SGProcessingManager, SuperGenius, libp2p, RocksDB, and other heavy runtime dependencies belong to the NeoSwarm implementation and its subordinate modules.

The public GCS headers must remain usable without including those dependencies. Implementation details should use PImpl or abstract interfaces where needed to preserve ABI and reduce compile coupling.

### 7. Flutter uses a stable C ABI

Dart FFI binds to exported C functions rather than the C++ ABI. The C ABI manages opaque handles and structured request/event payloads.

The initial boundary should support:

- create and destroy;
- configure and initialize;
- submit chat/runtime request;
- receive streamed callbacks or poll events;
- cancel;
- query status;
- release returned memory safely.

The wire representation may initially use canonical JSON, provided it is versioned and validated. A schema-based binary representation may be added later without exposing C++ types.

## Rejected alternatives

### Flutter linking directly to GNUS-NEO-SWARM internals

Rejected because it couples the UI to one runtime implementation, makes remote and mock backends difficult, and leaks heavy native dependencies into every application.

### One monolithic native target for the entire parent repository

Rejected because it obscures ownership, forces unnecessary dependencies onto consumers, slows builds, and differs from the modular conventions used by GNUS-NEO-SWARM.

### A root CMake file that manually lists all module sources

Rejected because module build ownership would be centralized and fragile. Adding or removing files would require unrelated root-level edits.

### Exposing C++ classes directly through Dart FFI

Rejected because C++ ABI stability, name mangling, exceptions, object lifetime, and platform compiler differences make it unsuitable as the public Flutter boundary.

## Consequences

- The parent repository will gain `include/`, `src/`, `test/`, `cmake/`, and `packages/` trees.
- Each native module and mirrored test module will gain its own `CMakeLists.txt`.
- GNUS-NEO-SWARM must expose a concrete runtime target that implements the parent `RuntimeCoordinator` contract.
- Existing NeoSwarm Flutter bridges and UI code should be evaluated for migration into `gcs_native`, `gcs_client`, and `gcs_chat` rather than duplicated.
- Applications can embed the same chat UI while selecting native, HTTP, gRPC, or mock backends.
- The public SDK can evolve independently of the internal NeoSwarm implementation as long as the runtime and C ABI contracts remain compatible.
