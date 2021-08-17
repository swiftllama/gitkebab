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

class Session {
  var session_ptr;
  
  Session(String url, int urlType, String localPath, String user) {
    session_ptr = GitKebab.lib.gk_session_new(url.toFfiPtr(), urlType, localPath.toFfiPtr(), user.toFfiPtr(), ffi.Pointer.fromAddress(0), ffi.Pointer.fromAddress(0));
    if (session_ptr == null) {
      throw "Error initializing session";
    }
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
}
