/// Root smoke test (plan 01-11 Task 3): the rewritten app boots to the themed
/// [GCSChat] shell.
///
/// Render-only -- no FFI assertion: under the test harness
/// `SessionCubit.openDefault` stays inert (no native library), and the shell
/// still renders rail + flow + composer (composer disabled until readiness
/// would be pushed). This replaces the stock `MyApp` test.
library;

import 'package:flutter_test/flutter_test.dart';
import 'package:frontend_scaffold/components/scaffold_composer.dart';

import 'package:flutter_app/main.dart';
import 'package:flutter_app/shell/gcs_shell.dart';
import 'package:flutter_app/shell/room_rail.dart';

void main() {
  testWidgets('GCSChatApp boots to the GCSChat shell (rail + composer)', (
    WidgetTester tester,
  ) async {
    await tester.pumpWidget(const GCSChatApp());

    expect(find.byType(GCSChat), findsOneWidget);
    expect(find.byType(RoomRail), findsOneWidget);
    expect(find.byType(ScaffoldComposer), findsOneWidget);
  });
}
