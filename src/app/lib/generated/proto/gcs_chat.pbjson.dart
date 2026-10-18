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

@$core.Deprecated('Use roleDescriptor instead')
const Role$json = {
  '1': 'Role',
  '2': [
    {'1': 'ROLE_UNSPECIFIED', '2': 0},
    {'1': 'ROLE_SUPER_ADMIN', '2': 1},
    {'1': 'ROLE_ADMIN', '2': 2},
    {'1': 'ROLE_MODERATOR', '2': 3},
    {'1': 'ROLE_MEMBER', '2': 4},
    {'1': 'ROLE_GUEST', '2': 5},
  ],
};

/// Descriptor for `Role`. Decode as a `google.protobuf.EnumDescriptorProto`.
final $typed_data.Uint8List roleDescriptor = $convert.base64Decode(
    'CgRSb2xlEhQKEFJPTEVfVU5TUEVDSUZJRUQQABIUChBST0xFX1NVUEVSX0FETUlOEAESDgoKUk'
    '9MRV9BRE1JThACEhIKDlJPTEVfTU9ERVJBVE9SEAMSDwoLUk9MRV9NRU1CRVIQBBIOCgpST0xF'
    'X0dVRVNUEAU=');

@$core.Deprecated('Use pendingActionDescriptor instead')
const PendingAction$json = {
  '1': 'PendingAction',
  '2': [
    {'1': 'PENDING_ACTION_UNSPECIFIED', '2': 0},
    {'1': 'PENDING_ACTION_REMOVE', '2': 1},
    {'1': 'PENDING_ACTION_ROLE_CHANGE', '2': 2},
    {'1': 'PENDING_ACTION_DELETE', '2': 3},
  ],
};

/// Descriptor for `PendingAction`. Decode as a `google.protobuf.EnumDescriptorProto`.
final $typed_data.Uint8List pendingActionDescriptor = $convert.base64Decode(
    'Cg1QZW5kaW5nQWN0aW9uEh4KGlBFTkRJTkdfQUNUSU9OX1VOU1BFQ0lGSUVEEAASGQoVUEVORE'
    'lOR19BQ1RJT05fUkVNT1ZFEAESHgoaUEVORElOR19BQ1RJT05fUk9MRV9DSEFOR0UQAhIZChVQ'
    'RU5ESU5HX0FDVElPTl9ERUxFVEUQAw==');

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
    {
      '1': 'invite',
      '3': 6,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.InviteCommand',
      '9': 0,
      '10': 'invite'
    },
    {
      '1': 'redeem',
      '3': 7,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.RedeemCommand',
      '9': 0,
      '10': 'redeem'
    },
    {
      '1': 'change_role',
      '3': 8,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.ChangeRoleCommand',
      '9': 0,
      '10': 'changeRole'
    },
    {
      '1': 'remove_member',
      '3': 9,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.RemoveMemberCommand',
      '9': 0,
      '10': 'removeMember'
    },
    {
      '1': 'leave',
      '3': 10,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.LeaveCommand',
      '9': 0,
      '10': 'leave'
    },
    {
      '1': 'delete_scope',
      '3': 11,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.DeleteScopeCommand',
      '9': 0,
      '10': 'deleteScope'
    },
    {
      '1': 'approve',
      '3': 12,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.ApproveCommand',
      '9': 0,
      '10': 'approve'
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
    'UYBSABKAsyHC5nY3MuY2hhdC5VcGRhdGVTcGFjZUNvbW1hbmRIAFILdXBkYXRlU3BhY2USMQoG'
    'aW52aXRlGAYgASgLMhcuZ2NzLmNoYXQuSW52aXRlQ29tbWFuZEgAUgZpbnZpdGUSMQoGcmVkZW'
    'VtGAcgASgLMhcuZ2NzLmNoYXQuUmVkZWVtQ29tbWFuZEgAUgZyZWRlZW0SPgoLY2hhbmdlX3Jv'
    'bGUYCCABKAsyGy5nY3MuY2hhdC5DaGFuZ2VSb2xlQ29tbWFuZEgAUgpjaGFuZ2VSb2xlEkQKDX'
    'JlbW92ZV9tZW1iZXIYCSABKAsyHS5nY3MuY2hhdC5SZW1vdmVNZW1iZXJDb21tYW5kSABSDHJl'
    'bW92ZU1lbWJlchIuCgVsZWF2ZRgKIAEoCzIWLmdjcy5jaGF0LkxlYXZlQ29tbWFuZEgAUgVsZW'
    'F2ZRJBCgxkZWxldGVfc2NvcGUYCyABKAsyHC5nY3MuY2hhdC5EZWxldGVTY29wZUNvbW1hbmRI'
    'AFILZGVsZXRlU2NvcGUSNAoHYXBwcm92ZRgMIAEoCzIYLmdjcy5jaGF0LkFwcHJvdmVDb21tYW'
    '5kSABSB2FwcHJvdmVCCQoHcGF5bG9hZA==');

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
    {
      '1': 'member_list',
      '3': 7,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.MemberList',
      '9': 0,
      '10': 'memberList'
    },
    {
      '1': 'invite_link',
      '3': 8,
      '4': 1,
      '5': 11,
      '6': '.gcs.chat.InviteLink',
      '9': 0,
      '10': 'inviteLink'
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
    'SGlzdG9yeRI3CgttZW1iZXJfbGlzdBgHIAEoCzIULmdjcy5jaGF0Lk1lbWJlckxpc3RIAFIKbW'
    'VtYmVyTGlzdBI3CgtpbnZpdGVfbGluaxgIIAEoCzIULmdjcy5jaGF0Lkludml0ZUxpbmtIAFIK'
    'aW52aXRlTGlua0IJCgdwYXlsb2Fk');

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
    {'1': 'creator', '3': 9, '4': 1, '5': 9, '10': 'creator'},
    {
      '1': 'members_can_invite',
      '3': 10,
      '4': 1,
      '5': 8,
      '10': 'membersCanInvite'
    },
    {'1': 'approved_by', '3': 11, '4': 1, '5': 9, '10': 'approvedBy'},
    {
      '1': 'pending_action',
      '3': 12,
      '4': 1,
      '5': 14,
      '6': '.gcs.chat.PendingAction',
      '10': 'pendingAction'
    },
  ],
};

/// Descriptor for `SpaceRecord`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List spaceRecordDescriptor = $convert.base64Decode(
    'CgtTcGFjZVJlY29yZBIOCgJpZBgBIAEoCVICaWQSEgoEbmFtZRgCIAEoCVIEbmFtZRIbCglpc1'
    '9wdWJsaWMYAyABKAhSCGlzUHVibGljEiYKD2F1dG9fam9pbl9yb29tcxgEIAEoCFINYXV0b0pv'
    'aW5Sb29tcxIiCg1jcmVhdGVkX2F0X21zGAUgASgDUgtjcmVhdGVkQXRNcxIiCg11cGRhdGVkX2'
    'F0X21zGAYgASgDUgt1cGRhdGVkQXRNcxIYCgdkZWxldGVkGAcgASgIUgdkZWxldGVkEiIKDWRl'
    'bGV0ZWRfYXRfbXMYCCABKANSC2RlbGV0ZWRBdE1zEhgKB2NyZWF0b3IYCSABKAlSB2NyZWF0b3'
    'ISLAoSbWVtYmVyc19jYW5faW52aXRlGAogASgIUhBtZW1iZXJzQ2FuSW52aXRlEh8KC2FwcHJv'
    'dmVkX2J5GAsgASgJUgphcHByb3ZlZEJ5Ej4KDnBlbmRpbmdfYWN0aW9uGAwgASgOMhcuZ2NzLm'
    'NoYXQuUGVuZGluZ0FjdGlvblINcGVuZGluZ0FjdGlvbg==');

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
    {'1': 'creator', '3': 8, '4': 1, '5': 9, '10': 'creator'},
    {
      '1': 'members_can_invite',
      '3': 9,
      '4': 1,
      '5': 8,
      '10': 'membersCanInvite'
    },
    {'1': 'approved_by', '3': 10, '4': 1, '5': 9, '10': 'approvedBy'},
    {
      '1': 'pending_action',
      '3': 11,
      '4': 1,
      '5': 14,
      '6': '.gcs.chat.PendingAction',
      '10': 'pendingAction'
    },
  ],
};

/// Descriptor for `RoomRecord`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List roomRecordDescriptor = $convert.base64Decode(
    'CgpSb29tUmVjb3JkEg4KAmlkGAEgASgJUgJpZBISCgRuYW1lGAIgASgJUgRuYW1lEiYKD3Bhcm'
    'VudF9zcGFjZV9pZBgDIAEoCVINcGFyZW50U3BhY2VJZBIiCg1jcmVhdGVkX2F0X21zGAQgASgD'
    'UgtjcmVhdGVkQXRNcxIiCg11cGRhdGVkX2F0X21zGAUgASgDUgt1cGRhdGVkQXRNcxIYCgdkZW'
    'xldGVkGAYgASgIUgdkZWxldGVkEiIKDWRlbGV0ZWRfYXRfbXMYByABKANSC2RlbGV0ZWRBdE1z'
    'EhgKB2NyZWF0b3IYCCABKAlSB2NyZWF0b3ISLAoSbWVtYmVyc19jYW5faW52aXRlGAkgASgIUh'
    'BtZW1iZXJzQ2FuSW52aXRlEh8KC2FwcHJvdmVkX2J5GAogASgJUgphcHByb3ZlZEJ5Ej4KDnBl'
    'bmRpbmdfYWN0aW9uGAsgASgOMhcuZ2NzLmNoYXQuUGVuZGluZ0FjdGlvblINcGVuZGluZ0FjdG'
    'lvbg==');

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
    {
      '1': 'members_can_invite',
      '3': 4,
      '4': 1,
      '5': 8,
      '10': 'membersCanInvite'
    },
  ],
};

/// Descriptor for `CreateSpaceCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List createSpaceCommandDescriptor = $convert.base64Decode(
    'ChJDcmVhdGVTcGFjZUNvbW1hbmQSEgoEbmFtZRgBIAEoCVIEbmFtZRIbCglpc19wdWJsaWMYAi'
    'ABKAhSCGlzUHVibGljEiYKD2F1dG9fam9pbl9yb29tcxgDIAEoCFINYXV0b0pvaW5Sb29tcxIs'
    'ChJtZW1iZXJzX2Nhbl9pbnZpdGUYBCABKAhSEG1lbWJlcnNDYW5JbnZpdGU=');

@$core.Deprecated('Use createRoomCommandDescriptor instead')
const CreateRoomCommand$json = {
  '1': 'CreateRoomCommand',
  '2': [
    {'1': 'name', '3': 1, '4': 1, '5': 9, '10': 'name'},
    {'1': 'parent_space_id', '3': 2, '4': 1, '5': 9, '10': 'parentSpaceId'},
    {
      '1': 'members_can_invite',
      '3': 3,
      '4': 1,
      '5': 8,
      '10': 'membersCanInvite'
    },
  ],
};

/// Descriptor for `CreateRoomCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List createRoomCommandDescriptor = $convert.base64Decode(
    'ChFDcmVhdGVSb29tQ29tbWFuZBISCgRuYW1lGAEgASgJUgRuYW1lEiYKD3BhcmVudF9zcGFjZV'
    '9pZBgCIAEoCVINcGFyZW50U3BhY2VJZBIsChJtZW1iZXJzX2Nhbl9pbnZpdGUYAyABKAhSEG1l'
    'bWJlcnNDYW5JbnZpdGU=');

@$core.Deprecated('Use updateSpaceCommandDescriptor instead')
const UpdateSpaceCommand$json = {
  '1': 'UpdateSpaceCommand',
  '2': [
    {'1': 'space_id', '3': 1, '4': 1, '5': 9, '10': 'spaceId'},
    {'1': 'name', '3': 2, '4': 1, '5': 9, '10': 'name'},
    {'1': 'is_public', '3': 3, '4': 1, '5': 8, '10': 'isPublic'},
    {'1': 'auto_join_rooms', '3': 4, '4': 1, '5': 8, '10': 'autoJoinRooms'},
    {
      '1': 'members_can_invite',
      '3': 5,
      '4': 1,
      '5': 8,
      '10': 'membersCanInvite'
    },
  ],
};

/// Descriptor for `UpdateSpaceCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List updateSpaceCommandDescriptor = $convert.base64Decode(
    'ChJVcGRhdGVTcGFjZUNvbW1hbmQSGQoIc3BhY2VfaWQYASABKAlSB3NwYWNlSWQSEgoEbmFtZR'
    'gCIAEoCVIEbmFtZRIbCglpc19wdWJsaWMYAyABKAhSCGlzUHVibGljEiYKD2F1dG9fam9pbl9y'
    'b29tcxgEIAEoCFINYXV0b0pvaW5Sb29tcxIsChJtZW1iZXJzX2Nhbl9pbnZpdGUYBSABKAhSEG'
    '1lbWJlcnNDYW5JbnZpdGU=');

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

@$core.Deprecated('Use wrappedRoomKeyDescriptor instead')
const WrappedRoomKey$json = {
  '1': 'WrappedRoomKey',
  '2': [
    {'1': 'room_id', '3': 1, '4': 1, '5': 9, '10': 'roomId'},
    {'1': 'envelope', '3': 2, '4': 1, '5': 12, '10': 'envelope'},
  ],
};

/// Descriptor for `WrappedRoomKey`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List wrappedRoomKeyDescriptor = $convert.base64Decode(
    'Cg5XcmFwcGVkUm9vbUtleRIXCgdyb29tX2lkGAEgASgJUgZyb29tSWQSGgoIZW52ZWxvcGUYAi'
    'ABKAxSCGVudmVsb3Bl');

@$core.Deprecated('Use memberRecordDescriptor instead')
const MemberRecord$json = {
  '1': 'MemberRecord',
  '2': [
    {'1': 'scope_id', '3': 1, '4': 1, '5': 9, '10': 'scopeId'},
    {'1': 'address', '3': 2, '4': 1, '5': 9, '10': 'address'},
    {'1': 'role', '3': 3, '4': 1, '5': 14, '6': '.gcs.chat.Role', '10': 'role'},
    {'1': 'explicit_leave', '3': 4, '4': 1, '5': 8, '10': 'explicitLeave'},
    {'1': 'deleted', '3': 5, '4': 1, '5': 8, '10': 'deleted'},
    {'1': 'created_at_ms', '3': 6, '4': 1, '5': 3, '10': 'createdAtMs'},
    {'1': 'updated_at_ms', '3': 7, '4': 1, '5': 3, '10': 'updatedAtMs'},
    {'1': 'approved_by', '3': 8, '4': 1, '5': 9, '10': 'approvedBy'},
    {
      '1': 'pending_action',
      '3': 9,
      '4': 1,
      '5': 14,
      '6': '.gcs.chat.PendingAction',
      '10': 'pendingAction'
    },
    {
      '1': 'pending_role',
      '3': 10,
      '4': 1,
      '5': 14,
      '6': '.gcs.chat.Role',
      '10': 'pendingRole'
    },
    {
      '1': 'wrapped_room_key',
      '3': 11,
      '4': 3,
      '5': 11,
      '6': '.gcs.chat.WrappedRoomKey',
      '10': 'wrappedRoomKey'
    },
  ],
};

/// Descriptor for `MemberRecord`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List memberRecordDescriptor = $convert.base64Decode(
    'CgxNZW1iZXJSZWNvcmQSGQoIc2NvcGVfaWQYASABKAlSB3Njb3BlSWQSGAoHYWRkcmVzcxgCIA'
    'EoCVIHYWRkcmVzcxIiCgRyb2xlGAMgASgOMg4uZ2NzLmNoYXQuUm9sZVIEcm9sZRIlCg5leHBs'
    'aWNpdF9sZWF2ZRgEIAEoCFINZXhwbGljaXRMZWF2ZRIYCgdkZWxldGVkGAUgASgIUgdkZWxldG'
    'VkEiIKDWNyZWF0ZWRfYXRfbXMYBiABKANSC2NyZWF0ZWRBdE1zEiIKDXVwZGF0ZWRfYXRfbXMY'
    'ByABKANSC3VwZGF0ZWRBdE1zEh8KC2FwcHJvdmVkX2J5GAggASgJUgphcHByb3ZlZEJ5Ej4KDn'
    'BlbmRpbmdfYWN0aW9uGAkgASgOMhcuZ2NzLmNoYXQuUGVuZGluZ0FjdGlvblINcGVuZGluZ0Fj'
    'dGlvbhIxCgxwZW5kaW5nX3JvbGUYCiABKA4yDi5nY3MuY2hhdC5Sb2xlUgtwZW5kaW5nUm9sZR'
    'JCChB3cmFwcGVkX3Jvb21fa2V5GAsgAygLMhguZ2NzLmNoYXQuV3JhcHBlZFJvb21LZXlSDndy'
    'YXBwZWRSb29tS2V5');

@$core.Deprecated('Use memberListDescriptor instead')
const MemberList$json = {
  '1': 'MemberList',
  '2': [
    {'1': 'scope_id', '3': 1, '4': 1, '5': 9, '10': 'scopeId'},
    {
      '1': 'member',
      '3': 2,
      '4': 3,
      '5': 11,
      '6': '.gcs.chat.MemberRecord',
      '10': 'member'
    },
    {'1': 'self_address', '3': 3, '4': 1, '5': 9, '10': 'selfAddress'},
  ],
};

/// Descriptor for `MemberList`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List memberListDescriptor = $convert.base64Decode(
    'CgpNZW1iZXJMaXN0EhkKCHNjb3BlX2lkGAEgASgJUgdzY29wZUlkEi4KBm1lbWJlchgCIAMoCz'
    'IWLmdjcy5jaGF0Lk1lbWJlclJlY29yZFIGbWVtYmVyEiEKDHNlbGZfYWRkcmVzcxgDIAEoCVIL'
    'c2VsZkFkZHJlc3M=');

@$core.Deprecated('Use inviteLinkDescriptor instead')
const InviteLink$json = {
  '1': 'InviteLink',
  '2': [
    {'1': 'url', '3': 1, '4': 1, '5': 9, '10': 'url'},
    {'1': 'scope_id', '3': 2, '4': 1, '5': 9, '10': 'scopeId'},
  ],
};

/// Descriptor for `InviteLink`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List inviteLinkDescriptor = $convert.base64Decode(
    'CgpJbnZpdGVMaW5rEhAKA3VybBgBIAEoCVIDdXJsEhkKCHNjb3BlX2lkGAIgASgJUgdzY29wZU'
    'lk');

@$core.Deprecated('Use inviteRecordDescriptor instead')
const InviteRecord$json = {
  '1': 'InviteRecord',
  '2': [
    {'1': 'scope_id', '3': 1, '4': 1, '5': 9, '10': 'scopeId'},
    {'1': 'role', '3': 2, '4': 1, '5': 14, '6': '.gcs.chat.Role', '10': 'role'},
    {
      '1': 'wrapped_room_key',
      '3': 3,
      '4': 3,
      '5': 11,
      '6': '.gcs.chat.WrappedRoomKey',
      '10': 'wrappedRoomKey'
    },
    {'1': 'is_space', '3': 4, '4': 1, '5': 8, '10': 'isSpace'},
    {'1': 'authenticator', '3': 5, '4': 1, '5': 9, '10': 'authenticator'},
    {'1': 'created_by', '3': 6, '4': 1, '5': 9, '10': 'createdBy'},
    {'1': 'issuer_signature', '3': 7, '4': 1, '5': 9, '10': 'issuerSignature'},
  ],
};

/// Descriptor for `InviteRecord`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List inviteRecordDescriptor = $convert.base64Decode(
    'CgxJbnZpdGVSZWNvcmQSGQoIc2NvcGVfaWQYASABKAlSB3Njb3BlSWQSIgoEcm9sZRgCIAEoDj'
    'IOLmdjcy5jaGF0LlJvbGVSBHJvbGUSQgoQd3JhcHBlZF9yb29tX2tleRgDIAMoCzIYLmdjcy5j'
    'aGF0LldyYXBwZWRSb29tS2V5Ug53cmFwcGVkUm9vbUtleRIZCghpc19zcGFjZRgEIAEoCFIHaX'
    'NTcGFjZRIkCg1hdXRoZW50aWNhdG9yGAUgASgJUg1hdXRoZW50aWNhdG9yEh0KCmNyZWF0ZWRf'
    'YnkYBiABKAlSCWNyZWF0ZWRCeRIpChBpc3N1ZXJfc2lnbmF0dXJlGAcgASgJUg9pc3N1ZXJTaW'
    'duYXR1cmU=');

@$core.Deprecated('Use inviteCommandDescriptor instead')
const InviteCommand$json = {
  '1': 'InviteCommand',
  '2': [
    {'1': 'scope_id', '3': 1, '4': 1, '5': 9, '10': 'scopeId'},
  ],
};

/// Descriptor for `InviteCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List inviteCommandDescriptor = $convert
    .base64Decode('Cg1JbnZpdGVDb21tYW5kEhkKCHNjb3BlX2lkGAEgASgJUgdzY29wZUlk');

@$core.Deprecated('Use redeemCommandDescriptor instead')
const RedeemCommand$json = {
  '1': 'RedeemCommand',
  '2': [
    {'1': 'invite_url', '3': 1, '4': 1, '5': 9, '10': 'inviteUrl'},
  ],
};

/// Descriptor for `RedeemCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List redeemCommandDescriptor = $convert.base64Decode(
    'Cg1SZWRlZW1Db21tYW5kEh0KCmludml0ZV91cmwYASABKAlSCWludml0ZVVybA==');

@$core.Deprecated('Use changeRoleCommandDescriptor instead')
const ChangeRoleCommand$json = {
  '1': 'ChangeRoleCommand',
  '2': [
    {'1': 'scope_id', '3': 1, '4': 1, '5': 9, '10': 'scopeId'},
    {'1': 'target', '3': 2, '4': 1, '5': 9, '10': 'target'},
    {'1': 'role', '3': 3, '4': 1, '5': 14, '6': '.gcs.chat.Role', '10': 'role'},
  ],
};

/// Descriptor for `ChangeRoleCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List changeRoleCommandDescriptor = $convert.base64Decode(
    'ChFDaGFuZ2VSb2xlQ29tbWFuZBIZCghzY29wZV9pZBgBIAEoCVIHc2NvcGVJZBIWCgZ0YXJnZX'
    'QYAiABKAlSBnRhcmdldBIiCgRyb2xlGAMgASgOMg4uZ2NzLmNoYXQuUm9sZVIEcm9sZQ==');

@$core.Deprecated('Use removeMemberCommandDescriptor instead')
const RemoveMemberCommand$json = {
  '1': 'RemoveMemberCommand',
  '2': [
    {'1': 'scope_id', '3': 1, '4': 1, '5': 9, '10': 'scopeId'},
    {'1': 'target', '3': 2, '4': 1, '5': 9, '10': 'target'},
  ],
};

/// Descriptor for `RemoveMemberCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List removeMemberCommandDescriptor = $convert.base64Decode(
    'ChNSZW1vdmVNZW1iZXJDb21tYW5kEhkKCHNjb3BlX2lkGAEgASgJUgdzY29wZUlkEhYKBnRhcm'
    'dldBgCIAEoCVIGdGFyZ2V0');

@$core.Deprecated('Use leaveCommandDescriptor instead')
const LeaveCommand$json = {
  '1': 'LeaveCommand',
  '2': [
    {'1': 'scope_id', '3': 1, '4': 1, '5': 9, '10': 'scopeId'},
  ],
};

/// Descriptor for `LeaveCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List leaveCommandDescriptor = $convert
    .base64Decode('CgxMZWF2ZUNvbW1hbmQSGQoIc2NvcGVfaWQYASABKAlSB3Njb3BlSWQ=');

@$core.Deprecated('Use deleteScopeCommandDescriptor instead')
const DeleteScopeCommand$json = {
  '1': 'DeleteScopeCommand',
  '2': [
    {'1': 'scope_id', '3': 1, '4': 1, '5': 9, '10': 'scopeId'},
  ],
};

/// Descriptor for `DeleteScopeCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List deleteScopeCommandDescriptor =
    $convert.base64Decode(
        'ChJEZWxldGVTY29wZUNvbW1hbmQSGQoIc2NvcGVfaWQYASABKAlSB3Njb3BlSWQ=');

@$core.Deprecated('Use approveCommandDescriptor instead')
const ApproveCommand$json = {
  '1': 'ApproveCommand',
  '2': [
    {'1': 'scope_id', '3': 1, '4': 1, '5': 9, '10': 'scopeId'},
    {'1': 'target', '3': 2, '4': 1, '5': 9, '10': 'target'},
  ],
};

/// Descriptor for `ApproveCommand`. Decode as a `google.protobuf.DescriptorProto`.
final $typed_data.Uint8List approveCommandDescriptor = $convert.base64Decode(
    'Cg5BcHByb3ZlQ29tbWFuZBIZCghzY29wZV9pZBgBIAEoCVIHc2NvcGVJZBIWCgZ0YXJnZXQYAi'
    'ABKAlSBnRhcmdldA==');
