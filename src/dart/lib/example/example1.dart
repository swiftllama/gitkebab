import 'dart:io';
import '../gitkebab.dart' as gk;

final ssh_key = "" + 
"-----BEGIN RSA PRIVATE KEY-----\n" + 
"...\n" +
"-----END RSA PRIVATE KEY-----";

void main() {
  // NOTE: this path is relative to the current folder
  final libraryPath = Directory.current.path + "/../../gitkebab-linux-debug/lib/libgitkebab.so";
  gk.GitKebab.load(libraryPath);

  final localPath = '/tmp/clone2';
  final session1 = gk.Session("git@gitea.ptskl.com:volund/experimental-notebook.git", gk.RepositorySourceUrlType.SSH, localPath, "");

  session1.initialize();
  session1.clone(credential:gk.InMemoryKeyCredential(user:"git", key:ssh_key, passphrase:''));
}
