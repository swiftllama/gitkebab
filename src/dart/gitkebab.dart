import 'dart:ffi' as ffi;
import 'gitkebab_lib.dart' as gitkebab_lib;
import 'package:ffi/ffi.dart' as ffip;

export 'gitkebab_lib.dart' show ConflictResolution, MergeConflictEntryType, RepositorySourceUrlType, RepositoryState, RepositoryVerifyCondition, SessionCredentialType, SessionProgresEventType;

extension FfiUtf8Casting on String {
  ffi.Pointer<ffi.Int8> toFfiPtr() {
    return this.toNativeUtf8().cast<ffi.Int8>();
  }
}


extension PointerExtensions<T extends ffi.NativeType> on ffi.Pointer<T> {
  String toDartString() {
    if (T == ffi.Int8) {
      return this.cast<ffip.Utf8>().toDartString();
    }

    throw UnsupportedError('${T} unsupported');
  }
}

class GitKebab {
  static gitkebab_lib.GitKebabLib? _lib = null;
  
  static load(String libraryPath) {
    _lib = gitkebab_lib.GitKebabLib(ffi.DynamicLibrary.open(libraryPath));
    _lib!.gk_init();
    
  }

  static gitkebab_lib.GitKebabLib get lib {
    if (_lib == null) {
      throw "Cannot access GitKebab library, GitKebab not initialized";
    }
    return _lib!;
  }
}

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
  session.onStateChanged(session, repository);
}

typedef void SessionProgressCallback(Session session, gitkebab_lib.gk_session_progress progress);
typedef void SessionStateCallback(Session session, gitkebab_lib.gk_repository repository);

class Session {
  ffi.Pointer<gitkebab_lib.gk_session> session_ptr = ffi.Pointer.fromAddress(0);
  String id = "<unknown>";
  SessionProgressCallback onProgress = (session, progress) => {};
  SessionStateCallback onStateChanged = (session, repository) => {};
  
  Session(String url, int urlType, String localPath, String user) {
    session_ptr = GitKebab.lib.gk_session_new(url.toFfiPtr(), urlType, localPath.toFfiPtr(), user.toFfiPtr(), ffi.Pointer.fromFunction(session_progress_callback), ffi.Pointer.fromFunction(session_state_callback));
    if (session_ptr == null) {
      throw "Error initializing session";
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

  void progressCallback(gitkebab_lib.gk_session_progress sessionProgress) {
    
  }
}
