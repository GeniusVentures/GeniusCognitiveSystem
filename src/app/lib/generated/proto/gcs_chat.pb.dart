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
    $core.String? mnemonic,
  }) {
    final result = create();
    if (dbPath != null) result.dbPath = dbPath;
    if (codec != null) result.codec = codec;
    if (mnemonic != null) result.mnemonic = mnemonic;
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
    ..aOS(3, _omitFieldNames ? '' : 'mnemonic')
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

  /// Optional mnemonic for the embedded SGNUS node's signing identity. Empty
  /// (default) = the SDK boots with the wallet persisted under the base path,
  /// creating a fresh CHILD WALLET when none exists; that child wallet connects
  /// to a parent wallet through other mechanisms (Child Wallets, later phase).
  /// Non-empty = boot the provided account's wallet directly.
  @$pb.TagNumber(3)
  $core.String get mnemonic => $_getSZ(2);
  @$pb.TagNumber(3)
  set mnemonic($core.String value) => $_setString(2, value);
  @$pb.TagNumber(3)
  $core.bool hasMnemonic() => $_has(2);
  @$pb.TagNumber(3)
  void clearMnemonic() => $_clearField(3);
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

enum GcsCommand_Payload {
  joinTopic,
  sendText,
  createSpace,
  createRoom,
  updateSpace,
  notSet
}

/// Envelope for every Dart -> C++ command publish (D-27: commands are topic publishes).
class GcsCommand extends $pb.GeneratedMessage {
  factory GcsCommand({
    JoinTopicCommand? joinTopic,
    SendTextCommand? sendText,
    CreateSpaceCommand? createSpace,
    CreateRoomCommand? createRoom,
    UpdateSpaceCommand? updateSpace,
  }) {
    final result = create();
    if (joinTopic != null) result.joinTopic = joinTopic;
    if (sendText != null) result.sendText = sendText;
    if (createSpace != null) result.createSpace = createSpace;
    if (createRoom != null) result.createRoom = createRoom;
    if (updateSpace != null) result.updateSpace = updateSpace;
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
    3: GcsCommand_Payload.createSpace,
    4: GcsCommand_Payload.createRoom,
    5: GcsCommand_Payload.updateSpace,
    0: GcsCommand_Payload.notSet
  };
  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'GcsCommand',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..oo(0, [1, 2, 3, 4, 5])
    ..aOM<JoinTopicCommand>(1, _omitFieldNames ? '' : 'joinTopic',
        subBuilder: JoinTopicCommand.create)
    ..aOM<SendTextCommand>(2, _omitFieldNames ? '' : 'sendText',
        subBuilder: SendTextCommand.create)
    ..aOM<CreateSpaceCommand>(3, _omitFieldNames ? '' : 'createSpace',
        subBuilder: CreateSpaceCommand.create)
    ..aOM<CreateRoomCommand>(4, _omitFieldNames ? '' : 'createRoom',
        subBuilder: CreateRoomCommand.create)
    ..aOM<UpdateSpaceCommand>(5, _omitFieldNames ? '' : 'updateSpace',
        subBuilder: UpdateSpaceCommand.create)
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

  @$pb.TagNumber(3)
  CreateSpaceCommand get createSpace => $_getN(2);
  @$pb.TagNumber(3)
  set createSpace(CreateSpaceCommand value) => $_setField(3, value);
  @$pb.TagNumber(3)
  $core.bool hasCreateSpace() => $_has(2);
  @$pb.TagNumber(3)
  void clearCreateSpace() => $_clearField(3);
  @$pb.TagNumber(3)
  CreateSpaceCommand ensureCreateSpace() => $_ensure(2);

  @$pb.TagNumber(4)
  CreateRoomCommand get createRoom => $_getN(3);
  @$pb.TagNumber(4)
  set createRoom(CreateRoomCommand value) => $_setField(4, value);
  @$pb.TagNumber(4)
  $core.bool hasCreateRoom() => $_has(3);
  @$pb.TagNumber(4)
  void clearCreateRoom() => $_clearField(4);
  @$pb.TagNumber(4)
  CreateRoomCommand ensureCreateRoom() => $_ensure(3);

  @$pb.TagNumber(5)
  UpdateSpaceCommand get updateSpace => $_getN(4);
  @$pb.TagNumber(5)
  set updateSpace(UpdateSpaceCommand value) => $_setField(5, value);
  @$pb.TagNumber(5)
  $core.bool hasUpdateSpace() => $_has(4);
  @$pb.TagNumber(5)
  void clearUpdateSpace() => $_clearField(5);
  @$pb.TagNumber(5)
  UpdateSpaceCommand ensureUpdateSpace() => $_ensure(4);
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
    $core.String? sender,
    $core.bool? deleted,
    $fixnum.Int64? deletedAtMs,
  }) {
    final result = create();
    if (id != null) result.id = id;
    if (roomTopic != null) result.roomTopic = roomTopic;
    if (role != null) result.role = role;
    if (state != null) result.state = state;
    if (text != null) result.text = text;
    if (timestamp != null) result.timestamp = timestamp;
    if (sender != null) result.sender = sender;
    if (deleted != null) result.deleted = deleted;
    if (deletedAtMs != null) result.deletedAtMs = deletedAtMs;
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
    ..aOS(7, _omitFieldNames ? '' : 'sender')
    ..aOB(8, _omitFieldNames ? '' : 'deleted')
    ..aInt64(9, _omitFieldNames ? '' : 'deletedAtMs')
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

  @$pb.TagNumber(7)
  $core.String get sender => $_getSZ(6);
  @$pb.TagNumber(7)
  set sender($core.String value) => $_setString(6, value);
  @$pb.TagNumber(7)
  $core.bool hasSender() => $_has(6);
  @$pb.TagNumber(7)
  void clearSender() => $_clearField(7);

  @$pb.TagNumber(8)
  $core.bool get deleted => $_getBF(7);
  @$pb.TagNumber(8)
  set deleted($core.bool value) => $_setBool(7, value);
  @$pb.TagNumber(8)
  $core.bool hasDeleted() => $_has(7);
  @$pb.TagNumber(8)
  void clearDeleted() => $_clearField(8);

  @$pb.TagNumber(9)
  $fixnum.Int64 get deletedAtMs => $_getI64(8);
  @$pb.TagNumber(9)
  set deletedAtMs($fixnum.Int64 value) => $_setInt64(8, value);
  @$pb.TagNumber(9)
  $core.bool hasDeletedAtMs() => $_has(8);
  @$pb.TagNumber(9)
  void clearDeletedAtMs() => $_clearField(9);
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

enum GcsEvent_Payload {
  message,
  roomList,
  readiness,
  error,
  spaceTree,
  messageHistory,
  notSet
}

/// Envelope for every C++ -> Dart pushed event (D-26 push-not-pull).
class GcsEvent extends $pb.GeneratedMessage {
  factory GcsEvent({
    ChatMessageState? message,
    RoomList? roomList,
    Readiness? readiness,
    ErrorNotice? error,
    SpaceTree? spaceTree,
    MessageHistory? messageHistory,
  }) {
    final result = create();
    if (message != null) result.message = message;
    if (roomList != null) result.roomList = roomList;
    if (readiness != null) result.readiness = readiness;
    if (error != null) result.error = error;
    if (spaceTree != null) result.spaceTree = spaceTree;
    if (messageHistory != null) result.messageHistory = messageHistory;
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
    5: GcsEvent_Payload.spaceTree,
    6: GcsEvent_Payload.messageHistory,
    0: GcsEvent_Payload.notSet
  };
  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'GcsEvent',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..oo(0, [1, 2, 3, 4, 5, 6])
    ..aOM<ChatMessageState>(1, _omitFieldNames ? '' : 'message',
        subBuilder: ChatMessageState.create)
    ..aOM<RoomList>(2, _omitFieldNames ? '' : 'roomList',
        subBuilder: RoomList.create)
    ..aOM<Readiness>(3, _omitFieldNames ? '' : 'readiness',
        subBuilder: Readiness.create)
    ..aOM<ErrorNotice>(4, _omitFieldNames ? '' : 'error',
        subBuilder: ErrorNotice.create)
    ..aOM<SpaceTree>(5, _omitFieldNames ? '' : 'spaceTree',
        subBuilder: SpaceTree.create)
    ..aOM<MessageHistory>(6, _omitFieldNames ? '' : 'messageHistory',
        subBuilder: MessageHistory.create)
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

  @$pb.TagNumber(5)
  SpaceTree get spaceTree => $_getN(4);
  @$pb.TagNumber(5)
  set spaceTree(SpaceTree value) => $_setField(5, value);
  @$pb.TagNumber(5)
  $core.bool hasSpaceTree() => $_has(4);
  @$pb.TagNumber(5)
  void clearSpaceTree() => $_clearField(5);
  @$pb.TagNumber(5)
  SpaceTree ensureSpaceTree() => $_ensure(4);

  @$pb.TagNumber(6)
  MessageHistory get messageHistory => $_getN(5);
  @$pb.TagNumber(6)
  set messageHistory(MessageHistory value) => $_setField(6, value);
  @$pb.TagNumber(6)
  $core.bool hasMessageHistory() => $_has(5);
  @$pb.TagNumber(6)
  void clearMessageHistory() => $_clearField(6);
  @$pb.TagNumber(6)
  MessageHistory ensureMessageHistory() => $_ensure(5);
}

/// Authoritative persistent space entity record. This proto type IS the C++ half
/// (D-01/D-02): C++ stamps id/timestamps/tombstone; Dart renders. Stored as the
/// value bytes under `gcs/entities/spaces/<id>`. Name is mutable display
/// metadata; id is identity.
class SpaceRecord extends $pb.GeneratedMessage {
  factory SpaceRecord({
    $core.String? id,
    $core.String? name,
    $core.bool? isPublic,
    $core.bool? autoJoinRooms,
    $fixnum.Int64? createdAtMs,
    $fixnum.Int64? updatedAtMs,
    $core.bool? deleted,
    $fixnum.Int64? deletedAtMs,
  }) {
    final result = create();
    if (id != null) result.id = id;
    if (name != null) result.name = name;
    if (isPublic != null) result.isPublic = isPublic;
    if (autoJoinRooms != null) result.autoJoinRooms = autoJoinRooms;
    if (createdAtMs != null) result.createdAtMs = createdAtMs;
    if (updatedAtMs != null) result.updatedAtMs = updatedAtMs;
    if (deleted != null) result.deleted = deleted;
    if (deletedAtMs != null) result.deletedAtMs = deletedAtMs;
    return result;
  }

  SpaceRecord._();

  factory SpaceRecord.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory SpaceRecord.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'SpaceRecord',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..aOS(1, _omitFieldNames ? '' : 'id')
    ..aOS(2, _omitFieldNames ? '' : 'name')
    ..aOB(3, _omitFieldNames ? '' : 'isPublic')
    ..aOB(4, _omitFieldNames ? '' : 'autoJoinRooms')
    ..aInt64(5, _omitFieldNames ? '' : 'createdAtMs')
    ..aInt64(6, _omitFieldNames ? '' : 'updatedAtMs')
    ..aOB(7, _omitFieldNames ? '' : 'deleted')
    ..aInt64(8, _omitFieldNames ? '' : 'deletedAtMs')
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  SpaceRecord clone() => SpaceRecord()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  SpaceRecord copyWith(void Function(SpaceRecord) updates) =>
      super.copyWith((message) => updates(message as SpaceRecord))
          as SpaceRecord;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static SpaceRecord create() => SpaceRecord._();
  @$core.override
  SpaceRecord createEmptyInstance() => create();
  static $pb.PbList<SpaceRecord> createRepeated() => $pb.PbList<SpaceRecord>();
  @$core.pragma('dart2js:noInline')
  static SpaceRecord getDefault() => _defaultInstance ??=
      $pb.GeneratedMessage.$_defaultFor<SpaceRecord>(create);
  static SpaceRecord? _defaultInstance;

  @$pb.TagNumber(1)
  $core.String get id => $_getSZ(0);
  @$pb.TagNumber(1)
  set id($core.String value) => $_setString(0, value);
  @$pb.TagNumber(1)
  $core.bool hasId() => $_has(0);
  @$pb.TagNumber(1)
  void clearId() => $_clearField(1);

  @$pb.TagNumber(2)
  $core.String get name => $_getSZ(1);
  @$pb.TagNumber(2)
  set name($core.String value) => $_setString(1, value);
  @$pb.TagNumber(2)
  $core.bool hasName() => $_has(1);
  @$pb.TagNumber(2)
  void clearName() => $_clearField(2);

  @$pb.TagNumber(3)
  $core.bool get isPublic => $_getBF(2);
  @$pb.TagNumber(3)
  set isPublic($core.bool value) => $_setBool(2, value);
  @$pb.TagNumber(3)
  $core.bool hasIsPublic() => $_has(2);
  @$pb.TagNumber(3)
  void clearIsPublic() => $_clearField(3);

  @$pb.TagNumber(4)
  $core.bool get autoJoinRooms => $_getBF(3);
  @$pb.TagNumber(4)
  set autoJoinRooms($core.bool value) => $_setBool(3, value);
  @$pb.TagNumber(4)
  $core.bool hasAutoJoinRooms() => $_has(3);
  @$pb.TagNumber(4)
  void clearAutoJoinRooms() => $_clearField(4);

  @$pb.TagNumber(5)
  $fixnum.Int64 get createdAtMs => $_getI64(4);
  @$pb.TagNumber(5)
  set createdAtMs($fixnum.Int64 value) => $_setInt64(4, value);
  @$pb.TagNumber(5)
  $core.bool hasCreatedAtMs() => $_has(4);
  @$pb.TagNumber(5)
  void clearCreatedAtMs() => $_clearField(5);

  @$pb.TagNumber(6)
  $fixnum.Int64 get updatedAtMs => $_getI64(5);
  @$pb.TagNumber(6)
  set updatedAtMs($fixnum.Int64 value) => $_setInt64(5, value);
  @$pb.TagNumber(6)
  $core.bool hasUpdatedAtMs() => $_has(5);
  @$pb.TagNumber(6)
  void clearUpdatedAtMs() => $_clearField(6);

  @$pb.TagNumber(7)
  $core.bool get deleted => $_getBF(6);
  @$pb.TagNumber(7)
  set deleted($core.bool value) => $_setBool(6, value);
  @$pb.TagNumber(7)
  $core.bool hasDeleted() => $_has(6);
  @$pb.TagNumber(7)
  void clearDeleted() => $_clearField(7);

  @$pb.TagNumber(8)
  $fixnum.Int64 get deletedAtMs => $_getI64(7);
  @$pb.TagNumber(8)
  set deletedAtMs($fixnum.Int64 value) => $_setInt64(7, value);
  @$pb.TagNumber(8)
  $core.bool hasDeletedAtMs() => $_has(7);
  @$pb.TagNumber(8)
  void clearDeletedAtMs() => $_clearField(8);
}

/// Authoritative persistent room entity record. C++ stamps id/timestamps/
/// tombstone; Dart renders. Stored as the value bytes under
/// `gcs/entities/rooms/<id>`. Topic is derived: `gcs/chat/<id>`.
class RoomRecord extends $pb.GeneratedMessage {
  factory RoomRecord({
    $core.String? id,
    $core.String? name,
    $core.String? parentSpaceId,
    $fixnum.Int64? createdAtMs,
    $fixnum.Int64? updatedAtMs,
    $core.bool? deleted,
    $fixnum.Int64? deletedAtMs,
  }) {
    final result = create();
    if (id != null) result.id = id;
    if (name != null) result.name = name;
    if (parentSpaceId != null) result.parentSpaceId = parentSpaceId;
    if (createdAtMs != null) result.createdAtMs = createdAtMs;
    if (updatedAtMs != null) result.updatedAtMs = updatedAtMs;
    if (deleted != null) result.deleted = deleted;
    if (deletedAtMs != null) result.deletedAtMs = deletedAtMs;
    return result;
  }

  RoomRecord._();

  factory RoomRecord.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory RoomRecord.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'RoomRecord',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..aOS(1, _omitFieldNames ? '' : 'id')
    ..aOS(2, _omitFieldNames ? '' : 'name')
    ..aOS(3, _omitFieldNames ? '' : 'parentSpaceId')
    ..aInt64(4, _omitFieldNames ? '' : 'createdAtMs')
    ..aInt64(5, _omitFieldNames ? '' : 'updatedAtMs')
    ..aOB(6, _omitFieldNames ? '' : 'deleted')
    ..aInt64(7, _omitFieldNames ? '' : 'deletedAtMs')
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  RoomRecord clone() => RoomRecord()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  RoomRecord copyWith(void Function(RoomRecord) updates) =>
      super.copyWith((message) => updates(message as RoomRecord)) as RoomRecord;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static RoomRecord create() => RoomRecord._();
  @$core.override
  RoomRecord createEmptyInstance() => create();
  static $pb.PbList<RoomRecord> createRepeated() => $pb.PbList<RoomRecord>();
  @$core.pragma('dart2js:noInline')
  static RoomRecord getDefault() => _defaultInstance ??=
      $pb.GeneratedMessage.$_defaultFor<RoomRecord>(create);
  static RoomRecord? _defaultInstance;

  @$pb.TagNumber(1)
  $core.String get id => $_getSZ(0);
  @$pb.TagNumber(1)
  set id($core.String value) => $_setString(0, value);
  @$pb.TagNumber(1)
  $core.bool hasId() => $_has(0);
  @$pb.TagNumber(1)
  void clearId() => $_clearField(1);

  @$pb.TagNumber(2)
  $core.String get name => $_getSZ(1);
  @$pb.TagNumber(2)
  set name($core.String value) => $_setString(1, value);
  @$pb.TagNumber(2)
  $core.bool hasName() => $_has(1);
  @$pb.TagNumber(2)
  void clearName() => $_clearField(2);

  @$pb.TagNumber(3)
  $core.String get parentSpaceId => $_getSZ(2);
  @$pb.TagNumber(3)
  set parentSpaceId($core.String value) => $_setString(2, value);
  @$pb.TagNumber(3)
  $core.bool hasParentSpaceId() => $_has(2);
  @$pb.TagNumber(3)
  void clearParentSpaceId() => $_clearField(3);

  @$pb.TagNumber(4)
  $fixnum.Int64 get createdAtMs => $_getI64(3);
  @$pb.TagNumber(4)
  set createdAtMs($fixnum.Int64 value) => $_setInt64(3, value);
  @$pb.TagNumber(4)
  $core.bool hasCreatedAtMs() => $_has(3);
  @$pb.TagNumber(4)
  void clearCreatedAtMs() => $_clearField(4);

  @$pb.TagNumber(5)
  $fixnum.Int64 get updatedAtMs => $_getI64(4);
  @$pb.TagNumber(5)
  set updatedAtMs($fixnum.Int64 value) => $_setInt64(4, value);
  @$pb.TagNumber(5)
  $core.bool hasUpdatedAtMs() => $_has(4);
  @$pb.TagNumber(5)
  void clearUpdatedAtMs() => $_clearField(5);

  @$pb.TagNumber(6)
  $core.bool get deleted => $_getBF(5);
  @$pb.TagNumber(6)
  set deleted($core.bool value) => $_setBool(5, value);
  @$pb.TagNumber(6)
  $core.bool hasDeleted() => $_has(5);
  @$pb.TagNumber(6)
  void clearDeleted() => $_clearField(6);

  @$pb.TagNumber(7)
  $fixnum.Int64 get deletedAtMs => $_getI64(6);
  @$pb.TagNumber(7)
  set deletedAtMs($fixnum.Int64 value) => $_setInt64(6, value);
  @$pb.TagNumber(7)
  $core.bool hasDeletedAtMs() => $_has(6);
  @$pb.TagNumber(7)
  void clearDeletedAtMs() => $_clearField(7);
}

/// Manifest of known entity ids (D-02). Value bytes under gcs/index/manifest.
/// Exists because GcsGlobalDb has no enumeration/prefix query. Writers follow a
/// read-union-write convention (append-only membership); readers skip ids whose
/// record is missing or tombstoned.
class EntityManifest extends $pb.GeneratedMessage {
  factory EntityManifest({
    $core.Iterable<$core.String>? spaceId,
    $core.Iterable<$core.String>? roomId,
  }) {
    final result = create();
    if (spaceId != null) result.spaceId.addAll(spaceId);
    if (roomId != null) result.roomId.addAll(roomId);
    return result;
  }

  EntityManifest._();

  factory EntityManifest.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory EntityManifest.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'EntityManifest',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..pPS(1, _omitFieldNames ? '' : 'spaceId')
    ..pPS(2, _omitFieldNames ? '' : 'roomId')
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  EntityManifest clone() => EntityManifest()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  EntityManifest copyWith(void Function(EntityManifest) updates) =>
      super.copyWith((message) => updates(message as EntityManifest))
          as EntityManifest;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static EntityManifest create() => EntityManifest._();
  @$core.override
  EntityManifest createEmptyInstance() => create();
  static $pb.PbList<EntityManifest> createRepeated() =>
      $pb.PbList<EntityManifest>();
  @$core.pragma('dart2js:noInline')
  static EntityManifest getDefault() => _defaultInstance ??=
      $pb.GeneratedMessage.$_defaultFor<EntityManifest>(create);
  static EntityManifest? _defaultInstance;

  @$pb.TagNumber(1)
  $pb.PbList<$core.String> get spaceId => $_getList(0);

  @$pb.TagNumber(2)
  $pb.PbList<$core.String> get roomId => $_getList(1);
}

/// Data-only create command (D-27: data-only; C++ stamps authority — the id,
/// timestamps, and tombstone fields never come from Dart).
class CreateSpaceCommand extends $pb.GeneratedMessage {
  factory CreateSpaceCommand({
    $core.String? name,
    $core.bool? isPublic,
    $core.bool? autoJoinRooms,
  }) {
    final result = create();
    if (name != null) result.name = name;
    if (isPublic != null) result.isPublic = isPublic;
    if (autoJoinRooms != null) result.autoJoinRooms = autoJoinRooms;
    return result;
  }

  CreateSpaceCommand._();

  factory CreateSpaceCommand.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory CreateSpaceCommand.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'CreateSpaceCommand',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..aOS(1, _omitFieldNames ? '' : 'name')
    ..aOB(2, _omitFieldNames ? '' : 'isPublic')
    ..aOB(3, _omitFieldNames ? '' : 'autoJoinRooms')
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  CreateSpaceCommand clone() => CreateSpaceCommand()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  CreateSpaceCommand copyWith(void Function(CreateSpaceCommand) updates) =>
      super.copyWith((message) => updates(message as CreateSpaceCommand))
          as CreateSpaceCommand;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static CreateSpaceCommand create() => CreateSpaceCommand._();
  @$core.override
  CreateSpaceCommand createEmptyInstance() => create();
  static $pb.PbList<CreateSpaceCommand> createRepeated() =>
      $pb.PbList<CreateSpaceCommand>();
  @$core.pragma('dart2js:noInline')
  static CreateSpaceCommand getDefault() => _defaultInstance ??=
      $pb.GeneratedMessage.$_defaultFor<CreateSpaceCommand>(create);
  static CreateSpaceCommand? _defaultInstance;

  @$pb.TagNumber(1)
  $core.String get name => $_getSZ(0);
  @$pb.TagNumber(1)
  set name($core.String value) => $_setString(0, value);
  @$pb.TagNumber(1)
  $core.bool hasName() => $_has(0);
  @$pb.TagNumber(1)
  void clearName() => $_clearField(1);

  @$pb.TagNumber(2)
  $core.bool get isPublic => $_getBF(1);
  @$pb.TagNumber(2)
  set isPublic($core.bool value) => $_setBool(1, value);
  @$pb.TagNumber(2)
  $core.bool hasIsPublic() => $_has(1);
  @$pb.TagNumber(2)
  void clearIsPublic() => $_clearField(2);

  @$pb.TagNumber(3)
  $core.bool get autoJoinRooms => $_getBF(2);
  @$pb.TagNumber(3)
  set autoJoinRooms($core.bool value) => $_setBool(2, value);
  @$pb.TagNumber(3)
  $core.bool hasAutoJoinRooms() => $_has(2);
  @$pb.TagNumber(3)
  void clearAutoJoinRooms() => $_clearField(3);
}

/// Data-only create command (D-27: data-only; C++ stamps authority).
class CreateRoomCommand extends $pb.GeneratedMessage {
  factory CreateRoomCommand({
    $core.String? name,
    $core.String? parentSpaceId,
  }) {
    final result = create();
    if (name != null) result.name = name;
    if (parentSpaceId != null) result.parentSpaceId = parentSpaceId;
    return result;
  }

  CreateRoomCommand._();

  factory CreateRoomCommand.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory CreateRoomCommand.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'CreateRoomCommand',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..aOS(1, _omitFieldNames ? '' : 'name')
    ..aOS(2, _omitFieldNames ? '' : 'parentSpaceId')
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  CreateRoomCommand clone() => CreateRoomCommand()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  CreateRoomCommand copyWith(void Function(CreateRoomCommand) updates) =>
      super.copyWith((message) => updates(message as CreateRoomCommand))
          as CreateRoomCommand;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static CreateRoomCommand create() => CreateRoomCommand._();
  @$core.override
  CreateRoomCommand createEmptyInstance() => create();
  static $pb.PbList<CreateRoomCommand> createRepeated() =>
      $pb.PbList<CreateRoomCommand>();
  @$core.pragma('dart2js:noInline')
  static CreateRoomCommand getDefault() => _defaultInstance ??=
      $pb.GeneratedMessage.$_defaultFor<CreateRoomCommand>(create);
  static CreateRoomCommand? _defaultInstance;

  @$pb.TagNumber(1)
  $core.String get name => $_getSZ(0);
  @$pb.TagNumber(1)
  set name($core.String value) => $_setString(0, value);
  @$pb.TagNumber(1)
  $core.bool hasName() => $_has(0);
  @$pb.TagNumber(1)
  void clearName() => $_clearField(1);

  @$pb.TagNumber(2)
  $core.String get parentSpaceId => $_getSZ(1);
  @$pb.TagNumber(2)
  set parentSpaceId($core.String value) => $_setString(1, value);
  @$pb.TagNumber(2)
  $core.bool hasParentSpaceId() => $_has(1);
  @$pb.TagNumber(2)
  void clearParentSpaceId() => $_clearField(2);
}

/// Data-only edit command (D-27: data-only; C++ stamps authority). Full desired
/// state, never a patch (per-key LWW replaces the whole value).
class UpdateSpaceCommand extends $pb.GeneratedMessage {
  factory UpdateSpaceCommand({
    $core.String? spaceId,
    $core.String? name,
    $core.bool? isPublic,
    $core.bool? autoJoinRooms,
  }) {
    final result = create();
    if (spaceId != null) result.spaceId = spaceId;
    if (name != null) result.name = name;
    if (isPublic != null) result.isPublic = isPublic;
    if (autoJoinRooms != null) result.autoJoinRooms = autoJoinRooms;
    return result;
  }

  UpdateSpaceCommand._();

  factory UpdateSpaceCommand.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory UpdateSpaceCommand.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'UpdateSpaceCommand',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..aOS(1, _omitFieldNames ? '' : 'spaceId')
    ..aOS(2, _omitFieldNames ? '' : 'name')
    ..aOB(3, _omitFieldNames ? '' : 'isPublic')
    ..aOB(4, _omitFieldNames ? '' : 'autoJoinRooms')
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  UpdateSpaceCommand clone() => UpdateSpaceCommand()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  UpdateSpaceCommand copyWith(void Function(UpdateSpaceCommand) updates) =>
      super.copyWith((message) => updates(message as UpdateSpaceCommand))
          as UpdateSpaceCommand;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static UpdateSpaceCommand create() => UpdateSpaceCommand._();
  @$core.override
  UpdateSpaceCommand createEmptyInstance() => create();
  static $pb.PbList<UpdateSpaceCommand> createRepeated() =>
      $pb.PbList<UpdateSpaceCommand>();
  @$core.pragma('dart2js:noInline')
  static UpdateSpaceCommand getDefault() => _defaultInstance ??=
      $pb.GeneratedMessage.$_defaultFor<UpdateSpaceCommand>(create);
  static UpdateSpaceCommand? _defaultInstance;

  @$pb.TagNumber(1)
  $core.String get spaceId => $_getSZ(0);
  @$pb.TagNumber(1)
  set spaceId($core.String value) => $_setString(0, value);
  @$pb.TagNumber(1)
  $core.bool hasSpaceId() => $_has(0);
  @$pb.TagNumber(1)
  void clearSpaceId() => $_clearField(1);

  @$pb.TagNumber(2)
  $core.String get name => $_getSZ(1);
  @$pb.TagNumber(2)
  set name($core.String value) => $_setString(1, value);
  @$pb.TagNumber(2)
  $core.bool hasName() => $_has(1);
  @$pb.TagNumber(2)
  void clearName() => $_clearField(2);

  @$pb.TagNumber(3)
  $core.bool get isPublic => $_getBF(2);
  @$pb.TagNumber(3)
  set isPublic($core.bool value) => $_setBool(2, value);
  @$pb.TagNumber(3)
  $core.bool hasIsPublic() => $_has(2);
  @$pb.TagNumber(3)
  void clearIsPublic() => $_clearField(3);

  @$pb.TagNumber(4)
  $core.bool get autoJoinRooms => $_getBF(3);
  @$pb.TagNumber(4)
  set autoJoinRooms($core.bool value) => $_setBool(3, value);
  @$pb.TagNumber(4)
  $core.bool hasAutoJoinRooms() => $_has(3);
  @$pb.TagNumber(4)
  void clearAutoJoinRooms() => $_clearField(4);
}

/// Pushed entity-catalog event (D-02): FLAT records — Dart builds the tree.
/// Reuses the same record protos stored in the KV values (one type, no drift).
class SpaceTree extends $pb.GeneratedMessage {
  factory SpaceTree({
    $core.Iterable<SpaceRecord>? space,
    $core.Iterable<RoomRecord>? room,
  }) {
    final result = create();
    if (space != null) result.space.addAll(space);
    if (room != null) result.room.addAll(room);
    return result;
  }

  SpaceTree._();

  factory SpaceTree.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory SpaceTree.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'SpaceTree',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..pc<SpaceRecord>(1, _omitFieldNames ? '' : 'space', $pb.PbFieldType.PM,
        subBuilder: SpaceRecord.create)
    ..pc<RoomRecord>(2, _omitFieldNames ? '' : 'room', $pb.PbFieldType.PM,
        subBuilder: RoomRecord.create)
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  SpaceTree clone() => SpaceTree()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  SpaceTree copyWith(void Function(SpaceTree) updates) =>
      super.copyWith((message) => updates(message as SpaceTree)) as SpaceTree;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static SpaceTree create() => SpaceTree._();
  @$core.override
  SpaceTree createEmptyInstance() => create();
  static $pb.PbList<SpaceTree> createRepeated() => $pb.PbList<SpaceTree>();
  @$core.pragma('dart2js:noInline')
  static SpaceTree getDefault() =>
      _defaultInstance ??= $pb.GeneratedMessage.$_defaultFor<SpaceTree>(create);
  static SpaceTree? _defaultInstance;

  @$pb.TagNumber(1)
  $pb.PbList<SpaceRecord> get space => $_getList(0);

  @$pb.TagNumber(2)
  $pb.PbList<RoomRecord> get room => $_getList(1);
}

/// Batch history replay (D-06): the full sorted room history pushed on join.
class MessageHistory extends $pb.GeneratedMessage {
  factory MessageHistory({
    $core.String? roomTopic,
    $core.Iterable<ChatMessageState>? message,
  }) {
    final result = create();
    if (roomTopic != null) result.roomTopic = roomTopic;
    if (message != null) result.message.addAll(message);
    return result;
  }

  MessageHistory._();

  factory MessageHistory.fromBuffer($core.List<$core.int> data,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromBuffer(data, registry);
  factory MessageHistory.fromJson($core.String json,
          [$pb.ExtensionRegistry registry = $pb.ExtensionRegistry.EMPTY]) =>
      create()..mergeFromJson(json, registry);

  static final $pb.BuilderInfo _i = $pb.BuilderInfo(
      _omitMessageNames ? '' : 'MessageHistory',
      package: const $pb.PackageName(_omitMessageNames ? '' : 'gcs.chat'),
      createEmptyInstance: create)
    ..aOS(1, _omitFieldNames ? '' : 'roomTopic')
    ..pc<ChatMessageState>(
        2, _omitFieldNames ? '' : 'message', $pb.PbFieldType.PM,
        subBuilder: ChatMessageState.create)
    ..hasRequiredFields = false;

  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  MessageHistory clone() => MessageHistory()..mergeFromMessage(this);
  @$core.Deprecated('See https://github.com/google/protobuf.dart/issues/998.')
  MessageHistory copyWith(void Function(MessageHistory) updates) =>
      super.copyWith((message) => updates(message as MessageHistory))
          as MessageHistory;

  @$core.override
  $pb.BuilderInfo get info_ => _i;

  @$core.pragma('dart2js:noInline')
  static MessageHistory create() => MessageHistory._();
  @$core.override
  MessageHistory createEmptyInstance() => create();
  static $pb.PbList<MessageHistory> createRepeated() =>
      $pb.PbList<MessageHistory>();
  @$core.pragma('dart2js:noInline')
  static MessageHistory getDefault() => _defaultInstance ??=
      $pb.GeneratedMessage.$_defaultFor<MessageHistory>(create);
  static MessageHistory? _defaultInstance;

  @$pb.TagNumber(1)
  $core.String get roomTopic => $_getSZ(0);
  @$pb.TagNumber(1)
  set roomTopic($core.String value) => $_setString(0, value);
  @$pb.TagNumber(1)
  $core.bool hasRoomTopic() => $_has(0);
  @$pb.TagNumber(1)
  void clearRoomTopic() => $_clearField(1);

  @$pb.TagNumber(2)
  $pb.PbList<ChatMessageState> get message => $_getList(1);
}

const $core.bool _omitFieldNames =
    $core.bool.fromEnvironment('protobuf.omit_field_names');
const $core.bool _omitMessageNames =
    $core.bool.fromEnvironment('protobuf.omit_message_names');
