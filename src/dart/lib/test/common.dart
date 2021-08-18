import 'dart:io';

import 'package:test/test.dart';
import '../gitkebab.dart' as gitkebab;

void initGitkebab() {
  var libraryPath = Directory.current.path + "/gitkebab-linux-debug/lib/libgitkebab.so";
  gitkebab.GitKebab.load(libraryPath);
}

String testFixturesPath() {
  return "${Directory.current.path}/../test/fixtures";
}

void recreateTestStagingDirectory() {
  Directory testStagingDir = Directory(testStagingPath());
  if (testStagingDir.existsSync()) {
    testStagingDir.deleteSync(recursive: true);
  }
  testStagingDir.createSync();
}

String testStagingPath() {
  return "${Directory.current.path}/test-staging";
}