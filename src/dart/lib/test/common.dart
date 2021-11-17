import 'dart:io';

import 'package:test/test.dart';
import '../gitkebab.dart';

void initGitkebab() {
  var libraryPath = Directory.current.path + "/gitkebab-linux-debug/lib/libgitkebab.so";
  GitKebab.load(libraryPath);
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

int diff(String sourcePath, String destPath) {
  ProcessResult res = Process.runSync("diff", ["-u", sourcePath, destPath]);
  //print("Files [$sourcePath] and [$destPath] differ: ${res.stdout} ${res.stderr}";
  return res.exitCode;
}

void copySourceRepoSimpleRepo1DotGit() {
  deleteDirIfExists(simpleRepo1DotGitSourcePath());
  copyDirectory("${testFixturesPath()}/simple-repo1.git", simpleRepo1DotGitSourcePath());
}

class SessionStateChangeHistory {
  List<SessionStateDiff> changes = [];

  void reset() {
    changes = [];
  }
}

var stateHistory = SessionStateChangeHistory();
void stateChangedCallbackWithHistory(SessionStateUpdateEvent stateUpdate) {
  if (!stateUpdate.diff.isEmptyDiff()) {
     stateHistory.changes.add(stateUpdate.diff);
  }
}

List<Session> createConflictingReposAAndBWithExtendedConflicts() {
  var session1 =  Session(simpleRepo1DotGitSourcePath(), RepositorySourceUrlType.FILESYSTEM, simpleRepoAPath(), "");
  var session2 =  Session(simpleRepo1DotGitSourcePath(), RepositorySourceUrlType.FILESYSTEM, simpleRepoBPath(), "");

  session1.initialize();
  session1.clone();

  session2.initialize();
  session2.clone();

  ////
  // Modify repo A, commit and push

  // modify file1 (should conflict with delete)
  File("${testFixturesPath()}/simple-repo1-modifications/file1-modified").copySync("${simpleRepoAPath()}/file1");

  // delete file2 (should conflict should conflict with modification)
  File("${simpleRepoAPath()}/file2").deleteSync();

  // modify file3 in incompatible ways (should conflict with incompatible edit)
  File("${testFixturesPath()}/simple-repo1-modifications/file3-mod-incompatible-a").copySync("${simpleRepoAPath()}/file3");

  // modify file4 in incompatible ways (should conflict with incompatible edit)
  File("${testFixturesPath()}/simple-repo1-modifications/file4-mod-incompatible-a").copySync("${simpleRepoAPath()}/file4");

  // modify file5 (should conflict with directory of same name)
  File("${testFixturesPath()}/simple-repo1-modifications/file5-mod-compatible-a").copySync("${simpleRepoAPath()}/file5");

  // create binary file 6 (should conflict with new text file)
  File("${testFixturesPath()}/simple-repo1-modifications/green.png").copySync("${simpleRepoAPath()}/file6");

  // create same new file 7 (should NOT conflict with identical file)
  File("${testFixturesPath()}/simple-repo1-modifications/file1-modified").copySync("${simpleRepoAPath()}/file7");

  // commit and push
  session1.addAll("*");
  session1.commit("commit changes in repo A");
  session1.push(session1.repositorySpec.remoteName);

  ////
  // Modify repo B, commit
  // delete file1 (should conflict with modification)
  File("${simpleRepoBPath()}/file1").deleteSync();

  // modify file2 (should conflict with delete)
  File("${testFixturesPath()}/simple-repo1-modifications/file1-modified").copySync("${simpleRepoBPath()}/file2");

  // modify file3 (should conflict with incompatible edit)
  File("${testFixturesPath()}/simple-repo1-modifications/file3-mod-incompatible-b").copySync("${simpleRepoBPath()}/file3");

  // modify file4 (should conflict with incompatible edit)
  File("${testFixturesPath()}/simple-repo1-modifications/file4-mod-incompatible-b").copySync("${simpleRepoBPath()}/file4");

  // Delete file5 and create a directory in its place (should conflict with edit)
  File("${simpleRepoBPath()}/file5").deleteSync();
  Directory("${simpleRepoBPath()}/file5").createSync();

  // create text file 6 (should conflict with new binary file)
  File("${testFixturesPath()}/simple-repo1-modifications/file1-modified").copySync("${simpleRepoBPath()}/file6");

  // create same new file 7 (should NOT conflict with identical file)
  File("${testFixturesPath()}/simple-repo1-modifications/file1-modified").copySync("${simpleRepoBPath()}/file7");

  // commit but don't push
  session2.addAll("*");
  session2.commit("commit changes in repo B");

  ////
  // RepoB fetch
  session2.fetch(session2.repositorySpec.remoteName);

  return [session1, session2];
}
