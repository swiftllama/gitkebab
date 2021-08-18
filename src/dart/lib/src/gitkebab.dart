import 'dart:ffi';

import 'gitkebab_lib.dart';

class GitKebab {
  static GitKebabLib? _lib = null;
  
  static load(String libraryPath) {
    _lib = GitKebabLib(DynamicLibrary.open(libraryPath));
    _lib!.gk_init();
  }

  static GitKebabLib get lib {
    if (_lib == null) {
      throw "Cannot access GitKebab library, GitKebab not initialized";
    }
    return _lib!;
  }
}

