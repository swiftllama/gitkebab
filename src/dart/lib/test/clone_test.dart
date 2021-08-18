import 'dart:io';

import 'package:test/test.dart';
import '../gitkebab.dart' as gitkebab;

import 'common.dart';

void main() {
  initGitkebab();
  recreateTestStagingDirectory();

  String cloneTest1 = "${testStagingPath()}/clone-test-1";
  setUp(() { deleteDirIfExists(cloneTest1); });

  test('Clone Simple', () {
    stateHistory.reset();
    String localPath = cloneTest1;
    var session = gitkebab.Session("${testFixturesPath()}/simple-repo1.git", gitkebab.RepositorySourceUrlType.FILESYSTEM, localPath, "");
    session.onStateChanged = stateChangedCallbackWithHistory;
    session.clone();

    expect(stateHistory.changes.length, equals(2));
    expect(stateHistory.changes[0], equals({"cloneInProgress": "on"}));
    expect(stateHistory.changes[1], equals({"cloneInProgress": "off", "localCheckoutExists":"on"}));
    expect(session.state.localCheckoutExists, equals(true));
    expect(session.state.cloneInProgress, equals(false));
    expect(Directory(localPath).existsSync(), equals(true));
    expect(File("$localPath/file1").existsSync(), equals(true));
    expect(Directory("$localPath/folder1").existsSync(), equals(true));
    expect(File("$localPath/folder1/file3").existsSync(), equals(true));
  });

  test('Clone - bad source path', () {
    stateHistory.reset();
    String localPath = "${testStagingPath()}/clone-test-1";
    var session = gitkebab.Session("${testFixturesPath()}/tmp/non-existent-source-path", gitkebab.RepositorySourceUrlType.FILESYSTEM, localPath, "");
    session.onStateChanged = stateChangedCallbackWithHistory;
    expect( (){ session.clone();},
        throwsA(isA<gitkebab.GitKebabException>().having(
            (error) => error.code, 'code', gitkebab.ResultCode.ERROR_CLONE_INEXISTENT_SOURCE_PATH))
    );

    expect(session.state.localCheckoutExists, equals(false));
    expect(stateHistory.changes.length, equals(0));
  });

  test('Clone - empty dest path', () {
    stateHistory.reset();
    String localPath = "";
    var session = gitkebab.Session("${testFixturesPath()}/simple-repo1.git", gitkebab.RepositorySourceUrlType.FILESYSTEM, localPath, "");
    session.onStateChanged = stateChangedCallbackWithHistory;
    expect( (){ session.clone();},
        throwsA(isA<gitkebab.GitKebabException>().having(
                (error) => error.code, 'code', gitkebab.ResultCode.ERROR_CLONE_INVALID_DESTINATION_PATH))
    );
    expect(session.state.localCheckoutExists, equals(false));
    expect(stateHistory.changes.length, equals(0));
  });
}
