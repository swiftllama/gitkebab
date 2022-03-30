import 'dart:io';

import 'package:test/test.dart';
import '../gitkebab.dart' as gk;

import 'common.dart';

void main() {
  initGitkebab();
  recreateTestStagingDirectory();

  final createLocal1 = "${testStagingPath()}/create-local-test-1";
  final createLocal2 = "${testStagingPath()}/create-local-test-2";
  setUp(() {
      deleteDirIfExists(createLocal1);
  });

  test('Create local repository - success', () {
      final session1 = gk.Session("", gk.RepositorySourceUrlType.SSH, createLocal1, "");
      session1.initialize();
      session1.createLocal();

      final repoDirectory = Directory(createLocal1);
      expect(repoDirectory.existsSync(), true);

      final remotes = session1.remotesList();
      expect(remotes.length, equals(0));
  });

  test('Create local repository and remotes - success', () {
    final session1 = gk.Session("", gk.RepositorySourceUrlType.SSH, createLocal2, "");
    session1.initialize();
    session1.createLocal();

    final repoDirectory = Directory(createLocal2);
    expect(repoDirectory.existsSync(), true);

    var remotes = session1.remotesList();
    expect(remotes.length, equals(0));

    session1.createRemote(gk.Remote(name:'origin1', url:'ssh://example.com'));
    remotes = session1.remotesList();
    expect(remotes.length, equals(1));
    expect(remotes[0].name, equals('origin1'));
    expect(remotes[0].url, equals('ssh://example.com'));

    session1.createRemote(gk.Remote(name:'origin2', url:'ssh://example.com/origin2'));
    remotes = session1.remotesList();
    expect(remotes.length, equals(2));
    expect(remotes[0].name, equals('origin1'));
    expect(remotes[0].url, equals('ssh://example.com'));
    expect(remotes[1].name, equals('origin2'));
    expect(remotes[1].url, equals('ssh://example.com/origin2'));

    session1.deleteRemoteNamed('origin1');
    remotes = session1.remotesList();
    expect(remotes.length, equals(1));
    expect(remotes[0].name, equals('origin2'));
    expect(remotes[0].url, equals('ssh://example.com/origin2'));
  });
}
