import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart' as ffip;

import 'gitkebab_lib.dart' as gitkebab_lib;
import 'gitkebab_lib.dart' show ConflictResolution, MergeConflictEntryType,
  RepositorySourceUrlType, RepositoryState, RepositoryVerifyCondition,
  SessionCredentialType, SessionProgressEventType,
  gk_repository, ResultCode;

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

