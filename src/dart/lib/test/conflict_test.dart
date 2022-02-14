import 'dart:io';

import 'package:test/test.dart';
import '../gitkebab.dart';

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

  test('Conflicts - various types', () {
    stateHistory.reset();
    List<Session> sessions = createConflictingReposAAndBWithExtendedConflicts();
    Session session1 = sessions.first;
    Session session2 = sessions.last;
    session2.onStateChanged = stateChangedCallbackWithHistory;

    expect(session2.state.hasChangesToMerge, equals(true)); // Fetch in Repo B should have brought in changes to merge
    expect(session2.state.hasConflicts, equals(false)); //don't know about conflicts until we try to merge

    stateHistory.reset();
    session2.mergeIntoHead(); // merge call succeeds despite the merge not finishing
    expect(stateHistory.changes[0], equals(SessionStateDiff(mergeInProgress: BooleanChange.TurnedOn)));
    expect(stateHistory.changes[1], equals(SessionStateDiff(hasConflicts: BooleanChange.TurnedOn)));
    expect(stateHistory.changes[2], equals(SessionStateDiff(mergeInProgress: BooleanChange.TurnedOff, mergeFinalizationPending: BooleanChange.TurnedOn)));

    expect(session2.mergeConflictSummary.conflicts.length, equals(6));
    expect(session2.mergeConflictSummary.conflicts[0].path, equals("file1"));
    expect(session2.mergeConflictSummary.conflicts[0].conflictType, equals(MergeConflictType.localDeleteRemoteEdit));
    expect(session2.mergeConflictSummary.conflicts[1].path, equals("file2"));
    expect(session2.mergeConflictSummary.conflicts[1].conflictType, equals(MergeConflictType.localEditRemoteDelete));
    expect(session2.mergeConflictSummary.conflicts[2].path, equals("file3"));
    expect(session2.mergeConflictSummary.conflicts[2].conflictType, equals(MergeConflictType.incompatibleTwoSidedEdit));
    expect(session2.mergeConflictSummary.conflicts[3].path, equals("file4"));
    expect(session2.mergeConflictSummary.conflicts[3].conflictType, equals(MergeConflictType.incompatibleTwoSidedEdit));
    expect(session2.mergeConflictSummary.conflicts[4].path, equals("file5"));
    expect(session2.mergeConflictSummary.conflicts[4].conflictType, equals(MergeConflictType.localDeleteRemoteEdit));
    expect(session2.mergeConflictSummary.conflicts[5].path, equals("file6"));
    expect(session2.mergeConflictSummary.conflicts[5].conflictType, equals(MergeConflictType.incompatibleTwoSidedCreate));

  });


  test('Conflicts - local delete remote edit file 1, accept local delete', () {
    stateHistory.reset();
    List<Session> sessions = createConflictingReposAAndBWithExtendedConflicts();
    Session session1 = sessions.first;
    Session session2 = sessions.last;
    session2.onStateChanged = stateChangedCallbackWithHistory;

    expect(session2.state.hasChangesToMerge, equals(
        true)); // Fetch in Repo B should have brought in changes to merge
    expect(session2.state.hasConflicts,
        equals(false)); //don't know about conflicts until we try to merge

    session2.mergeIntoHead(); // merge call succeeds despite the merge not finishing

    // file1 should have a local-delete-remote-edit type conflict
    expect(session2.mergeConflictSummary.conflicts.length, equals(6));
    expect(session2.mergeConflictSummary.conflicts[0].path, equals("file1"));
    expect(session2.mergeConflictSummary.conflicts[0].conflictType, equals(MergeConflictType.localDeleteRemoteEdit));

    // write "theirs" version onto disk at new location
    session2.writeBlobContents(session2.mergeConflictSummary.conflicts[0].theirsBlobId, "new_file1");

    // Verify that the file we "preserved" is the same as our edit
    expect(diff("${session1.repositorySpec.localPath}/file1", "${session2.repositorySpec.localPath}/new_file1"), equals(0));

    session2.conflictResolveAcceptRemoteDelete("file1");
    session2.mergeConflictsQuery();

    expect(session2.mergeConflictSummary.conflicts.length, equals(5));
    expect(session2.mergeConflictSummary.conflicts[0].path, equals("file2"));

  });

  test('Conflicts - local edit remote delete file 2, accept remote delete', ()
  {
    stateHistory.reset();
    List<Session> sessions = createConflictingReposAAndBWithExtendedConflicts();
    Session session1 = sessions.first;
    Session session2 = sessions.last;
    session2.onStateChanged = stateChangedCallbackWithHistory;

    expect(session2.state.hasChangesToMerge, equals(
        true)); // Fetch in Repo B should have brought in changes to merge
    expect(session2.state.hasConflicts,
        equals(false)); //don't know about conflicts until we try to merge

    session2.mergeIntoHead(); // merge call succeeds despite the merge not finishing

    // file2 should have a local-edit-remote-delete type conflict
    expect(session2.mergeConflictSummary.conflicts.length, equals(6));
    expect(session2.mergeConflictSummary.conflicts[1].path, equals("file2"));
    expect(session2.mergeConflictSummary.conflicts[1].conflictType, equals(MergeConflictType.localEditRemoteDelete));

    // write "ours" version onto disk at new location
    session2.writeBlobContents(session2.mergeConflictSummary.conflicts[1].oursBlobId, "new_file2");

    // Verify that the file we "preserved" is the same as our edit
    expect(diff("${session2.repositorySpec.localPath}/file2", "${session2.repositorySpec.localPath}/new_file2"), equals(0));

    session2.conflictResolveAcceptLocalDelete("file2");
    session2.mergeConflictsQuery();
    expect(session2.mergeConflictSummary.conflicts.length, equals(5));
    expect(session2.mergeConflictSummary.conflicts[1].path, equals("file3"));

  });

  test('Conflicts - incompatible two sided edit', ()
  {
    stateHistory.reset();
    List<Session> sessions = createConflictingReposAAndBWithExtendedConflicts();
    Session session1 = sessions.first;
    Session session2 = sessions.last;
    session2.onStateChanged = stateChangedCallbackWithHistory;

    expect(session2.state.hasChangesToMerge, equals(
        true)); // Fetch in Repo B should have brought in changes to merge
    expect(session2.state.hasConflicts,
        equals(false)); //don't know about conflicts until we try to merge

    session2.mergeIntoHead(); // merge call succeeds despite the merge not finishing

    // file3 should have a twosided-incompatible-edit type conflict
    expect(session2.mergeConflictSummary.conflicts.length, equals(6));
    expect(session2.mergeConflictSummary.conflicts[2].path, equals("file3"));
    expect(session2.mergeConflictSummary.conflicts[2].conflictType, equals(MergeConflictType.incompatibleTwoSidedEdit));

    // Similarity should be high, i.e. between 80 and 100 (same file's
    // been modified in only a few places, albeit incompatibly)
    expect(
        session2.compareBlobs(session2.mergeConflictSummary.conflicts[2].oursBlobId,
            session2.mergeConflictSummary.conflicts[2].theirsBlobId),
        inInclusiveRange(80, 100)
    );

    String mergedBuffer = session2.mergedBufferWithConflictMarkers(session2.mergeConflictSummary.conflicts[2]);
    expect(mergedBuffer, contains("<<<<<<< file3"));
    expect(mergedBuffer, contains("||||||| file3"));
    expect(mergedBuffer, contains(">>>>>>> file3"));

    session2.conflictResolveFromBuffer("file3", "hello");

    session2.mergeConflictsQuery();
    expect(session2.mergeConflictSummary.conflicts.length, equals(5));
    expect(session2.mergeConflictSummary.conflicts[2].path, equals("file4"));

  });

  test('Conflicts - incompatible two sided create', ()
  {
    stateHistory.reset();
    List<Session> sessions = createConflictingReposAAndBWithExtendedConflicts();
    Session session1 = sessions.first;
    Session session2 = sessions.last;
    session2.onStateChanged = stateChangedCallbackWithHistory;

    session2.mergeIntoHead(); // merge call succeeds despite the merge not finishing

    // file6 should have a twosided-incompatible-edit type conflict
    expect(session2.mergeConflictSummary.conflicts.length, equals(6));
    expect(session2.mergeConflictSummary.conflicts[5].path, equals("file6"));
    expect(session2.mergeConflictSummary.conflicts[5].conflictType, equals(MergeConflictType.incompatibleTwoSidedCreate));

    // Similarity should be low, i.e. between 0 and 20 (these are
    // two completely different files)
    expect(
        session2.compareBlobs(session2.mergeConflictSummary.conflicts[5].oursBlobId,
            session2.mergeConflictSummary.conflicts[5].theirsBlobId),
        inInclusiveRange(0, 20)
    );

    // Resolve by accepting theirs
    session2.conflictResolveAcceptExisting("file6", ConflictResolution.theirs);

    session2.mergeConflictsQuery();
    expect(session2.mergeConflictSummary.conflicts.length, equals(5));
    expect(session2.mergeConflictSummary.conflicts[4].path, equals("file5"));

  });


  test('Conflicts - partial resolution causes failed merge', ()
  {
    stateHistory.reset();
    List<Session> sessions = createConflictingReposAAndBWithExtendedConflicts();
    Session session2 = sessions.last;
    session2.onStateChanged = stateChangedCallbackWithHistory;

    // Should have changes to merge
    expect(session2.state.hasChangesToMerge, equals(true));
    expect(session2.state.hasConflicts, equals(false)); // no conflicts until we try to merge

    // Try to merge - succeeds despite the merge not finishing
    stateHistory.reset();
    session2.mergeIntoHead();
    expect(session2.state.hasConflicts, equals(true));
    expect(session2.state.hasChangesToMerge, equals(true));
    expect(session2.state.mergeFinalizationPending, equals(true));
    expect(session2.state.mergeInProgress, equals(false));
    expect(stateHistory.changes, equals([
      SessionStateDiff(mergeInProgress:BooleanChange.TurnedOn),
      SessionStateDiff(hasConflicts: BooleanChange.TurnedOn),
      SessionStateDiff(mergeFinalizationPending: BooleanChange.TurnedOn, mergeInProgress: BooleanChange.TurnedOff)
    ]));

    // We should now have 6 conflicts of various types
    expect(session2.mergeConflictSummary.conflicts.length, equals(6));

    // Resolve two out of the six
    session2.conflictResolveAcceptRemoteDelete("file1");
    session2.conflictResolveAcceptLocalDelete("file2");

    // Query again, we should have four left
    session2.mergeConflictsQuery();
    expect(session2.mergeConflictSummary.conflicts.length, equals(4));

    // Try to finalize the merge, it should fail
    expect(() { session2.mergeIntoHeadFinalize(); }, throwsA(isA<GitKebabException>().having((exc) => exc.code, 'code', equals(ResultCode.ERROR_MERGE_HAS_CONFLICTS))));
    expect(session2.state.hasConflicts, equals(true)); // merge no longer in progress, no conflicts currently
    expect(session2.state.hasChangesToMerge, equals(true));
    expect(session2.state.mergeFinalizationPending, equals(true));
    expect(session2.state.mergeInProgress, equals(false));
    expect(stateHistory.changes[stateHistory.changes.length - 2], equals(SessionStateDiff(mergeInProgress: BooleanChange.TurnedOn)));
    expect(stateHistory.changes[stateHistory.changes.length - 1], equals(SessionStateDiff(mergeInProgress: BooleanChange.TurnedOff)));

    // Abort, it should clean things up
    session2.mergeAbort();
    expect(session2.state.hasConflicts, equals(false)); // merge no longer in progress, no conflicts currently
    expect(session2.state.hasChangesToMerge, equals(true));
    expect(session2.state.mergeFinalizationPending, equals(false));
    expect(session2.state.mergeInProgress, equals(false));
    expect(stateHistory.changes.last, equals(SessionStateDiff(hasConflicts: BooleanChange.TurnedOff, mergeFinalizationPending: BooleanChange.TurnedOff)));

  });

  test('Conflicts - resolution and merge', ()
  {
    stateHistory.reset();
    List<Session> sessions = createConflictingReposAAndBWithExtendedConflicts();
    Session session2 = sessions.last;
    session2.onStateChanged = stateChangedCallbackWithHistory;

    // Should have changes to merge
    expect(session2.state.hasChangesToMerge, equals(true));
    expect(session2.state.hasConflicts, equals(false)); // no conflicts until we try to merge

    // Try to merge - succeeds despite the merge not finishing
    stateHistory.reset();
    session2.mergeIntoHead();
    expect(session2.state.hasConflicts, equals(true));
    expect(session2.state.hasChangesToMerge, equals(true));
    expect(session2.state.mergeFinalizationPending, equals(true));
    expect(session2.state.mergeInProgress, equals(false));
    expect(stateHistory.changes, equals([
      SessionStateDiff(mergeInProgress:BooleanChange.TurnedOn),
      SessionStateDiff(hasConflicts: BooleanChange.TurnedOn),
      SessionStateDiff(mergeFinalizationPending: BooleanChange.TurnedOn, mergeInProgress: BooleanChange.TurnedOff)
    ]));

    // We should now have 6 conflicts of various types
    expect(session2.mergeConflictSummary.conflicts.length, equals(6));

    // Resolve all six conflicts
    session2.conflictResolveAcceptRemoteDelete("file1");
    session2.conflictResolveAcceptLocalDelete("file2");
    session2.conflictResolveFromBuffer("file3", "hello3");
    session2.conflictResolveFromBuffer("file4", "hello4");
    session2.conflictResolveAcceptLocalDelete("file5");
    session2.conflictResolveAcceptExisting("file6", ConflictResolution.theirs);

    // We should now have 0 conflicts
    session2.mergeConflictsQuery();
    expect(session2.mergeConflictSummary.conflicts.length, equals(0));
    expect(stateHistory.changes.last, equals(SessionStateDiff(hasConflicts: BooleanChange.TurnedOff)));
    expect(session2.state.hasConflicts, equals(false));

    // Finalize the merge
    session2.mergeIntoHeadFinalize();
    expect(stateHistory.changes[stateHistory.changes.length - 2], equals(SessionStateDiff(mergeInProgress: BooleanChange.TurnedOn)));
    expect(stateHistory.changes[stateHistory.changes.length - 1], equals(SessionStateDiff(mergeInProgress: BooleanChange.TurnedOff, mergeFinalizationPending: BooleanChange.TurnedOff, hasChangesToMerge: BooleanChange.TurnedOff)));
    expect(session2.state.hasConflicts, equals(false));
    expect(session2.state.hasChangesToMerge, equals(false));
    expect(session2.state.mergeFinalizationPending, equals(false));
    expect(session2.state.mergeInProgress, equals(false));

    // Verify file3/file4 content after merge
    expect(File("${session2.repositorySpec.localPath}/file3").readAsStringSync(), equals("hello3"));
    expect(File("${session2.repositorySpec.localPath}/file4").readAsStringSync(), equals("hello4"));

  });

  test('Conflicts - status query during merge respects merge conflict data', () {
    /// Context:
    ///    Calling gk_status_summary_query while a merge is pending resets
    ///    conflict summary data (see https://github.com/projectjudo/gitkebab/issues/2)
    stateHistory.reset();
    List<Session> sessions = createConflictingReposAAndBWithExtendedConflicts();
    Session session1 = sessions.first;
    Session session2 = sessions.last;
    session2.onStateChanged = stateChangedCallbackWithHistory;

    expect(session2.state.hasChangesToMerge, equals(
        true)); // Fetch in Repo B should have brought in changes to merge
    expect(session2.state.hasConflicts,
        equals(false)); //don't know about conflicts until we try to merge

    session2.mergeIntoHead(); // merge call succeeds despite the merge not finishing

    // file1 should have a local-delete-remote-edit type conflict
    expect(session2.mergeConflictSummary.conflicts.length, equals(6));
    expect(session2.mergeConflictSummary.conflicts[0].path, equals("file1"));
    expect(session2.mergeConflictSummary.conflicts[0].conflictType, equals(MergeConflictType.localDeleteRemoteEdit));

    session2.conflictResolveAcceptRemoteDelete("file1");
    session2.mergeConflictsQuery();

    expect(session2.mergeConflictSummary.conflicts.length, equals(5));
    expect(session2.mergeConflictSummary.conflicts[0].path, equals("file2"));
    expect(session2.state.hasConflicts, equals(true));

    session2.queryStatus();

    expect(session2.mergeConflictSummary.conflicts.length, equals(5));
    expect(session2.mergeConflictSummary.conflicts[0].path, equals("file2"));
    expect(session2.state.hasConflicts, equals(true));

  });


  test('Conflicts - local delete, remote edit file 1, accept remote edit', () {
    stateHistory.reset();
    List<Session> sessions = createConflictingReposAAndBWithExtendedConflicts();
    Session session1 = sessions.first;
    Session session2 = sessions.last;
    session2.onStateChanged = stateChangedCallbackWithHistory;

    expect(session2.state.hasChangesToMerge, equals(
        true)); // Fetch in Repo B should have brought in changes to merge
    expect(session2.state.hasConflicts,
        equals(false)); //don't know about conflicts until we try to merge

    session2.mergeIntoHead(); // merge call succeeds despite the merge not finishing

    // file1 should have a local-delete-remote-edit type conflict
    expect(session2.mergeConflictSummary.conflicts.length, equals(6));
    expect(session2.mergeConflictSummary.conflicts[0].path, equals("file1"));
    expect(session2.mergeConflictSummary.conflicts[0].conflictType, equals(MergeConflictType.localDeleteRemoteEdit));

    session2.conflictResolveRestoreTheirsPathFromBlobId("file1", session2.mergeConflictSummary.conflicts[0].theirsBlobId);
    // Verify that the 'theirs' copy of "file1" is the same as in session1
    expect(diff("${session1.repositorySpec.localPath}/file1", "${session2.repositorySpec.localPath}/file1"), equals(0));    
    session2.mergeConflictsQuery();
    
    expect(session2.mergeConflictSummary.conflicts.length, equals(5));
    expect(session2.mergeConflictSummary.conflicts[0].path, equals("file2"));
  });


  test('Conflicts - local edit remote delete file 2, accept local edit', ()
  {
    stateHistory.reset();
    List<Session> sessions = createConflictingReposAAndBWithExtendedConflicts();
    Session session1 = sessions.first;
    Session session2 = sessions.last;
    session2.onStateChanged = stateChangedCallbackWithHistory;

    expect(session2.state.hasChangesToMerge, equals(
        true)); // Fetch in Repo B should have brought in changes to merge
    expect(session2.state.hasConflicts,
        equals(false)); //don't know about conflicts until we try to merge

    session2.mergeIntoHead(); // merge call succeeds despite the merge not finishing

    // file2 should have a local-edit-remote-delete type conflict
    expect(session2.mergeConflictSummary.conflicts.length, equals(6));
    expect(session2.mergeConflictSummary.conflicts[1].path, equals("file2"));
    expect(session2.mergeConflictSummary.conflicts[1].conflictType, equals(MergeConflictType.localEditRemoteDelete));

    session2.conflictResolveAcceptExisting("file2", ConflictResolution.ours);
    session2.mergeConflictsQuery();
    expect(session2.mergeConflictSummary.conflicts.length, equals(5));
    expect(session2.mergeConflictSummary.conflicts[1].path, equals("file3"));

  });

      
  test('Conflicts - local delete folder1 remote edit folder1/file3, accept remote edit', () {
    stateHistory.reset();

    var session1 =  Session(simpleRepo1DotGitSourcePath(), RepositorySourceUrlType.FILESYSTEM, simpleRepoAPath(), "");
    var session2 =  Session(simpleRepo1DotGitSourcePath(), RepositorySourceUrlType.FILESYSTEM, simpleRepoBPath(), "");

    session1.initialize();
    session1.clone();

    session2.initialize();
    session2.clone();

    // Modify repo A, commit and push
    File("${testFixturesPath()}/simple-repo1-modifications/file1-modified").copySync("${simpleRepoAPath()}/folder1/file3");
    session1.addAll("*");
    session1.commit("commit changes in repo A");
    session1.push(session1.repositorySpec.remoteName);
    
    ////
    // Modify repo B
    // delete folder1 (should conflict with modification)
    Directory("${simpleRepoBPath()}/folder1").deleteSync(recursive: true);

    // commit but don't push
    session2.addAll("*");
    session2.commit("commit changes in repo B");

    ////
    // RepoB fetch
    session2.fetch(session2.repositorySpec.remoteName);
    session2.mergeIntoHead();

    expect(session2.mergeConflictSummary.conflicts.length, equals(1));
    expect(session2.mergeConflictSummary.conflicts[0].path, equals("folder1/file3"));
    expect(session2.state.hasConflicts, equals(true));

    session2.conflictResolveRestoreTheirsPathFromBlobId("folder1/file3", session2.mergeConflictSummary.conflicts[0].theirsBlobId);
    // Verify that the 'theirs' copy of "folder1/file3" is the same as in session1
    expect(diff("${session1.repositorySpec.localPath}/folder1/file3", "${session2.repositorySpec.localPath}/folder1/file3"), equals(0));    

    
  });
}
