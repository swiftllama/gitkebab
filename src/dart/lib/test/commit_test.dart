import 'dart:io';

import 'package:test/test.dart';
import '../gitkebab.dart' as gitkebab;

import 'common.dart';


void main() {
  initGitkebab();
  recreateTestStagingDirectory();
  String cloneTest1 = "${testStagingPath()}/clone-test-1";
  setUp(() { deleteDirIfExists(cloneTest1); });

  setUp(() {
    if (Directory(cloneTest1).existsSync()) {
      Directory(cloneTest1).deleteSync(recursive: true);
    }
  });

  test('Commit - no changes', () {
    stateHistory.reset();

    var session = gitkebab.Session("${testFixturesPath()}/simple-repo1.git", gitkebab.RepositorySourceUrlType.FILESYSTEM, cloneTest1, "");
    session.onStateChanged = stateChangedCallbackWithHistory;
    session.clone();
    expect(session.lastResultCode(), equals(0));

    String commitId = session.commit("commit from dart");
    expect(session.lastResultCode(), equals(0));
    expect(commitId, isNot(""));
  });

  test('Commit - new file', () {
    stateHistory.reset();

    var session = gitkebab.Session("${testFixturesPath()}/simple-repo1.git", gitkebab.RepositorySourceUrlType.FILESYSTEM, cloneTest1, "");
    session.onStateChanged = stateChangedCallbackWithHistory;
    session.clone();
    expect(session.lastResultCode(), equals(0));

    File("$cloneTest1/file1").copySync("$cloneTest1/new-file1");

    session.queryStatus();
    expect(session.lastResultCode(), equals(0));
    expect(session.status.entries.length, equals(1));
    expect(session.state.hasChangesToCommit, equals(true));

    expect(session.status.entries.length, equals(1));
    expect(session.status.entries[0].path, equals("new-file1"));
    expect(session.status.entries[0].status, gitkebab.FileStatus.worktreeNew);

    session.addAll("*");
    session.commit("commit new file");

    session.queryStatus();
    expect(session.lastResultCode(), equals(0));
    expect(session.status.entries.length, equals(0));
    expect(session.state.hasChangesToCommit, equals(false));
  });
}