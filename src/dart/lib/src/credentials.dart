import 'dart:ffi';
import 'gitkebab_lib.dart' as gitkebab_lib;
import 'gitkebab.dart';
import 'pointer_casting.dart';

abstract class Credential {
  void prepareSession(Pointer<gitkebab_lib.gk_session> session_ptr);

  void cleanupSession(Pointer<gitkebab_lib.gk_session> session_ptr) {
    GitKebab.lib.gk_session_free_credential(session_ptr);
  }
}

class InMemoryKeyCredential extends Credential {
  final String key;
  final String passphrase;
  InMemoryKeyCredential({required this.key, required this.passphrase});

  @override
  void prepareSession(Pointer<gitkebab_lib.gk_session> session_ptr) {
    final keyBytes = key.toFfiPtr();
    final publicKeyBytes = ''.toFfiPtr();
    final passphraseBytes = passphrase.toFfiPtr();
    GitKebab.lib.gk_session_credential_ssh_key_memory_init(session_ptr, keyBytes, publicKeyBytes, passphraseBytes);
  }
}