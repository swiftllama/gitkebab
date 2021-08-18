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

String simpleRepo1DotGitSourcePath() {
  return "${testStagingPath()}/simple-repo1.git";
}

String simpleRepoAPath() {
  return "${testStagingPath()}/simple-repo-A";
}

String simpleRepoBPath() {
  return "${testStagingPath()}/simple-repo-B";
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

void deleteDirIfExists(String path) {
  if (Directory(path).existsSync()) {
    Directory(path).deleteSync(recursive: true);
  }
}

void copyDirectory(String sourcePath, String destPath) {
  ProcessResult res = Process.runSync("cp", ["-PR", sourcePath, destPath]);
  if (res.exitCode != 0) {
    throw "Error copying path [$sourcePath] to [$destPath]: ${res.stdout} ${res.stderr}";
  }
}

void copySourceRepoSimpleRepo1DotGit() {
  deleteDirIfExists(simpleRepo1DotGitSourcePath());
  copyDirectory("${testFixturesPath()}/simple-repo1.git", simpleRepo1DotGitSourcePath());
}

class SessionStateChangeHistory {
  List<Map<String, String>> changes = [];

  void reset() {
    changes = [];
  }
}


var stateHistory = SessionStateChangeHistory();
gitkebab.SessionState lastSessionState = gitkebab.SessionState(0);

void stateChangedCallbackWithHistory(gitkebab.Session session) {
  var diff = lastSessionState.diff(session.state);
  stateHistory.changes.add(diff);
  lastSessionState = session.state;
}