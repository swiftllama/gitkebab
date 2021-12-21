import 'dart:ffi';
import 'package:ffi/ffi.dart' as ffip;

import 'gitkebab_lib.dart';
import 'pointer_casting.dart';

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

  static String generateSshKey() {
    Pointer<Int32> lengthPtr = ffip.calloc<Int32>();
    final keyPtr = lib.gk_keys_rsa_key_generate(lengthPtr);
    if (keyPtr.address == 0) throw 'Error generating ssh key';
    final dartKey = keyPtr.toDartString();
    lib.gk_keys_rsa_key_free(keyPtr);
    ffip.calloc.free(lengthPtr);
    return dartKey;
  }
}

