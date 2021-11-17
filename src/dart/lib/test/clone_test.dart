import 'dart:io';

import 'package:test/test.dart';
import '../gitkebab.dart' as gk;

import 'common.dart';

void main() {
  initGitkebab();
  recreateTestStagingDirectory();

  String cloneTest1 = "${testStagingPath()}/clone-test-1";
  setUp(() { deleteDirIfExists(cloneTest1); });

  test('Initialize repository - success', () {
      String localPath = cloneTest1;
      final session1 = gk.Session("${testFixturesPath()}/simple-repo1.git", gk.RepositorySourceUrlType.FILESYSTEM, localPath, "");
      session1.initialize();
      session1.clone();

      final repoDirectory = Directory(cloneTest1);
      expect(repoDirectory.existsSync(), true);

      final session2 = gk.Session("${testFixturesPath()}/simple-repo1.git", gk.RepositorySourceUrlType.FILESYSTEM, localPath, "");
      expect(session2.state.localCheckoutExists, false);
      expect(session2.state.initialized, false);
      session2.initialize();
      expect(session2.state.localCheckoutExists, true);
  });
  
  test('Clone Simple', () {
    stateHistory.reset();
    String localPath = cloneTest1;
    var session = gk.Session("${testFixturesPath()}/simple-repo1.git", gk.RepositorySourceUrlType.FILESYSTEM, localPath, "");
    session.onStateChanged = stateChangedCallbackWithHistory;
    session.initialize();
    session.clone();


    expect(stateHistory.changes.length, equals(3));
    expect(stateHistory.changes[0].initialized.turnedOn, true);
    expect(stateHistory.changes[1].cloneInProgress.turnedOn, true);
    expect(stateHistory.changes[2], equals(gk.SessionStateDiff(cloneInProgress:gk.BooleanChange.TurnedOff, localCheckoutExists: gk.BooleanChange.TurnedOn)));
    expect(session.state.localCheckoutExists, equals(true));
    expect(session.state.cloneInProgress, equals(false));
    expect(Directory(localPath).existsSync(), equals(true));
    expect(File("$localPath/file1").existsSync(), equals(true));
    expect(Directory("$localPath/folder1").existsSync(), equals(true));
    expect(File("$localPath/folder1/file3").existsSync(), equals(true));
  });

  test('Clone - bad source path', () {
    stateHistory.reset();
    String localPath = cloneTest1;
    var session = gk.Session("${testFixturesPath()}/tmp/non-existent-source-path", gk.RepositorySourceUrlType.FILESYSTEM, localPath, "");
    session.onStateChanged = stateChangedCallbackWithHistory;
    session.initialize();
    expect( (){ session.clone();},
        throwsA(isA<gk.GitKebabException>().having(
            (error) => error.code, 'code', gk.ResultCode.ERROR_CLONE_INEXISTENT_SOURCE_PATH))
    );

    expect(session.state.localCheckoutExists, equals(false));
    expect(stateHistory.changes.length, equals(1)); // Initialize produces 1 event
  });

  test('Clone - empty dest path', () {
    stateHistory.reset();
    String localPath = "";
    var session = gk.Session("${testFixturesPath()}/simple-repo1.git", gk.RepositorySourceUrlType.FILESYSTEM, localPath, "");
    session.onStateChanged = stateChangedCallbackWithHistory;
    session.initialize();
    expect( (){ session.clone();},
        throwsA(isA<gk.GitKebabException>().having(
                (error) => error.code, 'code', gk.ResultCode.ERROR_CLONE_INVALID_DESTINATION_PATH))
    );
    expect(session.state.localCheckoutExists, equals(false));
    expect(stateHistory.changes.length, equals(1));
  });
}
