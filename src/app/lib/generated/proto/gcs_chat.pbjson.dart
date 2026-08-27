// This is a generated file - do not edit.
//
// Generated from proto/gcs_chat.proto.

// @dart = 3.3

// ignore_for_file: annotate_overrides, camel_case_types, comment_references
// ignore_for_file: constant_identifier_names
// ignore_for_file: curly_braces_in_flow_control_structures
// ignore_for_file: deprecated_member_use_from_same_package, library_prefixes
// ignore_for_file: non_constant_identifier_names, unused_import

import 'dart:convert' as $convert;
import 'dart:core' as $core;
import 'dart:typed_data' as $typed_data;

@$core.Deprecated('Use codecDescriptor instead')
const Codec$json = {
  '1': 'Codec',
  '2': [
    {'1': 'CODEC_UNSPECIFIED', '2': 0},
    {'1': 'CODEC_PROTOBUF', '2': 1},
    {'1': 'CODEC_JSON', '2': 2},
  ],
};

/// Descriptor for `Codec`. Decode as a `google.protobuf.EnumDescriptorProto`.
final $typed_data.Uint8List codecDescriptor = $convert.base64Decode(
    'CgVDb2RlYxIVChFDT0RFQ19VTlNQRUNJRklFRBAAEhIKDkNPREVDX1BST1RPQlVGEAESDgoKQ0'
    '9ERUNfSlNPThAC');

@$core.Deprecated('Use messageRoleDescriptor instead')
const MessageRole$json = {
  '1': 'MessageRole',
  '2': [
    {'1': 'MESSAGE_ROLE_UNSPECIFIED', '2': 0},
    {'1': 'MESSAGE_ROLE_USER_SELF', '2': 1},
    {'1': 'MESSAGE_ROLE_USER_PEER', '2': 2},
    {'1': 'MESSAGE_ROLE_ASSISTANT', '2': 3},
    {'1': 'MESSAGE_ROLE_SYSTEM', '2': 4},
  ],
};

/// Descriptor for `MessageRole`. Decode as a `google.protobuf.EnumDescriptorProto`.
final $typed_data.Uint8List messageRoleDescriptor = $convert.base64Decode(
    'CgtNZXNzYWdlUm9sZRIcChhNRVNTQUdFX1JPTEVfVU5TUEVDSUZJRUQQABIaChZNRVNTQUdFX1'
    'JPTEVfVVNFUl9TRUxGEAESGgoWTUVTU0FHRV9ST0xFX1VTRVJfUEVFUhACEhoKFk1FU1NBR0Vf'
    'Uk9MRV9BU1NJU1RBTlQQAxIXChNNRVNTQUdFX1JPTEVfU1lTVEVNEAQ=');

@$core.Deprecated('Use messageStateDescriptor instead')
const MessageState$json = {
  '1': 'MessageState',
  '2': [
    {'1': 'MESSAGE_STATE_UNSPECIFIED', '2': 0},
    {'1': 'MESSAGE_STATE_PENDING', '2': 1},
    {'1': 'MESSAGE_STATE_STREAMING', '2': 2},
    {'1': 'MESSAGE_STATE_THINKING', '2': 3},
    {'1': 'MESSAGE_STATE_COMPLETE', '2': 4},
    {'1': 'MESSAGE_STATE_ERROR', '2': 5},
  ],
};

/// Descriptor for `MessageState`. Decode as a `google.protobuf.EnumDescriptorProto`.
final $typed_data.Uint8List messageStateDescriptor = $convert.base64Decode(
    'CgxNZXNzYWdlU3RhdGUSHQoZTUVTU0FHRV9TVEFURV9VTlNQRUNJRklFRBAAEhkKFU1FU1NBR0'
    'VfU1RBVEVfUEVORElORxABEhsKF01FU1NBR0VfU1RBVEVfU1RSRUFNSU5HEAISGgoWTUVTU0FH'
    'RV9TVEFURV9USElOS0lORxADEhoKFk1FU1NBR0VfU1RBVEVfQ09NUExFVEUQBBIXChNNRVNTQU'
    'dFX1NUQVRFX0VSUk9SEAU=');

@$core.Deprecated('Use gcsConfigDescriptor instead')
const GcsConfig$json = {
  '1': 'GcsConfig',
  '2': [
    {'1': 'db_path', '3': 1, '4': 1, '5': 9, '10': 'dbPath'},
    {
      '1': 'codec',
      '3': 2,
      '4': 1,
      '5': 14,
      '6': '.gcs.chat.Codec',
      '10': 'codec'
    },
  ],
};

/// Descriptor for `GcsConfig`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List gcsConfigDescriptor = $convert.base64Decode(
    'CglHY3NDb25maWcSFwoHZGJfcGF0aBgBIAEoCVIGZGJQYXRoEiUKBWNvZGVjGAIgASgOMg8uZ2'
    'NzLmNoYXQuQ29kZWNSBWNvZGVj');

@$core.Deprecated('Use sendTextCommandDescriptor instead')
const SendTextCommand$json = {
  '1': 'SendTextCommand',
  '2': [
    {'1': 'room_topic', '3': 1, '4': 1, '5': 9, '10': 'roomTopic'},
    {'1': 'text', '3': 2, '4': 1, '5': 9, '10': 'text'},
  ],
};

/// Descriptor for `SendTextCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List sendTextCommandDescriptor = $convert.base64Decode(
    'Cg9TZW5kVGV4dENvbW1hbmQSHQoKcm9vbV90b3BpYxgBIAEoCVIJcm9vbVRvcGljEhIKBHRleH'
    'QYAiABKAlSBHRleHQ=');

@$core.Deprecated('Use joinTopicCommandDescriptor instead')
const JoinTopicCommand$json = {
  '1': 'JoinTopicCommand',
  '2': [
    {'1': 'room_topic', '3': 1, '4': 1, '5': 9, '10': 'roomTopic'},
  ],
};

/// Descriptor for `JoinTopicCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List joinTopicCommandDescriptor = $convert.base64Decode(
    'ChBKb2luVG9waWNDb21tYW5kEh0KCnJvb21fdG9waWMYASABKAlSCXJvb21Ub3BpYw==');

@$core.Deprecated('Use gcsCommandDescriptor instead')
const GcsCommand$json = {
  '1': 'GcsCommand',
  '2': [
    {
      '1': 'join_topic',
      '3': 1,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.JoinTopicCommand',
      '9': 0,
      '10': 'joinTopic'
    },
    {
      '1': 'send_text',
      '3': 2,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.SendTextCommand',
      '9': 0,
      '10': 'sendText'
    },
  ],
  '8': [
    {'1': 'payload'},
  ],
};

/// Descriptor for `GcsCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List gcsCommandDescriptor = $convert.base64Decode(
    'CgpHY3NDb21tYW5kEjsKCmpvaW5fdG9waWMYASABKAsyGi5nY3MuY2hhdC5Kb2luVG9waWNDb2'
    '1tYW5kSABSCWpvaW5Ub3BpYxI4CglzZW5kX3RleHQYAiABKAsyGS5nY3MuY2hhdC5TZW5kVGV4'
    'dENvbW1hbmRIAFIIc2VuZFRleHRCCQoHcGF5bG9hZA==');

@$core.Deprecated('Use chatMessageStateDescriptor instead')
const ChatMessageState$json = {
  '1': 'ChatMessageState',
  '2': [
    {'1': 'id', '3': 1, '4': 1, '5': 9, '10': 'id'},
    {'1': 'room_topic', '3': 2, '4': 1, '5': 9, '10': 'roomTopic'},
    {
      '1': 'role',
      '3': 3,
      '4': 1,
      '5': 14,
      '6': '.gcs.chat.MessageRole',
      '10': 'role'
    },
    {
      '1': 'state',
      '3': 4,
      '4': 1,
      '5': 14,
      '6': '.gcs.chat.MessageState',
      '10': 'state'
    },
    {'1': 'text', '3': 5, '4': 1, '5': 9, '10': 'text'},
    {'1': 'timestamp', '3': 6, '4': 1, '5': 3, '10': 'timestamp'},
  ],
};

/// Descriptor for `ChatMessageState`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List chatMessageStateDescriptor = $convert.base64Decode(
    'ChBDaGF0TWVzc2FnZVN0YXRlEg4KAmlkGAEgASgJUgJpZBIdCgpyb29tX3RvcGljGAIgASgJUg'
    'lyb29tVG9waWMSKQoEcm9sZRgDIAEoDjIVLmdjcy5jaGF0Lk1lc3NhZ2VSb2xlUgRyb2xlEiwK'
    'BXN0YXRlGAQgASgOMhYuZ2NzLmNoYXQuTWVzc2FnZVN0YXRlUgVzdGF0ZRISCgR0ZXh0GAUgAS'
    'gJUgR0ZXh0EhwKCXRpbWVzdGFtcBgGIAEoA1IJdGltZXN0YW1w');

@$core.Deprecated('Use roomListDescriptor instead')
const RoomList$json = {
  '1': 'RoomList',
  '2': [
    {'1': 'room_topic', '3': 1, '4': 3, '5': 9, '10': 'roomTopic'},
  ],
};

/// Descriptor for `RoomList`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List roomListDescriptor = $convert
    .base64Decode('CghSb29tTGlzdBIdCgpyb29tX3RvcGljGAEgAygJUglyb29tVG9waWM=');

@$core.Deprecated('Use readinessDescriptor instead')
const Readiness$json = {
  '1': 'Readiness',
  '2': [
    {'1': 'ready', '3': 1, '4': 1, '5': 8, '10': 'ready'},
  ],
};

/// Descriptor for `Readiness`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List readinessDescriptor =
    $convert.base64Decode('CglSZWFkaW5lc3MSFAoFcmVhZHkYASABKAhSBXJlYWR5');

@$core.Deprecated('Use errorNoticeDescriptor instead')
const ErrorNotice$json = {
  '1': 'ErrorNotice',
  '2': [
    {'1': 'message', '3': 1, '4': 1, '5': 9, '10': 'message'},
  ],
};

/// Descriptor for `ErrorNotice`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List errorNoticeDescriptor = $convert
    .base64Decode('CgtFcnJvck5vdGljZRIYCgdtZXNzYWdlGAEgASgJUgdtZXNzYWdl');

@$core.Deprecated('Use gcsEventDescriptor instead')
const GcsEvent$json = {
  '1': 'GcsEvent',
  '2': [
    {
      '1': 'message',
      '3': 1,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.ChatMessageState',
      '9': 0,
      '10': 'message'
    },
    {
      '1': 'room_list',
      '3': 2,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.RoomList',
      '9': 0,
      '10': 'roomList'
    },
    {
      '1': 'readiness',
      '3': 3,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.Readiness',
      '9': 0,
      '10': 'readiness'
    },
    {
      '1': 'error',
      '3': 4,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.ErrorNotice',
      '9': 0,
      '10': 'error'
    },
  ],
  '8': [
    {'1': 'payload'},
  ],
};

/// Descriptor for `GcsEvent`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List gcsEventDescriptor = $convert.base64Decode(
    'CghHY3NFdmVudBI2CgdtZXNzYWdlGAEgASgLMhouZ2NzLmNoYXQuQ2hhdE1lc3NhZ2VTdGF0ZU'
    'gAUgdtZXNzYWdlEjEKCXJvb21fbGlzdBgCIAEoCzISLmdjcy5jaGF0LlJvb21MaXN0SABSCHJv'
    'b21MaXN0EjMKCXJlYWRpbmVzcxgDIAEoCzITLmdjcy5jaGF0LlJlYWRpbmVzc0gAUglyZWFkaW'
    '5lc3MSLQoFZXJyb3IYBCABKAsyFS5nY3MuY2hhdC5FcnJvck5vdGljZUgAUgVlcnJvckIJCgdw'
    'YXlsb2Fk');
