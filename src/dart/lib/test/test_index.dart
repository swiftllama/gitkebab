import 'dart:io';

import 'package:test/test.dart';
import '../gitkebab.dart' as gitkebab;

import 'common.dart';


void main() {
  initGitkebab();
  recreateTestStagingDirectory();

  test('Clone Simple', () {
    expect(1, equals(2));
    /*
    String localPath = "${testStagingPath()}/clone-test-1";
    var session = gitkebab.Session("${testFixturesPath()}/simple-repo1.git",
        gitkebab.RepositorySourceUrlType.FILESYSTEM, localPath, "");
    session.clone();
    expect(session.lastResultCode(), equals(0));*/
  });
}