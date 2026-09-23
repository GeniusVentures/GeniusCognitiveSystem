// This is a generated file - do not edit.
//
// Generated from gcs_chat.proto.

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
    {'1': 'mnemonic', '3': 3, '4': 1, '5': 9, '10': 'mnemonic'},
  ],
};

/// Descriptor for `GcsConfig`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List gcsConfigDescriptor = $convert.base64Decode(
    'CglHY3NDb25maWcSFwoHZGJfcGF0aBgBIAEoCVIGZGJQYXRoEiUKBWNvZGVjGAIgASgOMg8uZ2'
    'NzLmNoYXQuQ29kZWNSBWNvZGVjEhoKCG1uZW1vbmljGAMgASgJUghtbmVtb25pYw==');

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
    {
      '1': 'create_space',
      '3': 3,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.CreateSpaceCommand',
      '9': 0,
      '10': 'createSpace'
    },
    {
      '1': 'create_room',
      '3': 4,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.CreateRoomCommand',
      '9': 0,
      '10': 'createRoom'
    },
    {
      '1': 'update_space',
      '3': 5,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.UpdateSpaceCommand',
      '9': 0,
      '10': 'updateSpace'
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
    'dENvbW1hbmRIAFIIc2VuZFRleHQSQQoMY3JlYXRlX3NwYWNlGAMgASgLMhwuZ2NzLmNoYXQuQ3'
    'JlYXRlU3BhY2VDb21tYW5kSABSC2NyZWF0ZVNwYWNlEj4KC2NyZWF0ZV9yb29tGAQgASgLMhsu'
    'Z2NzLmNoYXQuQ3JlYXRlUm9vbUNvbW1hbmRIAFIKY3JlYXRlUm9vbRJBCgx1cGRhdGVfc3BhY2'
    'UYBSABKAsyHC5nY3MuY2hhdC5VcGRhdGVTcGFjZUNvbW1hbmRIAFILdXBkYXRlU3BhY2VCCQoH'
    'cGF5bG9hZA==');

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
    {'1': 'sender', '3': 7, '4': 1, '5': 9, '10': 'sender'},
    {'1': 'deleted', '3': 8, '4': 1, '5': 8, '10': 'deleted'},
    {'1': 'deleted_at_ms', '3': 9, '4': 1, '5': 3, '10': 'deletedAtMs'},
  ],
};

/// Descriptor for `ChatMessageState`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List chatMessageStateDescriptor = $convert.base64Decode(
    'ChBDaGF0TWVzc2FnZVN0YXRlEg4KAmlkGAEgASgJUgJpZBIdCgpyb29tX3RvcGljGAIgASgJUg'
    'lyb29tVG9waWMSKQoEcm9sZRgDIAEoDjIVLmdjcy5jaGF0Lk1lc3NhZ2VSb2xlUgRyb2xlEiwK'
    'BXN0YXRlGAQgASgOMhYuZ2NzLmNoYXQuTWVzc2FnZVN0YXRlUgVzdGF0ZRISCgR0ZXh0GAUgAS'
    'gJUgR0ZXh0EhwKCXRpbWVzdGFtcBgGIAEoA1IJdGltZXN0YW1wEhYKBnNlbmRlchgHIAEoCVIG'
    'c2VuZGVyEhgKB2RlbGV0ZWQYCCABKAhSB2RlbGV0ZWQSIgoNZGVsZXRlZF9hdF9tcxgJIAEoA1'
    'ILZGVsZXRlZEF0TXM=');

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
    {
      '1': 'space_tree',
      '3': 5,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.SpaceTree',
      '9': 0,
      '10': 'spaceTree'
    },
    {
      '1': 'message_history',
      '3': 6,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.MessageHistory',
      '9': 0,
      '10': 'messageHistory'
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
    '5lc3MSLQoFZXJyb3IYBCABKAsyFS5nY3MuY2hhdC5FcnJvck5vdGljZUgAUgVlcnJvchI0Cgpz'
    'cGFjZV90cmVlGAUgASgLMhMuZ2NzLmNoYXQuU3BhY2VUcmVlSABSCXNwYWNlVHJlZRJDCg9tZX'
    'NzYWdlX2hpc3RvcnkYBiABKAsyGC5nY3MuY2hhdC5NZXNzYWdlSGlzdG9yeUgAUg5tZXNzYWdl'
    'SGlzdG9yeUIJCgdwYXlsb2Fk');

@$core.Deprecated('Use spaceRecordDescriptor instead')
const SpaceRecord$json = {
  '1': 'SpaceRecord',
  '2': [
    {'1': 'id', '3': 1, '4': 1, '5': 9, '10': 'id'},
    {'1': 'name', '3': 2, '4': 1, '5': 9, '10': 'name'},
    {'1': 'is_public', '3': 3, '4': 1, '5': 8, '10': 'isPublic'},
    {'1': 'auto_join_rooms', '3': 4, '4': 1, '5': 8, '10': 'autoJoinRooms'},
    {'1': 'created_at_ms', '3': 5, '4': 1, '5': 3, '10': 'createdAtMs'},
    {'1': 'updated_at_ms', '3': 6, '4': 1, '5': 3, '10': 'updatedAtMs'},
    {'1': 'deleted', '3': 7, '4': 1, '5': 8, '10': 'deleted'},
    {'1': 'deleted_at_ms', '3': 8, '4': 1, '5': 3, '10': 'deletedAtMs'},
  ],
};

/// Descriptor for `SpaceRecord`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List spaceRecordDescriptor = $convert.base64Decode(
    'CgtTcGFjZVJlY29yZBIOCgJpZBgBIAEoCVICaWQSEgoEbmFtZRgCIAEoCVIEbmFtZRIbCglpc1'
    '9wdWJsaWMYAyABKAhSCGlzUHVibGljEiYKD2F1dG9fam9pbl9yb29tcxgEIAEoCFINYXV0b0pv'
    'aW5Sb29tcxIiCg1jcmVhdGVkX2F0X21zGAUgASgDUgtjcmVhdGVkQXRNcxIiCg11cGRhdGVkX2'
    'F0X21zGAYgASgDUgt1cGRhdGVkQXRNcxIYCgdkZWxldGVkGAcgASgIUgdkZWxldGVkEiIKDWRl'
    'bGV0ZWRfYXRfbXMYCCABKANSC2RlbGV0ZWRBdE1z');

@$core.Deprecated('Use roomRecordDescriptor instead')
const RoomRecord$json = {
  '1': 'RoomRecord',
  '2': [
    {'1': 'id', '3': 1, '4': 1, '5': 9, '10': 'id'},
    {'1': 'name', '3': 2, '4': 1, '5': 9, '10': 'name'},
    {'1': 'parent_space_id', '3': 3, '4': 1, '5': 9, '10': 'parentSpaceId'},
    {'1': 'created_at_ms', '3': 4, '4': 1, '5': 3, '10': 'createdAtMs'},
    {'1': 'updated_at_ms', '3': 5, '4': 1, '5': 3, '10': 'updatedAtMs'},
    {'1': 'deleted', '3': 6, '4': 1, '5': 8, '10': 'deleted'},
    {'1': 'deleted_at_ms', '3': 7, '4': 1, '5': 3, '10': 'deletedAtMs'},
  ],
};

/// Descriptor for `RoomRecord`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List roomRecordDescriptor = $convert.base64Decode(
    'CgpSb29tUmVjb3JkEg4KAmlkGAEgASgJUgJpZBISCgRuYW1lGAIgASgJUgRuYW1lEiYKD3Bhcm'
    'VudF9zcGFjZV9pZBgDIAEoCVINcGFyZW50U3BhY2VJZBIiCg1jcmVhdGVkX2F0X21zGAQgASgD'
    'UgtjcmVhdGVkQXRNcxIiCg11cGRhdGVkX2F0X21zGAUgASgDUgt1cGRhdGVkQXRNcxIYCgdkZW'
    'xldGVkGAYgASgIUgdkZWxldGVkEiIKDWRlbGV0ZWRfYXRfbXMYByABKANSC2RlbGV0ZWRBdE1z');

@$core.Deprecated('Use entityManifestDescriptor instead')
const EntityManifest$json = {
  '1': 'EntityManifest',
  '2': [
    {'1': 'space_id', '3': 1, '4': 3, '5': 9, '10': 'spaceId'},
    {'1': 'room_id', '3': 2, '4': 3, '5': 9, '10': 'roomId'},
  ],
};

/// Descriptor for `EntityManifest`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List entityManifestDescriptor = $convert.base64Decode(
    'Cg5FbnRpdHlNYW5pZmVzdBIZCghzcGFjZV9pZBgBIAMoCVIHc3BhY2VJZBIXCgdyb29tX2lkGA'
    'IgAygJUgZyb29tSWQ=');

@$core.Deprecated('Use createSpaceCommandDescriptor instead')
const CreateSpaceCommand$json = {
  '1': 'CreateSpaceCommand',
  '2': [
    {'1': 'name', '3': 1, '4': 1, '5': 9, '10': 'name'},
    {'1': 'is_public', '3': 2, '4': 1, '5': 8, '10': 'isPublic'},
    {'1': 'auto_join_rooms', '3': 3, '4': 1, '5': 8, '10': 'autoJoinRooms'},
  ],
};

/// Descriptor for `CreateSpaceCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List createSpaceCommandDescriptor = $convert.base64Decode(
    'ChJDcmVhdGVTcGFjZUNvbW1hbmQSEgoEbmFtZRgBIAEoCVIEbmFtZRIbCglpc19wdWJsaWMYAi'
    'ABKAhSCGlzUHVibGljEiYKD2F1dG9fam9pbl9yb29tcxgDIAEoCFINYXV0b0pvaW5Sb29tcw==');

@$core.Deprecated('Use createRoomCommandDescriptor instead')
const CreateRoomCommand$json = {
  '1': 'CreateRoomCommand',
  '2': [
    {'1': 'name', '3': 1, '4': 1, '5': 9, '10': 'name'},
    {'1': 'parent_space_id', '3': 2, '4': 1, '5': 9, '10': 'parentSpaceId'},
  ],
};

/// Descriptor for `CreateRoomCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List createRoomCommandDescriptor = $convert.base64Decode(
    'ChFDcmVhdGVSb29tQ29tbWFuZBISCgRuYW1lGAEgASgJUgRuYW1lEiYKD3BhcmVudF9zcGFjZV'
    '9pZBgCIAEoCVINcGFyZW50U3BhY2VJZA==');

@$core.Deprecated('Use updateSpaceCommandDescriptor instead')
const UpdateSpaceCommand$json = {
  '1': 'UpdateSpaceCommand',
  '2': [
    {'1': 'space_id', '3': 1, '4': 1, '5': 9, '10': 'spaceId'},
    {'1': 'name', '3': 2, '4': 1, '5': 9, '10': 'name'},
    {'1': 'is_public', '3': 3, '4': 1, '5': 8, '10': 'isPublic'},
    {'1': 'auto_join_rooms', '3': 4, '4': 1, '5': 8, '10': 'autoJoinRooms'},
  ],
};

/// Descriptor for `UpdateSpaceCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List updateSpaceCommandDescriptor = $convert.base64Decode(
    'ChJVcGRhdGVTcGFjZUNvbW1hbmQSGQoIc3BhY2VfaWQYASABKAlSB3NwYWNlSWQSEgoEbmFtZR'
    'gCIAEoCVIEbmFtZRIbCglpc19wdWJsaWMYAyABKAhSCGlzUHVibGljEiYKD2F1dG9fam9pbl9y'
    'b29tcxgEIAEoCFINYXV0b0pvaW5Sb29tcw==');

@$core.Deprecated('Use spaceTreeDescriptor instead')
const SpaceTree$json = {
  '1': 'SpaceTree',
  '2': [
    {
      '1': 'space',
      '3': 1,
      '4': 3,
      '5': 11,
      '6': '.gcs.chat.SpaceRecord',
      '10': 'space'
    },
    {
      '1': 'room',
      '3': 2,
      '4': 3,
      '5': 11,
      '6': '.gcs.chat.RoomRecord',
      '10': 'room'
    },
  ],
};

/// Descriptor for `SpaceTree`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List spaceTreeDescriptor = $convert.base64Decode(
    'CglTcGFjZVRyZWUSKwoFc3BhY2UYASADKAsyFS5nY3MuY2hhdC5TcGFjZVJlY29yZFIFc3BhY2'
    'USKAoEcm9vbRgCIAMoCzIULmdjcy5jaGF0LlJvb21SZWNvcmRSBHJvb20=');

@$core.Deprecated('Use messageHistoryDescriptor instead')
const MessageHistory$json = {
  '1': 'MessageHistory',
  '2': [
    {'1': 'room_topic', '3': 1, '4': 1, '5': 9, '10': 'roomTopic'},
    {
      '1': 'message',
      '3': 2,
      '4': 3,
      '5': 11,
      '6': '.gcs.chat.ChatMessageState',
      '10': 'message'
    },
  ],
};

/// Descriptor for `MessageHistory`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List messageHistoryDescriptor = $convert.base64Decode(
    'Cg5NZXNzYWdlSGlzdG9yeRIdCgpyb29tX3RvcGljGAEgASgJUglyb29tVG9waWMSNAoHbWVzc2'
    'FnZRgCIAMoCzIaLmdjcy5jaGF0LkNoYXRNZXNzYWdlU3RhdGVSB21lc3NhZ2U=');
