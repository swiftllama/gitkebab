import 'dart:io';

import 'package:test/test.dart';
import '../gitkebab.dart' as gk;

import 'common.dart';

final testLogPath = '/tmp/gitkebab-test-log';

void main() {
  gk.GitKebab.logFileThreshold = 10; // force log rotation

  final logFile = File(testLogPath);
  if (logFile.existsSync()) {
    logFile.deleteSync();
    logFile.createSync();
  }
  logFile.writeAsStringSync("01234567891"); // this should be the rotated log

  final rotatedLog = File(gk.GitKebab.rotatedLogPath ?? '$testLogPath-1');
  if (rotatedLog.existsSync()) {
    rotatedLog.deleteSync();
  }

  initGitkebab(logPath:testLogPath);
  recreateTestStagingDirectory();

  String cloneTest1 = "${testStagingPath()}/clone-test-1";
  setUp(() { deleteDirIfExists(cloneTest1); });

  // NOTE: this is somewhat badly written, can only test log rotation once, part of the code being tested is in the setup above
  test('Clone with log', () {

    expect(logFile.existsSync(), true);
    expect(rotatedLog.existsSync(), true);

    String localPath = cloneTest1;
    final session1 = gk.Session("${testFixturesPath()}/simple-repo1.git", gk.RepositorySourceUrlType.FILESYSTEM, localPath, "");
    session1.initialize();
    session1.clone();

    final repoDirectory = Directory(cloneTest1);
    expect(repoDirectory.existsSync(), true);

    final logPath = gk.GitKebab.logPath;
    expect(logPath, isNotNull);
    expect(logFile.existsSync(), true);
    expect(rotatedLog.readAsStringSync(), equals("01234567891"));
  });

  test('log rotation', () {


    gk.GitKebab.logFileThreshold = 10;

    String localPath = cloneTest1;
    final session1 = gk.Session("${testFixturesPath()}/simple-repo1.git", gk.RepositorySourceUrlType.FILESYSTEM, localPath, "");
    session1.initialize();
    session1.clone();

    final repoDirectory = Directory(cloneTest1);
    expect(repoDirectory.existsSync(), true);

    final logPath = gk.GitKebab.logPath;
    expect(logPath, isNotNull);
    final logFile = File(logPath!);
    expect(logFile.existsSync(), true);
  });
}
