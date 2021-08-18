import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart' as ffip;

import 'gitkebab.dart';
import 'gitkebab_lib.dart' as gitkebab_lib;
import 'status.dart';

Map<String, Session> g_sessions = {};

void session_progress_callback(ffi.Pointer<ffi.Int8> session_id_ptr, ffi.Pointer<gitkebab_lib.gk_session_progress> sessionProgress) {
  if (session_id_ptr.address == 0) {
    print("WARNING: received session progress callback with NULL session_id");
    return;
  }
  var session_id = session_id_ptr.toDartString();
  if (sessionProgress.address == 0) {
    print("WARNING: received session progress callback for session [$session_id] with null progress structure");
    return;
  }

  Session? session = g_sessions[session_id];
  if (session == null) {
    print("WARNING: received session progress callback for session [$session_id] but no such session exists");
    return;
  }

  var progress = sessionProgress.ref;
  session.onProgress(session, progress);
}



void session_state_callback(ffi.Pointer<ffi.Int8> session_id_ptr, ffi.Pointer<gitkebab_lib.gk_repository> repositoryPtr) {
  if (session_id_ptr.address == 0) {
    print("WARNING: received session state changed callback with NULL session_id");
    return;
  }
  var session_id = session_id_ptr.toDartString();
  if (repositoryPtr.address == 0) {
    print("WARNING: received session state changed callback for session [$session_id] with NULL repository structure");
    return;
  }

  Session? session = g_sessions[session_id];
  if (session == null) {
    print("WARNING: received session state changed callback for session [$session_id] but no such session exists");
    return;
  }

  var repository = repositoryPtr.ref;
  session.state = SessionState(repository.state);
  session.onStateChanged(session);
}

typedef void SessionProgressCallback(Session session, gitkebab_lib.gk_session_progress progress);
typedef void SessionStateCallback(Session session);

class Session {
  ffi.Pointer<gitkebab_lib.gk_session> session_ptr = ffi.Pointer.fromAddress(0);
  String id = "<unknown>";
  SessionState state = SessionState(0);
  RepositoryStatusList status = RepositoryStatusList();

  SessionProgressCallback onProgress = (session, progress) => {};
  SessionStateCallback onStateChanged = (session) => {};

  Session(String url, int urlType, String localPath, String user) {
    session_ptr = GitKebab.lib.gk_session_new(url.toFfiPtr(), urlType, localPath.toFfiPtr(), user.toFfiPtr(), ffi.Pointer.fromFunction(session_progress_callback), ffi.Pointer.fromFunction(session_state_callback));
    if (session_ptr.address == 0) {
      throw "Error initializing session, received null pointer";
    }

    id = session_ptr.ref.id_ptr.toDartString();
    g_sessions[id] = this;
  }

  void clone() {
    GitKebab.lib.gk_clone(session_ptr);
  }

  void fetch(String remoteName) {
    GitKebab.lib.gk_fetch(session_ptr, remoteName.toFfiPtr());
  }

  void push(String remoteName) {
    GitKebab.lib.gk_push(session_ptr, remoteName.toFfiPtr());
  }

  int lastResultCode() {
    return GitKebab.lib.gk_session_last_result_code(session_ptr);
  }

  String lastResultMessage() {
    return GitKebab.lib.gk_session_last_result_message(session_ptr).cast<ffip.Utf8>().toDartString();
  }

  int addPath(String path) {
    return GitKebab.lib.gk_index_add_path(session_ptr, path.toFfiPtr());
  }

  int removePath(String path) {
    return GitKebab.lib.gk_index_remove_path(session_ptr, path.toFfiPtr());
  }

  int addAll(String pattern) {
    return GitKebab.lib.gk_index_add_all(session_ptr, pattern.toFfiPtr());
  }

  int updateAll(String pattern) {
    return GitKebab.lib.gk_index_update_all(session_ptr, pattern.toFfiPtr());
  }

  int queryStatus() {
    int rc = GitKebab.lib.gk_status_summary_query(session_ptr);
    if (rc != 0) {
      return rc;
    }
    status = RepositoryStatusList.forQueriedSession(session_ptr);
    GitKebab.lib.gk_status_summary_close(session_ptr);
    return 0;
  }
}

class SessionState {
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

  SessionState(int state):
        localCheckoutExists = stateIncludes(state, gitkebab_lib.RepositoryState.LOCAL_CHECKOUT_EXISTS),
        hasConflicts = stateIncludes(state, gitkebab_lib.RepositoryState.HAS_CONFLICTS),
        hasChangesToCommit = stateIncludes(state, gitkebab_lib.RepositoryState.HAS_CHANGES_TO_COMMIT),
        hasChangesToMerge = stateIncludes(state, gitkebab_lib.RepositoryState.HAS_CHANGES_TO_MERGE),
        cloneInProgress = stateIncludes(state, gitkebab_lib.RepositoryState.CLONE_IN_PROGRESS),
        mergeFinalizationPending = stateIncludes(state, gitkebab_lib.RepositoryState.MERGE_FINALIZATION_PENDING),
        mergePendingOnDisk = stateIncludes(state, gitkebab_lib.RepositoryState.MERGE_PENDING_ON_DISK),
        pushInProgress = stateIncludes(state, gitkebab_lib.RepositoryState.PUSH_IN_PROGRESS),
        fetchInProgress = stateIncludes(state, gitkebab_lib.RepositoryState.FETCH_IN_PROGRESS),
        mergeInProgress = stateIncludes(state, gitkebab_lib.RepositoryState.MERGE_IN_PROGRESS) {
  }

  static bool stateIncludes(int totalState, int flag) {
    return (totalState > 0) && ((totalState & flag) == flag);
  }

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
    return diffs;
  }

  String toString() {
    String str = "<SessionState ";
    str += localCheckoutExists ? "localCheckoutExists " : "";
    str += hasConflicts ? "hasConflicts " : "";
    str += hasChangesToMerge ? "hasChangesToMerge " : "";
    str += cloneInProgress ? "cloneInProgress " : "";
    str += mergeFinalizationPending ? "mergeFinalizationPending " : "";
    str += mergePendingOnDisk ? "mergePendingOnDisk " : "";
    str += pushInProgress ? "pushInProgress " : "";
    str += fetchInProgress ? "fetchInProgress " : "";
    str += mergeInProgress ? "mergeInProgress " : "";
    str += ">";
    return str;
  }
}
