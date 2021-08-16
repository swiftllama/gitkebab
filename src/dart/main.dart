
import 'dart:io' show Platform, Directory;
import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart' as ffip;
//import 'gitkebab_lib.dart' as gk;
import 'gitkebab.dart' as gitkebab;

void main(List<String> arguments) {

  var libraryPath = Directory.current.path + "/gitkebab-linux-debug/lib/libgitkebab.so";

  gitkebab.GitKebab.load(libraryPath);

  String url = "/home/amos/Projects/gitkebab/src/test/fixtures/simple-repo1.git";
  String local_path = "/tmp/checkout-simple-repo1";
  String user = "";
  int GK_REPOSITORY_SOURCE_URL_FILESYSTEM = 2;
  
  var session = gitkebab.Session(url, GK_REPOSITORY_SOURCE_URL_FILESYSTEM, local_path, user);
  session.clone();

  
  
  //final gklib = gk.GitKebabLib(ffi.DynamicLibrary.open(libraryPath));
  //
  //gklib.gk_init();
  //
  //
  //String url = "/home/amos/Projects/gitkebab/src/test/fixtures/simple-repo1.git";
  //String local_path = "/tmp/checkout-simple-repo1";
  //String user = "";
  //int GK_REPOSITORY_SOURCE_URL_FILESYSTEM = 2;
  //
  //final ffi.Pointer<ffip.Utf8> charPointer = url.toNativeUtf8();
  //
  //var session = gklib.gk_session_new(url.toNativeUtf8().cast<ffi.Int8>(), GK_REPOSITORY_SOURCE_URL_FILESYSTEM, local_path.toNativeUtf8().cast<ffi.Int8>(), user.toNativeUtf8().cast<ffi.Int8>(), ffi.Pointer.fromAddress(0), ffi.Pointer.fromAddress(0));
  // 
  //print('Created session!: ${session}');
  //
  //gklib.gk_clone(session);
  print("cloned session maybe");
}
