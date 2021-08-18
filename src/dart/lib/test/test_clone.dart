import 'dart:io';

import 'package:test/test.dart';
import '../gitkebab.dart' as gitkebab;


class SessionStateChangeHistory {
  List<Map<String, String>> changes = [];

  void reset() {
    changes = [];
  }
}

var stateHistory = SessionStateChangeHistory();
gitkebab.SessionState lastSessionState = gitkebab.SessionState(0);

void stateChangedCallback(gitkebab.Session session) {
  var diff = lastSessionState.diff(session.state);
  stateHistory.changes.add(diff);
  lastSessionState = session.state;
}


void main() {
  var libraryPath = Directory.current.path + "/gitkebab-linux-debug/lib/libgitkebab.so";
  gitkebab.GitKebab.load(libraryPath);

  var fixtures = "${Directory.current.path}/../test/fixtures";
  var testStagingDir = Directory("${Directory.current.path}/test-staging");
  if (testStagingDir.existsSync()) {
    testStagingDir.deleteSync(recursive: true);
  }
  print("DBG creating [${testStagingDir.path}]");
  testStagingDir.createSync();

  test('Clone Simple', () {
    stateHistory.reset();
    String localPath = "${testStagingDir.path}/clone-test-1";
    var session = gitkebab.Session("$fixtures/simple-repo1.git", gitkebab.RepositorySourceUrlType.FILESYSTEM, localPath, "");
    session.onStateChanged = stateChangedCallback;
    session.clone();
    expect(session.lastResultCode(), equals(0));
    expect(stateHistory.changes.length, equals(2));
    expect(stateHistory.changes[0], equals({"cloneInProgress": "on"}));
    expect(stateHistory.changes[1], equals({"cloneInProgress": "off", "localCheckoutExists":"on"}));
    expect(session.state.localCheckoutExists, equals(true));
    expect(session.state.cloneInProgress, equals(false));
    print("DBG path [$localPath]");
    expect(Directory(localPath).existsSync(), equals(true));
    expect(File("$localPath/file1").existsSync(), equals(true));
    expect(Directory("$localPath/folder1").existsSync(), equals(true));
    expect(File("$localPath/folder1/file3").existsSync(), equals(true));
  });

  test('Clone - bad source path', () {
    stateHistory.reset();
    String localPath = "${testStagingDir.path}/clone-test-1";
    var session = gitkebab.Session("$fixtures/tmp/non-existent-source-path", gitkebab.RepositorySourceUrlType.FILESYSTEM, localPath, "");
    session.onStateChanged = stateChangedCallback;
    session.clone();
    expect(session.lastResultCode(), equals(gitkebab.ResultCode.ERROR_CLONE_INEXISTENT_SOURCE_PATH));
    expect(session.state.localCheckoutExists, equals(false));
    expect(stateHistory.changes.length, equals(0));
  });

  test('Clone - empty dest path', () {
    stateHistory.reset();
    String localPath = "";
    var session = gitkebab.Session("$fixtures/simple-repo1.git", gitkebab.RepositorySourceUrlType.FILESYSTEM, localPath, "");
    session.onStateChanged = stateChangedCallback;
    session.clone();
    expect(session.lastResultCode(), equals(gitkebab.ResultCode.ERROR_CLONE_INVALID_DESTINATION_PATH));
    expect(session.state.localCheckoutExists, equals(false));
    expect(stateHistory.changes.length, equals(0));
  });
}
