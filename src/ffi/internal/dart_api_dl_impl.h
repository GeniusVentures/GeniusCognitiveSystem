/**
 * @file       dart_api_dl_impl.h
 * @brief      Vendored verbatim from the Dart SDK 3.11.5 headers (Flutter
 *             3.41.9, bin/cache/dart-sdk/include/internal/dart_api_dl_impl.h) on 2026-08-26 for the
 *             gcs_ffi Dart_PostCObject posting seam (GSD 01-05 Task 1).
 *             This provenance comment is the only modification.
 */
/*
 * Copyright (c) 2020, the Dart project authors.  Please see the AUTHORS file
 * for details. All rights reserved. Use of this source code is governed by a
 * BSD-style license that can be found in the LICENSE file.
 */

#ifndef RUNTIME_INCLUDE_INTERNAL_DART_API_DL_IMPL_H_
#define RUNTIME_INCLUDE_INTERNAL_DART_API_DL_IMPL_H_

typedef struct {
  const char* name;
  void (*function)(void);
} DartApiEntry;

typedef struct {
  const int major;
  const int minor;
  const DartApiEntry* const functions;
} DartApi;

#endif /* RUNTIME_INCLUDE_INTERNAL_DART_API_DL_IMPL_H_ */ /* NOLINT */
