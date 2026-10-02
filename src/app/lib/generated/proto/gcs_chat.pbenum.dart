// This is a generated file - do not edit.
//
// Generated from gcs_chat.proto.

// @dart = 3.3

// ignore_for_file: annotate_overrides, camel_case_types, comment_references
// ignore_for_file: constant_identifier_names
// ignore_for_file: curly_braces_in_flow_control_structures
// ignore_for_file: deprecated_member_use_from_same_package, library_prefixes
// ignore_for_file: non_constant_identifier_names

import 'dart:core' as $core;

import 'package:protobuf/protobuf.dart' as $pb;

/// Wire codec bound to a store at creation (D-29: per-store codec choice).
class Codec extends $pb.ProtobufEnum {
  static const Codec CODEC_UNSPECIFIED =
      Codec._(0, _omitEnumNames ? '' : 'CODEC_UNSPECIFIED');
  static const Codec CODEC_PROTOBUF =
      Codec._(1, _omitEnumNames ? '' : 'CODEC_PROTOBUF');
  static const Codec CODEC_JSON =
      Codec._(2, _omitEnumNames ? '' : 'CODEC_JSON');

  static const $core.List<Codec> values = <Codec>[
    CODEC_UNSPECIFIED,
    CODEC_PROTOBUF,
    CODEC_JSON,
  ];

  static final $core.List<Codec?> _byValue =
      $pb.ProtobufEnum.$_initByValueList(values, 2);
  static Codec? valueOf($core.int value) =>
      value < 0 || value >= _byValue.length ? null : _byValue[value];

  const Codec._(super.value, super.name);
}

/// Message role taxonomy (D-18).
class MessageRole extends $pb.ProtobufEnum {
  static const MessageRole MESSAGE_ROLE_UNSPECIFIED =
      MessageRole._(0, _omitEnumNames ? '' : 'MESSAGE_ROLE_UNSPECIFIED');
  static const MessageRole MESSAGE_ROLE_USER_SELF =
      MessageRole._(1, _omitEnumNames ? '' : 'MESSAGE_ROLE_USER_SELF');
  static const MessageRole MESSAGE_ROLE_USER_PEER =
      MessageRole._(2, _omitEnumNames ? '' : 'MESSAGE_ROLE_USER_PEER');
  static const MessageRole MESSAGE_ROLE_ASSISTANT =
      MessageRole._(3, _omitEnumNames ? '' : 'MESSAGE_ROLE_ASSISTANT');
  static const MessageRole MESSAGE_ROLE_SYSTEM =
      MessageRole._(4, _omitEnumNames ? '' : 'MESSAGE_ROLE_SYSTEM');

  static const $core.List<MessageRole> values = <MessageRole>[
    MESSAGE_ROLE_UNSPECIFIED,
    MESSAGE_ROLE_USER_SELF,
    MESSAGE_ROLE_USER_PEER,
    MESSAGE_ROLE_ASSISTANT,
    MESSAGE_ROLE_SYSTEM,
  ];

  static final $core.List<MessageRole?> _byValue =
      $pb.ProtobufEnum.$_initByValueList(values, 4);
  static MessageRole? valueOf($core.int value) =>
      value < 0 || value >= _byValue.length ? null : _byValue[value];

  const MessageRole._(super.value, super.name);
}

/// Message state taxonomy (D-19).
class MessageState extends $pb.ProtobufEnum {
  static const MessageState MESSAGE_STATE_UNSPECIFIED =
      MessageState._(0, _omitEnumNames ? '' : 'MESSAGE_STATE_UNSPECIFIED');
  static const MessageState MESSAGE_STATE_PENDING =
      MessageState._(1, _omitEnumNames ? '' : 'MESSAGE_STATE_PENDING');
  static const MessageState MESSAGE_STATE_STREAMING =
      MessageState._(2, _omitEnumNames ? '' : 'MESSAGE_STATE_STREAMING');
  static const MessageState MESSAGE_STATE_THINKING =
      MessageState._(3, _omitEnumNames ? '' : 'MESSAGE_STATE_THINKING');
  static const MessageState MESSAGE_STATE_COMPLETE =
      MessageState._(4, _omitEnumNames ? '' : 'MESSAGE_STATE_COMPLETE');
  static const MessageState MESSAGE_STATE_ERROR =
      MessageState._(5, _omitEnumNames ? '' : 'MESSAGE_STATE_ERROR');

  static const $core.List<MessageState> values = <MessageState>[
    MESSAGE_STATE_UNSPECIFIED,
    MESSAGE_STATE_PENDING,
    MESSAGE_STATE_STREAMING,
    MESSAGE_STATE_THINKING,
    MESSAGE_STATE_COMPLETE,
    MESSAGE_STATE_ERROR,
  ];

  static final $core.List<MessageState?> _byValue =
      $pb.ProtobufEnum.$_initByValueList(values, 5);
  static MessageState? valueOf($core.int value) =>
      value < 0 || value >= _byValue.length ? null : _byValue[value];

  const MessageState._(super.value, super.name);
}

/// Five-tier capability role (MEMB-02/D-08). C++ stamps the role at mint/redeem/
/// role-change; it is NEVER read from a client-supplied field (T-04-01-EoP).
class Role extends $pb.ProtobufEnum {
  static const Role ROLE_UNSPECIFIED =
      Role._(0, _omitEnumNames ? '' : 'ROLE_UNSPECIFIED');
  static const Role ROLE_SUPER_ADMIN =
      Role._(1, _omitEnumNames ? '' : 'ROLE_SUPER_ADMIN');
  static const Role ROLE_ADMIN = Role._(2, _omitEnumNames ? '' : 'ROLE_ADMIN');
  static const Role ROLE_MODERATOR =
      Role._(3, _omitEnumNames ? '' : 'ROLE_MODERATOR');
  static const Role ROLE_MEMBER =
      Role._(4, _omitEnumNames ? '' : 'ROLE_MEMBER');
  static const Role ROLE_GUEST = Role._(5, _omitEnumNames ? '' : 'ROLE_GUEST');

  static const $core.List<Role> values = <Role>[
    ROLE_UNSPECIFIED,
    ROLE_SUPER_ADMIN,
    ROLE_ADMIN,
    ROLE_MODERATOR,
    ROLE_MEMBER,
    ROLE_GUEST,
  ];

  static final $core.List<Role?> _byValue =
      $pb.ProtobufEnum.$_initByValueList(values, 5);
  static Role? valueOf($core.int value) =>
      value < 0 || value >= _byValue.length ? null : _byValue[value];

  const Role._(super.value, super.name);
}

/// Dormant destructive-op kind carried on the target record (D-07). An empty
/// `approved_by` plus a non-zero action means the op awaits Super Admin activation.
class PendingAction extends $pb.ProtobufEnum {
  static const PendingAction PENDING_ACTION_UNSPECIFIED =
      PendingAction._(0, _omitEnumNames ? '' : 'PENDING_ACTION_UNSPECIFIED');
  static const PendingAction PENDING_ACTION_REMOVE =
      PendingAction._(1, _omitEnumNames ? '' : 'PENDING_ACTION_REMOVE');
  static const PendingAction PENDING_ACTION_ROLE_CHANGE =
      PendingAction._(2, _omitEnumNames ? '' : 'PENDING_ACTION_ROLE_CHANGE');
  static const PendingAction PENDING_ACTION_DELETE =
      PendingAction._(3, _omitEnumNames ? '' : 'PENDING_ACTION_DELETE');

  static const $core.List<PendingAction> values = <PendingAction>[
    PENDING_ACTION_UNSPECIFIED,
    PENDING_ACTION_REMOVE,
    PENDING_ACTION_ROLE_CHANGE,
    PENDING_ACTION_DELETE,
  ];

  static final $core.List<PendingAction?> _byValue =
      $pb.ProtobufEnum.$_initByValueList(values, 3);
  static PendingAction? valueOf($core.int value) =>
      value < 0 || value >= _byValue.length ? null : _byValue[value];

  const PendingAction._(super.value, super.name);
}

const $core.bool _omitEnumNames =
    $core.bool.fromEnvironment('protobuf.omit_enum_names');
