import 'dart:ffi' as ffi;
import 'gitkebab_lib.dart' as gitkebab_lib;
import 'package:ffi/ffi.dart' as ffip;


extension FfiUtf8Casting on String {
  ffi.Pointer<ffi.Int8> toFfiPtr() {
    return this.toNativeUtf8().cast<ffi.Int8>();
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

  
}
