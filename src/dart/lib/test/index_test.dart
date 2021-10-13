import 'dart:io';

import 'package:ffigen_test/src/errors.dart';
import 'package:test/test.dart';
import '../gitkebab.dart' as gitkebab;

import 'common.dart';


void main() {
  initGitkebab();
  recreateTestStagingDirectory();
  String cloneTest1 = "${testStagingPath()}/clone-test-1";
  setUp(() { deleteDirIfExists(cloneTest1); });

  test('Commit - no changes', () {
    stateHistory.reset();

    var session = gitkebab.Session("${testFixturesPath()}/simple-repo1.git", gitkebab.RepositorySourceUrlType.FILESYSTEM, cloneTest1, "");
    session.onStateChanged = stateChangedCallbackWithHistory;
    session.initialize();
    session.clone();
    expect(session.lastResultCode(), equals(0));

    session.commit("commit from dart");
    expect(session.lastResultCode(), equals(0));
  });


  test('Index - add all', () {
    stateHistory.reset();
    var session = gitkebab.Session("${testFixturesPath()}/simple-repo1.git", gitkebab.RepositorySourceUrlType.FILESYSTEM, cloneTest1, "");
    session.onStateChanged = stateChangedCallbackWithHistory;
    session.initialize();
    session.clone();
    expect(session.lastResultCode(), equals(0));

    File("$cloneTest1/file1").copySync("$cloneTest1/new-file1");
    File("$cloneTest1/file1").copySync("$cloneTest1/new-file2");
    File("$cloneTest1/file1").copySync("$cloneTest1/ignored-file1");
    File("${testFixturesPath()}/simple-repo1-modifications/file1-modified").copySync("$cloneTest1/file1");
    File("$cloneTest1/file2").deleteSync();

    session.queryStatus();
    expect(session.lastResultCode(), equals(0));

    session.addAll("*");
    session.queryStatus();
    expect(session.lastResultCode(), equals(0));

    expect(session.status.entries.length, equals(4));
    expect(session.status.entries[0].path, equals("file1"));
    expect(session.status.entries[0].status, gitkebab.FileStatus.indexModified);
    expect(session.status.entries[1].path, equals("file2"));
    expect(session.status.entries[1].status, gitkebab.FileStatus.indexDeleted);
    expect(session.status.entries[2].path, equals("new-file1"));
    expect(session.status.entries[2].status, gitkebab.FileStatus.indexNew);
    expect(session.status.entries[3].path, equals("new-file2"));
    expect(session.status.entries[3].status, gitkebab.FileStatus.indexNew);

    expect(session.state.hasChangesToCommit, equals(true));
    expect(session.state.localCheckoutExists, equals(true));
  });

  test('Index - add non-existent path', () {
    stateHistory.reset();
    var session = gitkebab.Session("${testFixturesPath()}/simple-repo1.git", gitkebab.RepositorySourceUrlType.FILESYSTEM, cloneTest1, "");
    session.onStateChanged = stateChangedCallbackWithHistory;
    session.initialize();
    session.clone();
    expect(session.lastResultCode(), equals(0));

    expect(() { session.addPath("non-existent-file1"); },
        throwsA(isA<gitkebab.GitKebabException>().having(
                (error) => error.code, 'code', gitkebab.ResultCode.ERROR_NOT_FOUND)));
  });
}
