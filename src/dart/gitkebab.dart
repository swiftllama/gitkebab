import 'dart:ffi' as ffi;

typedef gkVoid_func = ffi.Void Function();
typedef gkVoid = void Function();

typedef gkSessionNew_func = ffi.Pointer<ffi.Void> Function(ffi.Pointer<Utf8>, int, ffi.Pointer<Utf8>, ffi.Pointer<Utf8>, );
typedef gkVoidPointer = ffi.Pointer<void> Function();

class GitKebabLib {
  ffi.DynamicLibrary? gitkebab_lib;
  gkVoid gk_init = () => print("gk_init not initialized");
  gkVoidPointer gk_session_new = () { print("gk_session_new not initialized"); return ffi.Pointer.fromAddress(0); };

  GitKebabLib(String libraryPath) {
    gitkebab_lib = ffi.DynamicLibrary.open(libraryPath);
    // NOTE: null check doesn't work for class variable, so copy
    //       to local variable instead
    ffi.DynamicLibrary? gk_lib = gitkebab_lib;
    if (gk_lib == null) {
      throw "Error loading GitKebab lib from path [${libraryPath}]";
    }
    else {
      gk_init = gk_lib.lookup<ffi.NativeFunction<gkVoid_func>>('gk_init').asFunction();
      gk_session_new = gitkebab_lib?.lookup<ffi.NativeFunction<gkSession
    }
  }
  
  void init() {
    gk_init();
  }

  
}
