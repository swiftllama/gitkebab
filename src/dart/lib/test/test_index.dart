import 'dart:io';

import 'package:test/test.dart';
import '../gitkebab.dart' as gitkebab;

import 'common.dart';


void main() {
  initGitkebab();
  recreateTestStagingDirectory();

  test('Clone Simple', () {
    stateHistory.reset();
    String localPath = "${testStagingPath()}/clone-test-1";
    var session = gitkebab.Session("${testFixturesPath()}/simple-repo1.git", gitkebab.RepositorySourceUrlType.FILESYSTEM, localPath, "");
    session.onStateChanged = stateChangedCallbackWithHistory;
    session.clone();
    expect(session.lastResultCode(), equals(0));

    File("$localPath/file1").copySync("$localPath/new-file1");
    File("$localPath/file1").copySync("$localPath/new-file2");
    File("$localPath/file1").copySync("$localPath/ignored-file1");
    File("$localPath/file2").deleteSync();

    session.queryStatus();
    expect(session.lastResultCode(), equals(0));
    expect(session.status.entries.length, equals(3));
    expect(session.status.entries[0].path, equals("file2"));
    expect(session.status.entries[0].status, gitkebab.FileStatus.worktreeDeleted);
    expect(session.status.entries[1].path, equals("new-file1"));
    expect(session.status.entries[1].status, gitkebab.FileStatus.worktreeNew);
    expect(session.status.entries[2].path, equals("new-file2"));
    expect(session.status.entries[2].status, gitkebab.FileStatus.worktreeNew);

    expect(stateHistory.changes.length, equals(3));
    expect(stateHistory.changes.last, equals({"hasChangesToCommit":"on"}));
    expect(session.state.hasChangesToCommit, equals(true));
    expect(session.state.localCheckoutExists, equals(true));
  });
}