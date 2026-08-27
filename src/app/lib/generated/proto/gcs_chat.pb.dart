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

import 'package:fixnum/fixnum.dart' as $fixnum;
import 'package:protobuf/protobuf.dart' as $pb;

import 'gcs_chat.pbenum.dart';

export 'package:protobuf/protobuf.dart' show GeneratedMessageGenericExtensions;

export 'gcs_chat.pbenum.dart';

/// Store/session configuration carried by gcs_init bytes (D-29: codec bound at creation).
class GcsConfig extends $pb.GeneratedMessage {
  factory GcsConfig({
    $core.String? dbPath,
    Codec? codec,
  }) {
    final result = create();
    if (dbPath != null) result.dbPath = dbPath;
    if (codec != null) result.codec = codec;
    return result;
  }

  GcsConfig._();

  factory GcsConfig.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory GcsConfig.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'GcsConfig',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..aOS(1, _omitFieldNames ? '' : 'dbPath')
    ..e<Codec>(2, _omitFieldNames ? '' : 'codec', $pb.PbFieldType.OE,
        defaultOrMaker: Codec.CODEC_UNSPECIFIED,
        valueOf: Codec.valueOf,
        enumValues: Codec.values)
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  GcsConfig clone() => GcsConfig()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  GcsConfig copyWith(void Function(GcsConfig) updates) =>
      super.copyWith((message) => updates(message as GcsConfig)) as GcsConfig;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static GcsConfig create() => GcsConfig._();
  @$core.override
  GcsConfig createEmptyInstance() => create();
  static $pb.PbList<GcsConfig> createRepeated() => $pb.PbList<GcsConfig>();
  @$core.pragma('dart2js:noInline')
  static GcsConfig getDefault() =>
      _defaultInstance ??= $pb.GeneratedMessage.$_defaultFor<GcsConfig>(create);
  static GcsConfig? _defaultInstance;

  @$pb.TagNumber(1)
  $core.String get dbPath => $_getSZ(0);
  @$pb.TagNumber(1)
  set dbPath($core.String value) => $_setString(0, value);
  @$pb.TagNumber(1)
  $core.bool hasDbPath() => $_has(0);
  @$pb.TagNumber(1)
  void clearDbPath() => $_clearField(1);

  @$pb.TagNumber(2)
  Codec get codec => $_getN(1);
  @$pb.TagNumber(2)
  set codec(Codec value) => $_setField(2, value);
  @$pb.TagNumber(2)
  $core.bool hasCodec() => $_has(1);
  @$pb.TagNumber(2)
  void clearCodec() => $_clearField(2);
}

/// Thin chat-shaped struct Dart publishes for a send (D-27: data-only; C++ stamps authority).
class SendTextCommand extends $pb.GeneratedMessage {
  factory SendTextCommand({
    $core.String? roomTopic,
    $core.String? text,
  }) {
    final result = create();
    if (roomTopic != null) result.roomTopic = roomTopic;
    if (text != null) result.text = text;
    return result;
  }

  SendTextCommand._();

  factory SendTextCommand.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory SendTextCommand.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'SendTextCommand',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..aOS(1, _omitFieldNames ? '' : 'roomTopic')
    ..aOS(2, _omitFieldNames ? '' : 'text')
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  SendTextCommand clone() => SendTextCommand()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  SendTextCommand copyWith(void Function(SendTextCommand) updates) =>
      super.copyWith((message) => updates(message as SendTextCommand))
          as SendTextCommand;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static SendTextCommand create() => SendTextCommand._();
  @$core.override
  SendTextCommand createEmptyInstance() => create();
  static $pb.PbList<SendTextCommand> createRepeated() =>
      $pb.PbList<SendTextCommand>();
  @$core.pragma('dart2js:noInline')
  static SendTextCommand getDefault() => _defaultInstance ??=
      $pb.GeneratedMessage.$_defaultFor<SendTextCommand>(create);
  static SendTextCommand? _defaultInstance;

  @$pb.TagNumber(1)
  $core.String get roomTopic => $_getSZ(0);
  @$pb.TagNumber(1)
  set roomTopic($core.String value) => $_setString(0, value);
  @$pb.TagNumber(1)
  $core.bool hasRoomTopic() => $_has(0);
  @$pb.TagNumber(1)
  void clearRoomTopic() => $_clearField(1);

  @$pb.TagNumber(2)
  $core.String get text => $_getSZ(1);
  @$pb.TagNumber(2)
  set text($core.String value) => $_setString(1, value);
  @$pb.TagNumber(2)
  $core.bool hasText() => $_has(1);
  @$pb.TagNumber(2)
  void clearText() => $_clearField(2);
}

/// Join a chat room topic (C++ joins GossipSub and pushes the updated RoomList).
class JoinTopicCommand extends $pb.GeneratedMessage {
  factory JoinTopicCommand({
    $core.String? roomTopic,
  }) {
    final result = create();
    if (roomTopic != null) result.roomTopic = roomTopic;
    return result;
  }

  JoinTopicCommand._();

  factory JoinTopicCommand.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory JoinTopicCommand.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'JoinTopicCommand',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..aOS(1, _omitFieldNames ? '' : 'roomTopic')
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  JoinTopicCommand clone() => JoinTopicCommand()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  JoinTopicCommand copyWith(void Function(JoinTopicCommand) updates) =>
      super.copyWith((message) => updates(message as JoinTopicCommand))
          as JoinTopicCommand;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static JoinTopicCommand create() => JoinTopicCommand._();
  @$core.override
  JoinTopicCommand createEmptyInstance() => create();
  static $pb.PbList<JoinTopicCommand> createRepeated() =>
      $pb.PbList<JoinTopicCommand>();
  @$core.pragma('dart2js:noInline')
  static JoinTopicCommand getDefault() => _defaultInstance ??=
      $pb.GeneratedMessage.$_defaultFor<JoinTopicCommand>(create);
  static JoinTopicCommand? _defaultInstance;

  @$pb.TagNumber(1)
  $core.String get roomTopic => $_getSZ(0);
  @$pb.TagNumber(1)
  set roomTopic($core.String value) => $_setString(0, value);
  @$pb.TagNumber(1)
  $core.bool hasRoomTopic() => $_has(0);
  @$pb.TagNumber(1)
  void clearRoomTopic() => $_clearField(1);
}

enum GcsCommand_Payload { joinTopic, sendText, notSet }

/// Envelope for every Dart -> C++ command publish (D-27: commands are topic publishes).
class GcsCommand extends $pb.GeneratedMessage {
  factory GcsCommand({
    JoinTopicCommand? joinTopic,
    SendTextCommand? sendText,
  }) {
    final result = create();
    if (joinTopic != null) result.joinTopic = joinTopic;
    if (sendText != null) result.sendText = sendText;
    return result;
  }

  GcsCommand._();

  factory GcsCommand.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory GcsCommand.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static const $core.Map<$core.int, GcsCommand_Payload>
      _GcsCommand_PayloadByTag = {
    1: GcsCommand_Payload.joinTopic,
    2: GcsCommand_Payload.sendText,
    0: GcsCommand_Payload.notSet
  };
  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'GcsCommand',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..oo(0, [1, 2])
    ..aOM<JoinTopicCommand>(1, _omitFieldNames ? '' : 'joinTopic',
        subBuilder: JoinTopicCommand.create)
    ..aOM<SendTextCommand>(2, _omitFieldNames ? '' : 'sendText',
        subBuilder: SendTextCommand.create)
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  GcsCommand clone() => GcsCommand()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  GcsCommand copyWith(void Function(GcsCommand) updates) =>
      super.copyWith((message) => updates(message as GcsCommand)) as GcsCommand;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static GcsCommand create() => GcsCommand._();
  @$core.override
  GcsCommand createEmptyInstance() => create();
  static $pb.PbList<GcsCommand> createRepeated() => $pb.PbList<GcsCommand>();
  @$core.pragma('dart2js:noInline')
  static GcsCommand getDefault() => _defaultInstance ??=
      $pb.GeneratedMessage.$_defaultFor<GcsCommand>(create);
  static GcsCommand? _defaultInstance;

  GcsCommand_Payload whichPayload() =>
      _GcsCommand_PayloadByTag[$_whichOneof(0)]!;
  void clearPayload() => $_clearField($_whichOneof(0));

  @$pb.TagNumber(1)
  JoinTopicCommand get joinTopic => $_getN(0);
  @$pb.TagNumber(1)
  set joinTopic(JoinTopicCommand value) => $_setField(1, value);
  @$pb.TagNumber(1)
  $core.bool hasJoinTopic() => $_has(0);
  @$pb.TagNumber(1)
  void clearJoinTopic() => $_clearField(1);
  @$pb.TagNumber(1)
  JoinTopicCommand ensureJoinTopic() => $_ensure(0);

  @$pb.TagNumber(2)
  SendTextCommand get sendText => $_getN(1);
  @$pb.TagNumber(2)
  set sendText(SendTextCommand value) => $_setField(2, value);
  @$pb.TagNumber(2)
  $core.bool hasSendText() => $_has(1);
  @$pb.TagNumber(2)
  void clearSendText() => $_clearField(2);
  @$pb.TagNumber(2)
  SendTextCommand ensureSendText() => $_ensure(1);
}

/// Authoritative message record. This proto type IS the C++ half (D-24 reinterpreted by D-26):
/// C++ stamps id/timestamp/role/state; Dart renders.
class ChatMessageState extends $pb.GeneratedMessage {
  factory ChatMessageState({
    $core.String? id,
    $core.String? roomTopic,
    MessageRole? role,
    MessageState? state,
    $core.String? text,
    $fixnum.Int64? timestamp,
  }) {
    final result = create();
    if (id != null) result.id = id;
    if (roomTopic != null) result.roomTopic = roomTopic;
    if (role != null) result.role = role;
    if (state != null) result.state = state;
    if (text != null) result.text = text;
    if (timestamp != null) result.timestamp = timestamp;
    return result;
  }

  ChatMessageState._();

  factory ChatMessageState.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory ChatMessageState.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'ChatMessageState',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..aOS(1, _omitFieldNames ? '' : 'id')
    ..aOS(2, _omitFieldNames ? '' : 'roomTopic')
    ..e<MessageRole>(3, _omitFieldNames ? '' : 'role', $pb.PbFieldType.OE,
        defaultOrMaker: MessageRole.MESSAGE_ROLE_UNSPECIFIED,
        valueOf: MessageRole.valueOf,
        enumValues: MessageRole.values)
    ..e<MessageState>(4, _omitFieldNames ? '' : 'state', $pb.PbFieldType.OE,
        defaultOrMaker: MessageState.MESSAGE_STATE_UNSPECIFIED,
        valueOf: MessageState.valueOf,
        enumValues: MessageState.values)
    ..aOS(5, _omitFieldNames ? '' : 'text')
    ..aInt64(6, _omitFieldNames ? '' : 'timestamp')
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  ChatMessageState clone() => ChatMessageState()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  ChatMessageState copyWith(void Function(ChatMessageState) updates) =>
      super.copyWith((message) => updates(message as ChatMessageState))
          as ChatMessageState;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static ChatMessageState create() => ChatMessageState._();
  @$core.override
  ChatMessageState createEmptyInstance() => create();
  static $pb.PbList<ChatMessageState> createRepeated() =>
      $pb.PbList<ChatMessageState>();
  @$core.pragma('dart2js:noInline')
  static ChatMessageState getDefault() => _defaultInstance ??=
      $pb.GeneratedMessage.$_defaultFor<ChatMessageState>(create);
  static ChatMessageState? _defaultInstance;

  @$pb.TagNumber(1)
  $core.String get id => $_getSZ(0);
  @$pb.TagNumber(1)
  set id($core.String value) => $_setString(0, value);
  @$pb.TagNumber(1)
  $core.bool hasId() => $_has(0);
  @$pb.TagNumber(1)
  void clearId() => $_clearField(1);

  @$pb.TagNumber(2)
  $core.String get roomTopic => $_getSZ(1);
  @$pb.TagNumber(2)
  set roomTopic($core.String value) => $_setString(1, value);
  @$pb.TagNumber(2)
  $core.bool hasRoomTopic() => $_has(1);
  @$pb.TagNumber(2)
  void clearRoomTopic() => $_clearField(2);

  @$pb.TagNumber(3)
  MessageRole get role => $_getN(2);
  @$pb.TagNumber(3)
  set role(MessageRole value) => $_setField(3, value);
  @$pb.TagNumber(3)
  $core.bool hasRole() => $_has(2);
  @$pb.TagNumber(3)
  void clearRole() => $_clearField(3);

  @$pb.TagNumber(4)
  MessageState get state => $_getN(3);
  @$pb.TagNumber(4)
  set state(MessageState value) => $_setField(4, value);
  @$pb.TagNumber(4)
  $core.bool hasState() => $_has(3);
  @$pb.TagNumber(4)
  void clearState() => $_clearField(4);

  @$pb.TagNumber(5)
  $core.String get text => $_getSZ(4);
  @$pb.TagNumber(5)
  set text($core.String value) => $_setString(4, value);
  @$pb.TagNumber(5)
  $core.bool hasText() => $_has(4);
  @$pb.TagNumber(5)
  void clearText() => $_clearField(5);

  @$pb.TagNumber(6)
  $fixnum.Int64 get timestamp => $_getI64(5);
  @$pb.TagNumber(6)
  set timestamp($fixnum.Int64 value) => $_setInt64(5, value);
  @$pb.TagNumber(6)
  $core.bool hasTimestamp() => $_has(5);
  @$pb.TagNumber(6)
  void clearTimestamp() => $_clearField(6);
}

/// Pushed room-list event (D-21/D-26): the set of topics the session is in.
class RoomList extends $pb.GeneratedMessage {
  factory RoomList({
    $core.Iterable<$core.String>? roomTopic,
  }) {
    final result = create();
    if (roomTopic != null) result.roomTopic.addAll(roomTopic);
    return result;
  }

  RoomList._();

  factory RoomList.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory RoomList.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'RoomList',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..pPS(1, _omitFieldNames ? '' : 'roomTopic')
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  RoomList clone() => RoomList()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  RoomList copyWith(void Function(RoomList) updates) =>
      super.copyWith((message) => updates(message as RoomList)) as RoomList;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static RoomList create() => RoomList._();
  @$core.override
  RoomList createEmptyInstance() => create();
  static $pb.PbList<RoomList> createRepeated() => $pb.PbList<RoomList>();
  @$core.pragma('dart2js:noInline')
  static RoomList getDefault() =>
      _defaultInstance ??= $pb.GeneratedMessage.$_defaultFor<RoomList>(create);
  static RoomList? _defaultInstance;

  @$pb.TagNumber(1)
  $pb.PbList<$core.String> get roomTopic => $_getList(0);
}

/// Pushed readiness event (D-26): true once init + smoke-topic pre-join completes.
class Readiness extends $pb.GeneratedMessage {
  factory Readiness({
    $core.bool? ready,
  }) {
    final result = create();
    if (ready != null) result.ready = ready;
    return result;
  }

  Readiness._();

  factory Readiness.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory Readiness.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'Readiness',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..aOB(1, _omitFieldNames ? '' : 'ready')
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  Readiness clone() => Readiness()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  Readiness copyWith(void Function(Readiness) updates) =>
      super.copyWith((message) => updates(message as Readiness)) as Readiness;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static Readiness create() => Readiness._();
  @$core.override
  Readiness createEmptyInstance() => create();
  static $pb.PbList<Readiness> createRepeated() => $pb.PbList<Readiness>();
  @$core.pragma('dart2js:noInline')
  static Readiness getDefault() =>
      _defaultInstance ??= $pb.GeneratedMessage.$_defaultFor<Readiness>(create);
  static Readiness? _defaultInstance;

  @$pb.TagNumber(1)
  $core.bool get ready => $_getBF(0);
  @$pb.TagNumber(1)
  set ready($core.bool value) => $_setBool(0, value);
  @$pb.TagNumber(1)
  $core.bool hasReady() => $_has(0);
  @$pb.TagNumber(1)
  void clearReady() => $_clearField(1);
}

/// Pushed raw error string (D-29: errors cross FFI as raw strings on the push port).
class ErrorNotice extends $pb.GeneratedMessage {
  factory ErrorNotice({
    $core.String? message,
  }) {
    final result = create();
    if (message != null) result.message = message;
    return result;
  }

  ErrorNotice._();

  factory ErrorNotice.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory ErrorNotice.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'ErrorNotice',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..aOS(1, _omitFieldNames ? '' : 'message')
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  ErrorNotice clone() => ErrorNotice()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  ErrorNotice copyWith(void Function(ErrorNotice) updates) =>
      super.copyWith((message) => updates(message as ErrorNotice))
          as ErrorNotice;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static ErrorNotice create() => ErrorNotice._();
  @$core.override
  ErrorNotice createEmptyInstance() => create();
  static $pb.PbList<ErrorNotice> createRepeated() => $pb.PbList<ErrorNotice>();
  @$core.pragma('dart2js:noInline')
  static ErrorNotice getDefault() => _defaultInstance ??=
      $pb.GeneratedMessage.$_defaultFor<ErrorNotice>(create);
  static ErrorNotice? _defaultInstance;

  @$pb.TagNumber(1)
  $core.String get message => $_getSZ(0);
  @$pb.TagNumber(1)
  set message($core.String value) => $_setString(0, value);
  @$pb.TagNumber(1)
  $core.bool hasMessage() => $_has(0);
  @$pb.TagNumber(1)
  void clearMessage() => $_clearField(1);
}

enum GcsEvent_Payload { message, roomList, readiness, error, notSet }

/// Envelope for every C++ -> Dart pushed event (D-26 push-not-pull).
class GcsEvent extends $pb.GeneratedMessage {
  factory GcsEvent({
    ChatMessageState? message,
    RoomList? roomList,
    Readiness? readiness,
    ErrorNotice? error,
  }) {
    final result = create();
    if (message != null) result.message = message;
    if (roomList != null) result.roomList = roomList;
    if (readiness != null) result.readiness = readiness;
    if (error != null) result.error = error;
    return result;
  }

  GcsEvent._();

  factory GcsEvent.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory GcsEvent.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static const $core.Map<$core.int, GcsEvent_Payload> _GcsEvent_PayloadByTag = {
    1: GcsEvent_Payload.message,
    2: GcsEvent_Payload.roomList,
    3: GcsEvent_Payload.readiness,
    4: GcsEvent_Payload.error,
    0: GcsEvent_Payload.notSet
  };
  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'GcsEvent',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..oo(0, [1, 2, 3, 4])
    ..aOM<ChatMessageState>(1, _omitFieldNames ? '' : 'message',
        subBuilder: ChatMessageState.create)
    ..aOM<RoomList>(2, _omitFieldNames ? '' : 'roomList',
        subBuilder: RoomList.create)
    ..aOM<Readiness>(3, _omitFieldNames ? '' : 'readiness',
        subBuilder: Readiness.create)
    ..aOM<ErrorNotice>(4, _omitFieldNames ? '' : 'error',
        subBuilder: ErrorNotice.create)
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  GcsEvent clone() => GcsEvent()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  GcsEvent copyWith(void Function(GcsEvent) updates) =>
      super.copyWith((message) => updates(message as GcsEvent)) as GcsEvent;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static GcsEvent create() => GcsEvent._();
  @$core.override
  GcsEvent createEmptyInstance() => create();
  static $pb.PbList<GcsEvent> createRepeated() => $pb.PbList<GcsEvent>();
  @$core.pragma('dart2js:noInline')
  static GcsEvent getDefault() =>
      _defaultInstance ??= $pb.GeneratedMessage.$_defaultFor<GcsEvent>(create);
  static GcsEvent? _defaultInstance;

  GcsEvent_Payload whichPayload() => _GcsEvent_PayloadByTag[$_whichOneof(0)]!;
  void clearPayload() => $_clearField($_whichOneof(0));

  @$pb.TagNumber(1)
  ChatMessageState get message => $_getN(0);
  @$pb.TagNumber(1)
  set message(ChatMessageState value) => $_setField(1, value);
  @$pb.TagNumber(1)
  $core.bool hasMessage() => $_has(0);
  @$pb.TagNumber(1)
  void clearMessage() => $_clearField(1);
  @$pb.TagNumber(1)
  ChatMessageState ensureMessage() => $_ensure(0);

  @$pb.TagNumber(2)
  RoomList get roomList => $_getN(1);
  @$pb.TagNumber(2)
  set roomList(RoomList value) => $_setField(2, value);
  @$pb.TagNumber(2)
  $core.bool hasRoomList() => $_has(1);
  @$pb.TagNumber(2)
  void clearRoomList() => $_clearField(2);
  @$pb.TagNumber(2)
  RoomList ensureRoomList() => $_ensure(1);

  @$pb.TagNumber(3)
  Readiness get readiness => $_getN(2);
  @$pb.TagNumber(3)
  set readiness(Readiness value) => $_setField(3, value);
  @$pb.TagNumber(3)
  $core.bool hasReadiness() => $_has(2);
  @$pb.TagNumber(3)
  void clearReadiness() => $_clearField(3);
  @$pb.TagNumber(3)
  Readiness ensureReadiness() => $_ensure(2);

  @$pb.TagNumber(4)
  ErrorNotice get error => $_getN(3);
  @$pb.TagNumber(4)
  set error(ErrorNotice value) => $_setField(4, value);
  @$pb.TagNumber(4)
  $core.bool hasError() => $_has(3);
  @$pb.TagNumber(4)
  void clearError() => $_clearField(4);
  @$pb.TagNumber(4)
  ErrorNotice ensureError() => $_ensure(3);
}

const $core.bool _omitFieldNames =
    $core.bool.fromEnvironment('protobuf.omit_field_names');
const $core.bool _omitMessageNames =
    $core.bool.fromEnvironment('protobuf.omit_message_names');
