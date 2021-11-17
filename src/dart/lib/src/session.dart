import 'dart:async';
import 'dart:ffi';
import 'package:ffi/ffi.dart' as ffip;

import 'gitkebab.dart';
import 'gitkebab_lib.dart' as gitkebab_lib;
import 'pointer_casting.dart';
import 'status.dart';
import 'errors.dart';
import 'repository.dart';
import 'credentials.dart';
import 'session_state.dart';

Map<String, Session> g_sessions = {};

enum SessionCallbackMode {
  synchronous, polling
}

Session sessionFromSessionIdPtr(Pointer<Int8> session_id_ptr, String callbackName) {
  if (session_id_ptr.address == 0) {
    throw "received $callbackName callback with NULL session_id";
  }
  var session_id = session_id_ptr.toDartString();
  Session? session = g_sessions[session_id];
  if (session == null) {
    throw "received $callbackName callback for session [$session_id] but no such session exists";
  }
  return session;
}

void session_state_callback(Pointer<Int8> session_id_ptr, Pointer<gitkebab_lib.gk_repository> repositoryPtr, Pointer<gitkebab_lib.gk_session_progress> progressPtr) {
  String sessionId = "(unknown-id)";
  try {
    final session = sessionFromSessionIdPtr(session_id_ptr, "state changed");
    String sessionId = session.id;
    if (repositoryPtr.address == 0) {
      print(
          "WARNING: received session state changed callback for session [${session.id}] with NULL repository structure");
      return;
    }

    final oldState = session.state;
    session.state = SessionState.fromSessionPointer(session.session_ptr);
    final updateEvent = SessionStateUpdateEvent.from(oldState:oldState, newState:session.state);
    session.notifySessionStateChanged(updateEvent);
  }
  catch (exc) {
    print("Error during session state changed callback for session [$sessionId]: $exc");
  }
}

void session_merge_conflicts_query_callback(Pointer<Int8> session_id_ptr, Pointer<gitkebab_lib.gk_repository> repositoryPtr) {
  String sessionId = "(unknown-id)";
  try {
    Session session = sessionFromSessionIdPtr(session_id_ptr, "merge conflicts query");
    sessionId = session.id;
    if (repositoryPtr.address == 0) {
      print("WARNING: received merge conflicts query callback for session [${sessionId}] with NULL repository structure");
      return;
    }
    // NOTE: possible optimization, compare fetch/repository heads to
    //       existing conflict summary and only process if different
    session.mergeConflictSummary = MergeConflictSummary.forRepositoryPointer(repositoryPtr);
    session.onDidQueryMergeConflicts(session);
  }
  catch (exc) {
    print("Error during session state changed callback for session [$sessionId]: $exc");
  }
}

typedef SessionStateChangedCallback = void Function(SessionStateUpdateEvent);
typedef void MergeStateQueryCallback(Session session);

class Session {
  final callbackMode;
  Pointer<gitkebab_lib.gk_session> session_ptr = Pointer.fromAddress(0);
  String id = "<unknown>";
  late RepositorySpec repositorySpec;
  SessionState state = SessionState(0, counter: 0, progressPercent: 0);
  RepositoryStatusList status = RepositoryStatusList();
  MergeConflictSummary mergeConflictSummary = MergeConflictSummary();

  SessionStateChangedCallback? onStateChanged;

  final stateStreamController = StreamController<SessionStateUpdateEvent>.broadcast();
  get stateStream => stateStreamController.stream;

  MergeStateQueryCallback onDidQueryMergeConflicts = (session) => {};

  bool get isSynchronous => callbackMode == SessionCallbackMode.synchronous;
  bool get isPolling => callbackMode == SessionCallbackMode.polling;

  Session(String url, int urlType, String localPath, String user,
      {this.callbackMode = SessionCallbackMode.synchronous}) {
    final sessionPointer = GitKebab.lib.gk_session_new(
        url.toFfiPtr(),
        urlType,
        localPath.toFfiPtr(),
        user.toFfiPtr(),
        isSynchronous ? Pointer.fromFunction(session_state_callback) : Pointer.fromAddress(0),
        isSynchronous ? Pointer.fromFunction(session_merge_conflicts_query_callback) : Pointer.fromAddress(0));
    initWithSessionPointer(sessionPointer);
  }

  Session.fromPointer(Pointer<gitkebab_lib.gk_session> sessionPointer,
      {this.callbackMode = SessionCallbackMode.synchronous}) {
    initWithSessionPointer(sessionPointer);
  }

  void initWithSessionPointer(Pointer<gitkebab_lib.gk_session> sessionPointer) {
    this.session_ptr = sessionPointer;
    if (session_ptr.address == 0) {
      throw GitKebabException(
          gitkebab_lib.ResultCode.ERROR, "GitKebab session unexpectedly null");
    }
    if (session_ptr.ref.repository.address == 0) {
      throw GitKebabException(gitkebab_lib.ResultCode.ERROR,
          "GitKebab session repository unexpectedly null");
    }
    id = session_ptr.ref.id_ptr.toDartString();
    repositorySpec = RepositorySpec(session_ptr.ref.repository.ref.spec);

    g_sessions[id] = this;
  }

  void notifySessionStateChanged(SessionStateUpdateEvent updateEvent){
    final callback = onStateChanged;
    stateStreamController.add(updateEvent);
    if (callback != null) callback(updateEvent);
  }

  ////
  // Errors
  int lastResultCode() {
    return GitKebab.lib.gk_session_last_result_code(session_ptr);
  }

  String lastResultMessage() {
    return GitKebab.lib.gk_session_last_result_message(session_ptr).cast<
        ffip.Utf8>().toDartString();
  }

  GitKebabException lastResultException() {
    return GitKebabException(lastResultCode(), lastResultMessage());
  }

  ////
  // Initialisation
  void initialize() {
    if (GitKebab.lib.gk_session_initialize(session_ptr) != 0) {
      throw lastResultException();
    }
    if (isPolling) updateStateWithLock();
  }

  ////
  //  Remotes
  void _withCredential(Credential? credential, void Function() closure) {
    if (credential != null) credential.prepareSession(session_ptr);
    closure();
    if (credential != null) credential.cleanupSession(session_ptr);
  }

  void clone({Credential? credential}) {
    _withCredential(credential, () {
      if (GitKebab.lib.gk_clone(session_ptr) != 0) throw lastResultException();
    });
  }

  void fetch(String remoteName, {Credential? credential}) {
    _withCredential(credential, () {
      if (GitKebab.lib.gk_fetch(session_ptr, remoteName.toFfiPtr()) !=
          0) throw lastResultException();
    });
  }

  void push(String remoteName, {Credential? credential}) {
    _withCredential(credential, () {
      if (GitKebab.lib.gk_push(session_ptr, remoteName.toFfiPtr()) !=
          0) throw lastResultException();
    });
  }

  void sync({Credential? credential}) {
    _withCredential(credential, () {
      if (GitKebab.lib.gk_sync(session_ptr) != 0) throw lastResultException();
    });
  }

  Future<void> backgroundSync(
      {Credential? credential, void Function(int)? stateChangedCallback}) {
    credential?.prepareSession(session_ptr);
    GitKebab.lib.gk_background_sync(session_ptr);
    updateStateWithLock();
    return Future.doWhile(() {
      updateStateWithLock();
      if (!state.backgroundSyncInProgress) {
        credential?.cleanupSession(session_ptr);
        return false;
      }
      return Future.delayed(const Duration(milliseconds: 10), () => true);
    });
  }

  void updateStateWithLock() {
    int currentStateCounter = state.counter;
    if (currentStateCounter == session_ptr.ref.repository.ref.state_counter) return;
    if (GitKebab.lib.gk_session_state_lock(session_ptr) != 0) {
      throw lastResultException();
    }
    int progress = session_ptr.ref.progress.address == 0 ? 0 : session_ptr.ref
        .progress.ref.percent;
    final oldState = state;
    state = SessionState(session_ptr.ref.repository.ref.state,
        counter: session_ptr.ref.repository.ref.state_counter,
        progressPercent: progress);
    notifySessionStateChanged(SessionStateUpdateEvent.from(oldState: oldState, newState: state));
    if (GitKebab.lib.gk_session_state_unlock(session_ptr) != 0) {
      throw lastResultException();
    }
  }

  ////
  // Index
  void addPath(String path) {
    if (GitKebab.lib.gk_index_add_path(session_ptr, path.toFfiPtr()) != 0) {
      throw lastResultException();
    }
  }

  void removePath(String path) {
    if (GitKebab.lib.gk_index_remove_path(session_ptr, path.toFfiPtr()) !=
        0) {
      throw lastResultException();
    }
  }

  void addAll(String pattern) {
    if (GitKebab.lib.gk_index_add_all(session_ptr, pattern.toFfiPtr()) != 0) {
      throw lastResultException();
    }
  }

  void updateAll(String pattern) {
    if (GitKebab.lib.gk_index_update_all(session_ptr, pattern.toFfiPtr()) !=
        0) {
      throw lastResultException();
    }
  }

  ////
  // Status
  void queryStatus() {
    if (GitKebab.lib.gk_status_summary_query(session_ptr) != 0) {
      throw lastResultException();
    }
    status = RepositoryStatusList.forQueriedSession(session_ptr);
    if (isPolling) updateStateWithLock();
    GitKebab.lib.gk_status_summary_close(session_ptr);
  }

  ////
  // Commit
  String commit(String commitMessage) {
    Pointer<gitkebab_lib.gk_object_id> object_id_ptr = ffip.calloc<
        gitkebab_lib.gk_object_id>();

    int rc = GitKebab.lib.gk_commit(
        session_ptr, commitMessage.toFfiPtr(), object_id_ptr);
    String commitId = object_id_ptr.address == 0 ? "" : GitKebab.lib
        .gk_object_id_ptr(object_id_ptr).toDartString();
    ffip.calloc.free(object_id_ptr);

    if (rc != 0) {
      throw lastResultException();
    }
    if (object_id_ptr.address == 0) {
      throw GitKebabException(gitkebab_lib.ResultCode.ERROR,
          "commit ID after commit is unexpectedly null");
    }

    return commitId;
  }

  ////
  // Merge
  void mergeIntoHead() {
    if (GitKebab.lib.gk_merge_into_head(session_ptr) != 0) {
      throw lastResultException();
    }
  }

  void mergeIntoHeadFinalize() {
    if (GitKebab.lib.gk_merge_into_head_finalize(session_ptr) != 0) {
      throw lastResultException();
    }
  }

  void mergeAbort() {
    if (GitKebab.lib.gk_merge_abort(session_ptr) != 0) {
      throw lastResultException();
    }
  }

  void mergeConflictsQuery() {
    if (GitKebab.lib.gk_merge_conflicts_query(session_ptr) != 0) {
      throw lastResultException();
    }
  }

  ////
  // Conflicts
  String blobContents(String blobId) {
    /*
  Pointer<Int8> contents_ptr = ffip.calloc<Int8>();
  Pointer<Uint64> length_ptr = ffip.calloc<Uint64>();
  GitKebab.lib.gk_blob_contents(session_ptr, contents_ptr.cast(), length_ptr, blobId.toFfiPtr());
  String contents = contents_ptr.toDartString();
  ffip.calloc.free(contents_ptr);
  ffip.calloc.free(length_ptr);*/

    Pointer<Int8> contents_ptr = GitKebab.lib.gk_blob_new_char_contents(
        session_ptr, blobId.toFfiPtr());
    if (contents_ptr.address == 0) throw lastResultException();
    String contents = contents_ptr.toDartString();
    GitKebab.lib.gk_blob_free_char_contents(contents_ptr);
    return contents;
  }

  void writeBlobContents(String blobId, String path,
      {bool relativizePath = true}) {
    if (GitKebab.lib.gk_blob_write_contents(
        session_ptr, blobId.toFfiPtr(), path.toFfiPtr(),
        relativizePath ? 1 : 0) != 0) {
      throw lastResultException();
    }
  }

  void conflictResolveAcceptRemoteDelete(String path) {
    if (GitKebab.lib.gk_conflict_resolve_accept_remote_delete(
        session_ptr, path.toFfiPtr()) != 0) {
      throw lastResultException();
    }
  }

  void conflictResolveAcceptLocalDelete(String path) {
    if (GitKebab.lib.gk_conflict_resolve_accept_local_delete(
        session_ptr, path.toFfiPtr()) != 0) {
      throw lastResultException();
    }
  }

  void conflictResolveAcceptExisting(String path,
      ConflictResolution resolution) {
    if (GitKebab.lib.gk_conflict_resolve_accept_existing(
        session_ptr, path.toFfiPtr(), resolution.intValue()) != 0) {
      throw lastResultException();
    }
  }

  int compareBlobs(String blob1Id, String blob2Id) {
    Pointer<Int32> similarityPtr = ffip.calloc<Int32>();
    int rc = GitKebab.lib.gk_compare_blobs(
        session_ptr, similarityPtr, blob1Id.toFfiPtr(), blob2Id.toFfiPtr());
    int similarity = similarityPtr.address == 0 ? 0 : similarityPtr.value;
    ffip.calloc.free(similarityPtr);
    if (rc != 0) {
      throw lastResultException();
    }
    return similarity;
  }

  String mergedBufferWithConflictMarkers(MergeConflict conflict) {
    Pointer<Int8> bufferPtr = GitKebab.lib
        .gk_conflict_merged_buffer_with_conflict_markers(
        session_ptr, conflict.ancestorBlobId.toFfiPtr(),
        conflict.oursBlobId.toFfiPtr(), conflict.theirsBlobId.toFfiPtr(),
        conflict.path.toFfiPtr());
    String buffer = bufferPtr.toDartString();
    GitKebab.lib.gk_conflict_merged_buffer_free(bufferPtr);
    return buffer;
  }

  void conflictResolveFromBuffer(String path, String buffer) {
    if (GitKebab.lib.gk_conflict_resolve_from_buffer(
        session_ptr, path.toFfiPtr(), buffer.toVoidFfiPtr(),
        buffer.codeUnits.length) != 0) {
      throw lastResultException();
    }
  }

  ////
  // Misc
  String resolveReference(String reference) {
    Pointer<gitkebab_lib.gk_object_id> object_id_ptr = ffip.calloc<
        gitkebab_lib.gk_object_id>();

    int rc = GitKebab.lib.gk_resolve_reference(
        session_ptr, reference.toFfiPtr(), object_id_ptr);
    String commitId = object_id_ptr.address == 0 ? "" : GitKebab.lib
        .gk_object_id_ptr(object_id_ptr).toDartString();
    ffip.calloc.free(object_id_ptr);

    if (rc != 0) {
      throw lastResultException();
    }
    if (object_id_ptr.address == 0) {
      throw GitKebabException(gitkebab_lib.ResultCode.ERROR,
          "commit ID of reference is unexpectedly null");
    }

    return commitId;
  }
}

enum ConflictResolution {
  ours,
  theirs,
  ancestor
}

extension ConflictResolutionIntValue on ConflictResolution {
  int intValue() {
    switch (this) {
      case ConflictResolution.ours: return gitkebab_lib.ConflictResolution.OURS;
      case ConflictResolution.theirs: return gitkebab_lib.ConflictResolution.THEIRS;
      case ConflictResolution.ancestor: return gitkebab_lib.ConflictResolution.ANCESTOR;
    }
  }
}
