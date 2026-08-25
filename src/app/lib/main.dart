// Bare app entry point — placeholder until the 01-05 Dart rewrite composes
// the real UI from scaffold primitives (D-10: flutter_chat_ui/core dropped).
// Kept compilable so `flutter build macos` / CI app targets work.
import 'package:flutter/material.dart';

void main() {
  runApp(const GeniusSwarmApp());
}

class GeniusSwarmApp extends StatelessWidget {
  const GeniusSwarmApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'GNUS NEO SWARM',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(
          seedColor: const Color(0xFF6C3CE1), // GNUS purple
          brightness: Brightness.dark,
        ),
        useMaterial3: true,
      ),
      home: const PlaceholderScreen(),
    );
  }
}

class PlaceholderScreen extends StatelessWidget {
  const PlaceholderScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('GNUS NEO SWARM')),
      body: const Center(
        child: Text('GCS app skeleton — UI lands with the scaffold rewrite'),
      ),
    );
  }
}
