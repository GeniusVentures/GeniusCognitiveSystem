// This is a generated file - do not edit.
//
// Generated from proto/gcs_chat.proto.

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

const $core.bool _omitEnumNames =
    $core.bool.fromEnvironment('protobuf.omit_enum_names');
