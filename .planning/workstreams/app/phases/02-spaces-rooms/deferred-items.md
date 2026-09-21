# Phase 02 Deferred Items (out-of-scope discoveries)

## test_gcs_global_db_sdk — intermittent teardown segfault (discovered 2026-09-19, plan 02-01)

- **Observed:** `ctest -R test_gcs_global_db_sdk` fails ~1-in-3 runs with SEGFAULT during
  global test-environment tear-down — the gtest body itself reports `[ PASSED ] 1 test` before
  the crash. Direct binary runs passed 3/3 during triage.
- **Scope ruling:** PRE-EXISTING, unrelated to 02-01. The binary links
  `gcs_storage;neoswarm_common;sgns::crdt_globaldb` only (test/CMakeLists.txt:69) — it has no
  link path to gcs_core, gcs_entity_store, or gcs_proto (the only targets 02-01 touched).
  Looks like a static-destruction race in the GeniusSDK node path.
- **Action:** not fixed (scope boundary rule). Revisit if it starts failing deterministically;
  candidate angles: SDK node static teardown ordering, soralog logging-system teardown.
- **2026-09-19 A/B re-check (file-logging change):** the logging.hpp edit sits in this
  binary's include path, so the ruling was re-validated with a stash/rebuild A/B under an
  identical 8-run loop: pre-change 3/8 crashes, post-change 2/8 and 3/8 — indistinguishable
  from the documented ~1-in-3. Rate is load-sensitive (one rapid loop hit 8/8 under
  background load; a spaced single run passes). Ruling stands: ambient, unrelated to the
  GCS logging facade.
