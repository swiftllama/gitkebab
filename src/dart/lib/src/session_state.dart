import 'dart:ffi';
import 'package:ffigen_test/gitkebab.dart';

import 'gitkebab_lib.dart' as gitkebab_lib;

class BooleanChange {
  static const TurnedOn = const BooleanChange._(1);
  static const TurnedOff = const BooleanChange._(-1);
  static const Unchanged = const BooleanChange._(0);

  final int change;
  const BooleanChange._(this.change);

  bool get turnedOn => change == 1;
  bool get turnedOff => change == -1;
  bool get unchanged => change == 0;

  static BooleanChange from({required bool oldValue, required bool newValue}) {
    if (oldValue == newValue) return Unchanged;
    if (newValue) return TurnedOn;
    return TurnedOff;
  }
}

class SessionStateUpdateEvent {
  final SessionState newState;
  final SessionStateDiff diff;
  SessionStateUpdateEvent({required this.newState, required this.diff});
  SessionStateUpdateEvent.from({required SessionState oldState, required this.newState}): diff = SessionStateDiff.from(oldState: oldState, newState: newState);
}

class SessionStateDiff {
  final BooleanChange initialized;
  final BooleanChange localCheckoutExists;
  final BooleanChange hasConflicts;
  final BooleanChange hasChangesToCommit;
  final BooleanChange hasChangesToMerge;
  final BooleanChange cloneInProgress;
  final BooleanChange mergeFinalizationPending;
  final BooleanChange mergePendingOnDisk;
  final BooleanChange pushInProgress;
  final BooleanChange fetchInProgress;
  final BooleanChange mergeInProgress;
  final BooleanChange syncInProgress;
  final BooleanChange backgroundSyncInProgress;

  SessionStateDiff({
    this.initialized = BooleanChange.Unchanged,
    this.localCheckoutExists = BooleanChange.Unchanged,
    this.hasConflicts = BooleanChange.Unchanged,
    this.hasChangesToCommit = BooleanChange.Unchanged,
    this.hasChangesToMerge = BooleanChange.Unchanged,
    this.cloneInProgress = BooleanChange.Unchanged,
    this.mergeFinalizationPending = BooleanChange.Unchanged,
    this.mergePendingOnDisk = BooleanChange.Unchanged,
    this.pushInProgress = BooleanChange.Unchanged,
    this.fetchInProgress = BooleanChange.Unchanged,
    this.mergeInProgress = BooleanChange.Unchanged,
    this.syncInProgress = BooleanChange.Unchanged,
    this.backgroundSyncInProgress = BooleanChange.Unchanged
  });

  static SessionStateDiff from({required SessionState oldState, required SessionState newState}) {
    return SessionStateDiff(
        initialized: BooleanChange.from(oldValue: oldState.initialized, newValue: newState.initialized),
        localCheckoutExists: BooleanChange.from(oldValue:oldState.localCheckoutExists, newValue: newState.localCheckoutExists),
        hasConflicts: BooleanChange.from(oldValue:oldState.hasConflicts, newValue: newState.hasConflicts),
        hasChangesToCommit: BooleanChange.from(oldValue:oldState.hasChangesToCommit, newValue: newState.hasChangesToCommit),
        hasChangesToMerge: BooleanChange.from(oldValue:oldState.hasChangesToMerge, newValue: newState.hasChangesToMerge),
        cloneInProgress: BooleanChange.from(oldValue:oldState.cloneInProgress, newValue: newState.cloneInProgress),
        mergeFinalizationPending: BooleanChange.from(oldValue:oldState.mergeFinalizationPending, newValue: newState.mergeFinalizationPending),
        mergePendingOnDisk: BooleanChange.from(oldValue:oldState.mergePendingOnDisk, newValue: newState.mergePendingOnDisk),
        pushInProgress: BooleanChange.from(oldValue:oldState.pushInProgress, newValue: newState.pushInProgress),
        fetchInProgress: BooleanChange.from(oldValue:oldState.fetchInProgress, newValue: newState.fetchInProgress),
        mergeInProgress: BooleanChange.from(oldValue:oldState.mergeInProgress, newValue: newState.mergeInProgress),
        syncInProgress: BooleanChange.from(oldValue:oldState.syncInProgress, newValue: newState.syncInProgress),
        backgroundSyncInProgress: BooleanChange.from(oldValue:oldState.backgroundSyncInProgress, newValue: newState.backgroundSyncInProgress)
    );
  }

  List<BooleanChange> get allAttributes => [initialized, localCheckoutExists, hasConflicts, hasChangesToCommit, hasChangesToMerge, cloneInProgress, mergeFinalizationPending, mergePendingOnDisk, pushInProgress, fetchInProgress, mergeInProgress, syncInProgress, backgroundSyncInProgress];
  bool isEmptyDiff() => allAttributes.every((element) => element.unchanged);

  @override
  bool operator ==(Object other) {
    if (other is! SessionStateDiff) return false;
    return identical(initialized, other.initialized) &&
        identical(localCheckoutExists, other.localCheckoutExists) &&
        identical(hasConflicts, other.hasConflicts) &&
        identical(hasChangesToCommit, other.hasChangesToCommit) &&
        identical(hasChangesToMerge, other.hasChangesToMerge) &&
        identical(cloneInProgress, other.cloneInProgress) &&
        identical(mergeFinalizationPending, other.mergeFinalizationPending) &&
        identical(mergePendingOnDisk, other.mergePendingOnDisk) &&
        identical(pushInProgress, other.pushInProgress) &&
        identical(fetchInProgress, other.fetchInProgress) &&
        identical(mergeInProgress, other.mergeInProgress) &&
        identical(syncInProgress, other.syncInProgress) &&
        identical(backgroundSyncInProgress, other.backgroundSyncInProgress);
  }

  @override
  int get hashCode {
    int sum = 0;
    for (var index = 0; index < allAttributes.length; index += 1) sum += allAttributes[index].change*(1>>index);
    return sum;
  }

  @override
  String toString() {
    var str = '<SessionStateDiff ';
    str += '${initialized.unchanged ? '' : 'initialized:' + (initialized.turnedOn ? 'on ': 'off ')} ';
    str += '${localCheckoutExists.unchanged ? '' : 'localCheckoutExists:' + (localCheckoutExists.turnedOn ? 'on ' : 'off ')}';
    str += '${hasConflicts.unchanged ? '' : 'hasConflicts:' + (hasConflicts.turnedOn ? 'on ': 'off ')}';
    str += '${hasChangesToCommit.unchanged ? '' : 'hasChangesToCommit:' + (hasChangesToCommit.turnedOn ? 'on ': 'off ')}';
    str += '${hasChangesToMerge.unchanged ? '' : 'hasChangesToMerge:' + (hasChangesToMerge.turnedOn ? 'on ': 'off ')}';
    str += '${cloneInProgress.unchanged ? '' : 'cloneInProgress:' + (cloneInProgress.turnedOn ? 'on ': 'off ')}';
    str += '${mergeFinalizationPending.unchanged ? '' : 'mergeFinalizationPending:' + (mergeFinalizationPending.turnedOn ? 'on ': 'off ')}';
    str += '${mergePendingOnDisk.unchanged ? '' : 'mergePendingOnDisk:' + (mergePendingOnDisk.turnedOn ? 'on ': 'off ')}';
    str += '${pushInProgress.unchanged ? '' : 'pushInProgress:' + (pushInProgress.turnedOn ? 'on ': 'off ')}';
    str += '${fetchInProgress.unchanged ? '' : 'fetchInProgress:' + (fetchInProgress.turnedOn ? 'on ': 'off ')}';
    str += '${mergeInProgress.unchanged ? '' : 'mergeInProgress:' + (mergeInProgress.turnedOn ? 'on ': 'off ')}';
    str += '${syncInProgress.unchanged ? '' : 'syncInProgress:' + (syncInProgress.turnedOn ? 'on ': 'off ')}';
    str += '${backgroundSyncInProgress.unchanged ? '' : 'backgroundSyncInProgress:' + (backgroundSyncInProgress.turnedOn ? 'on ': 'off ')}';
    str += '>';
    return str;
  }
}

class SessionState {
  final bool initialized;
  final bool localCheckoutExists;
  final bool hasConflicts;
  final bool hasChangesToCommit;
  final bool hasChangesToMerge;
  final bool cloneInProgress;
  final bool mergeFinalizationPending;
  final bool mergePendingOnDisk;
  final bool pushInProgress;
  final bool fetchInProgress;
  final bool mergeInProgress;
  final bool syncInProgress;
  final bool backgroundSyncInProgress;
  final int progressPercent;
  final int counter;

  SessionState(int state, {this.counter = 0, this.progressPercent = 0}):
        initialized = stateIncludes(state, gitkebab_lib.RepositoryState.INITIALIZED),
        localCheckoutExists = stateIncludes(state, gitkebab_lib.RepositoryState.LOCAL_CHECKOUT_EXISTS),
        hasConflicts = stateIncludes(state, gitkebab_lib.RepositoryState.HAS_CONFLICTS),
        hasChangesToCommit = stateIncludes(state, gitkebab_lib.RepositoryState.HAS_CHANGES_TO_COMMIT),
        hasChangesToMerge = stateIncludes(state, gitkebab_lib.RepositoryState.HAS_CHANGES_TO_MERGE),
        cloneInProgress = stateIncludes(state, gitkebab_lib.RepositoryState.CLONE_IN_PROGRESS),
        mergeFinalizationPending = stateIncludes(state, gitkebab_lib.RepositoryState.MERGE_FINALIZATION_PENDING),
        mergePendingOnDisk = stateIncludes(state, gitkebab_lib.RepositoryState.MERGE_PENDING_ON_DISK),
        pushInProgress = stateIncludes(state, gitkebab_lib.RepositoryState.PUSH_IN_PROGRESS),
        fetchInProgress = stateIncludes(state, gitkebab_lib.RepositoryState.FETCH_IN_PROGRESS),
        mergeInProgress = stateIncludes(state, gitkebab_lib.RepositoryState.MERGE_IN_PROGRESS),
        syncInProgress = stateIncludes(state, gitkebab_lib.RepositoryState.SYNC_IN_PROGRESS),
        backgroundSyncInProgress = stateIncludes(state, gitkebab_lib.RepositoryState.BACKGROUND_SYNC_IN_PROGRESS) {
  }

  static SessionState fromSessionPointer(Pointer<gitkebab_lib.gk_session> sessionPtr) {
    final repository = sessionPtr.address == 0 ? null : sessionPtr.ref.repository.address == 0 ? null : sessionPtr.ref.repository.ref;
    final progress = sessionPtr.address == 0 ? null : sessionPtr.ref.progress.address == 0 ? null : sessionPtr.ref.progress.ref;
    return SessionState(repository?.state ?? 0, counter:repository?.state_counter ?? 0, progressPercent:progress?.percent ?? 0);
  }

  static bool stateIncludes(int totalState, int flag) {
    return (totalState > 0) && ((totalState & flag) == flag);
  }

  /*
  Map<String, String> diff(SessionState other) {
    Map<String, String> diffs = {};
    if (localCheckoutExists != other.localCheckoutExists) {
      diffs["localCheckoutExists"] = localCheckoutExists ? "off" : "on";
    }
    if (hasConflicts != other.hasConflicts) {
      diffs["hasConflicts"] = hasConflicts ? "off" : "on";
    }
    if (hasChangesToCommit != other.hasChangesToCommit) {
      diffs["hasChangesToCommit"] = hasChangesToMerge ? "off" : "on";
    }
    if (hasChangesToMerge != other.hasChangesToMerge) {
      diffs["hasChangesToMerge"] = hasChangesToMerge ? "off" : "on";
    }
    if (cloneInProgress != other.cloneInProgress) {
      diffs["cloneInProgress"] = cloneInProgress ? "off" : "on";
    }
    if (mergeFinalizationPending != other.mergeFinalizationPending) {
      diffs["mergeFinalizationPending"] = mergeFinalizationPending ? "off" : "on";
    }
    if (mergePendingOnDisk != other.mergePendingOnDisk) {
      diffs["mergePendingOnDisk"] = mergePendingOnDisk ? "off" : "on";
    }
    if (pushInProgress != other.pushInProgress) {
      diffs["pushInProgress"] = pushInProgress ? "off" : "on";
    }
    if (fetchInProgress != other.fetchInProgress) {
      diffs["fetchInProgress"] = fetchInProgress ? "off" : "on";
    }
    if (mergeInProgress != other.mergeInProgress) {
      diffs["mergeInProgress"] = mergeInProgress ? "off" : "on";
    }
    if (syncInProgress != other.syncInProgress) {
      diffs["syncInProgress"] = syncInProgress ? "off" : "on";
    }
    if (backgroundSyncInProgress != other.backgroundSyncInProgress) {
      diffs["backgroundSyncInProgress"] = backgroundSyncInProgress ? "off" : "on";
    }
    return diffs;
  }*/

  String toString() {
    String str = "<SessionState counter:$counter progres:$progressPercent% ";
    str += localCheckoutExists ? "localCheckoutExists " : "";
    str += hasConflicts ? "hasConflicts " : "";
    str += hasChangesToMerge ? "hasChangesToMerge " : "";
    str += cloneInProgress ? "cloneInProgress " : "";
    str += mergeFinalizationPending ? "mergeFinalizationPending " : "";
    str += mergePendingOnDisk ? "mergePendingOnDisk " : "";
    str += pushInProgress ? "pushInProgress " : "";
    str += fetchInProgress ? "fetchInProgress " : "";
    str += mergeInProgress ? "mergeInProgress " : "";
    str += syncInProgress ? "syncInProgress " : "";
    str += backgroundSyncInProgress ? "backbackgroundSyncInProgress " : "";
    str += ">";
    return str;
  }
}
