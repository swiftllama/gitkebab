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

    expect(session2.state.hasChangesToMerge, equals(
        true)); // Fetch in Repo B should have brought in changes to merge
    expect(session2.state.hasConflicts,
        equals(false)); //don't know about conflicts until we try to merge

    stateHistory.reset();
    session2.mergeIntoHead(); // merge call succeeds despite the merge not finishing
    expect(stateHistory.changes[0]["hasChangesToMerge"], equals("on")); // first change will "artificially" have a "localCheckoutExists" entry because reset after clone
    expect(stateHistory.changes[0]["mergeInProgress"], equals("on"));
    expect(stateHistory.changes[1], equals({"hasConflicts":"on"}));
    expect(stateHistory.changes[2], equals({"mergeFinalizationPending":"on", "mergeInProgress":"off"}));

    expect(session2.mergeConflictSummary.conflicts.length, equals(6));
  });
}
