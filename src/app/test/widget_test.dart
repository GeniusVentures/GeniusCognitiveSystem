// Bare-app smoke test (01-03): pumps the placeholder GeniusSwarmApp so the
// default `flutter test` suite compiles and the entry widget renders. The
// real UI tests land with the scaffold-rewrite phase.

import 'package:flutter_test/flutter_test.dart';

import 'package:flutter_app/main.dart';

void main() {
  testWidgets('GeniusSwarmApp builds the placeholder screen', (WidgetTester tester) async {
    // Build the app and trigger a frame.
    await tester.pumpWidget(const GeniusSwarmApp());

    // The placeholder shell renders its app-bar title and body note.
    expect(find.text('GNUS NEO SWARM'), findsOneWidget);
    expect(find.text('GCS app skeleton — UI lands with the scaffold rewrite'), findsOneWidget);
  });
}
