
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
  
  var session = gitkebab.Session(url, gitkebab.RepositorySourceUrlType.FILESYSTEM, local_path, user);
  session.onProgress = (session, progress) => print("session progress: ${progress.percent}% ${progress.description_ptr.toDartString()}");
  session.onStateChanged = (session, repository) => print("session state changed: ${repository.state}");

  session.clone();
  print("DBG finished clone with result code: [${session.lastResultCode()}]");

  session.clone();
  print("DBG second clone attempt: [${session.lastResultCode()}]");
  print("DBG second clone attempt: [${session.lastResultMessage()}]");
  
}
