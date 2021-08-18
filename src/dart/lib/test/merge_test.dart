import 'dart:io';

import 'package:test/test.dart';
import '../gitkebab.dart' as gitkebab;

import 'common.dart';

void main() {
  initGitkebab();
  recreateTestStagingDirectory();

  String cloneTest1 = "${testStagingPath()}/clone-test-1";
  setUp(() {
    deleteDirIfExists(cloneTest1);
    deleteDirIfExists(simpleRepoAPath());
    deleteDirIfExists(simpleRepoBPath());
    copySourceRepoSimpleRepo1DotGit();
  });

  test('Merge - no changes', () {
    stateHistory.reset();
    String localPath = cloneTest1;
    var session = gitkebab.Session(simpleRepo1DotGitSourcePath(), gitkebab.RepositorySourceUrlType.FILESYSTEM, localPath, "");
    session.onStateChanged = stateChangedCallbackWithHistory;
    session.clone();

    String originalHead = session.resolveReference("HEAD");
    expect(originalHead, equals("9e9d6321552cf12518065d01caa50460603b8477"));

    session.fetch("origin");
    String fetchedCommit = session.resolveReference(session.repositorySpec.remoteRefName);
    String newHeadBeforeMerge = session.resolveReference("HEAD");

    session.mergeIntoHead();
    String newHeadAfterMerge = session.resolveReference("HEAD");

    expect(originalHead, equals(fetchedCommit));
    expect(originalHead, equals(newHeadBeforeMerge));
    expect(originalHead, equals(newHeadAfterMerge));
  });


  test('Merge - one commit', () {
    stateHistory.reset();
    var session1 = gitkebab.Session(simpleRepo1DotGitSourcePath(), gitkebab.RepositorySourceUrlType.FILESYSTEM, simpleRepoAPath(), "");
    var session2 = gitkebab.Session(simpleRepo1DotGitSourcePath(), gitkebab.RepositorySourceUrlType.FILESYSTEM, simpleRepoBPath(), "");
    session2.onStateChanged = stateChangedCallbackWithHistory;

    session1.clone();
    session2.clone();

    String repoAOriginalHead = session1.resolveReference("HEAD");
    String repoBOriginalHead = session2.resolveReference("HEAD");

    // Modify repo A, commit and push
    File("${testFixturesPath()}/simple-repo1-modifications/file1-modified").copySync("${simpleRepoAPath()}/file1");
    session1.addPath("file1");
    String repoANewCommit = session1.commit("change file1 in repoA");
    session1.push(session1.repositorySpec.remoteName);

    // Fetch repo B
    stateHistory.reset();
    session2.fetch(session2.repositorySpec.remoteName);
    expect(session2.state.hasChangesToMerge, equals(true));
    expect(stateHistory.changes, equals([{"fetchInProgress":"on"}, {"hasChangesToMerge":"on", "fetchInProgress":"off"}]));
    String repoBFetchedCommit = session2.resolveReference(session2.repositorySpec.remoteRefName);

    // Merge in repo B
    stateHistory.reset();
    session2.mergeIntoHead();
    String repoBNewHead = session2.resolveReference("HEAD");
    expect(session2.state.hasChangesToMerge, equals(false));
    expect(stateHistory.changes, equals([{"mergeInProgress":"on"}, {"mergeInProgress":"off", "hasChangesToMerge":"off"}]));

    // Compare
    expect(repoAOriginalHead, equals(repoBOriginalHead));
    expect(repoAOriginalHead, isNot(repoANewCommit));
    expect(repoBFetchedCommit, equals(repoANewCommit));
    expect(repoBNewHead, equals(repoANewCommit));
  });
}
