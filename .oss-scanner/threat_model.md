# Anthropic OSS Scanner threat model — Genius Cognitive System (GCS)

## Project boundary

GCS is a C++ cognitive system with local/peer storage, routed small-model execution, API and SDK/Flutter FFI surfaces, chat/entity state, and cryptographic messaging. The `GNUS-NEO-SWARM` pinned submodule adds inference/routing and distributed execution components. The code and unit tests checked out under `/src` are the target; the prebuilt thirdparty/zkLLVM/SuperGenius/GeniusSDK releases are required to link and exercise it.

## Untrusted inputs and security goals

- Treat remote and local API requests, chat/room traffic, document content, model output, peer messages, synchronization records, node discovery, and all FFI calls as untrusted.
- Prevent memory errors, out-of-bounds reads/writes, use-after-free, unsafe lifetime across Dart/C++ boundaries, and malformed serialization/parsing bugs.
- Keep tenant/user/room authorization checks consistent in storage, messaging, sync, and UI-to-native calls. Private conversations and local data must not leak across contexts.
- Authenticate/sign/encrypt messages correctly; reject replay, nonce reuse, forged sender IDs, and downgrade to cleartext where private mode is promised.
- Reject malformed CRDT/GlobalDB updates without corrupting state or letting peers overwrite entries they do not own.
- Treat LLM and tool output as *data* unless an explicit authorization gate grants permission. Prompt injection must not lead to tool misuse, privilege escalation, or leaks.
- Avoid remotely triggered unbounded CPU, memory, disk, network fanout, or model execution. Budget checks should work with nested/delegated jobs.

## Severity

**Critical:** unauthenticated remote code execution, cross-user secret extraction, arbitrary durable data corruption, signing/key compromise, or unauthorized distributed tool execution with material impact.

**High:** reachable memory corruption, serious privilege bypass, exposed private room state, network-authentication bypass, or exploitable replay/identity confusion.

**Medium:** bounded DoS, avoidable amplification, unsafe defaults requiring user cooperation, or limited data leak. **Low:** defense-in-depth with no reachable exploit.

Each report must state an actual attacker-controlled input, trust boundary, source location, a working local reproducer with bad and corrected outcomes, and a narrow fix. Separate real defects from missing future features.

## Reproduce offline

```bash
cd /src
ctest --test-dir build/Linux/Release/x86_64 -R '^test_gcs_crypto$' --output-on-failure --timeout 180
```

`test_gcs_crypto` exercises the stateless crypto adapter without remote RPC/LLM services. Other existing CTests can be run selectively; some network/node-booting suites need dbus/keyring or local networking and must not call outside services. Inspect `test/CMakeLists.txt` and `GNUS-NEO-SWARM/test`.

## Exclusions and handling

Do not scan live production endpoints, fetch credentials, ask to run cloud agents, make outside RPC calls, or publish exploit details. Avoid third-party-only library reports unless the vulnerable path is actually reachable in GCS. Deliver reports privately to `admin@gnus.ai`.
