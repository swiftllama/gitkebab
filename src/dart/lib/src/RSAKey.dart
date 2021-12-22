import 'gitkebab.dart';
import 'pointer_casting.dart';

class RSAKey {
  final String privateKey;
  final String publicKey;

  RSAKey(this.privateKey, this.publicKey);
}

class RSAKeyGenerator {
  static final instance = RSAKeyGenerator();
  static final timeout = 10;

  RSAKey generateRsaKeySync() {
    GitKebab.lib.gk_keys_rsa_key_generate();
    if (GitKebab.lib.gk_keys_has_errors() == 1) {
      String errors = GitKebab.lib.gk_keys_errors().toDartString();
      GitKebab.lib.gk_keys_rsa_key_free();
      throw errors;
    }

    String privateKey = GitKebab.lib.gk_keys_generated_private_key().toDartString();
    String publicKey = GitKebab.lib.gk_keys_generated_public_key().toDartString();
    GitKebab.lib.gk_keys_rsa_key_free();
    return RSAKey(privateKey, publicKey);
  }

  // Note: synchronous key generation takes around 50ms on my laptop
  Future<RSAKey> generateRsaKey() async {
    GitKebab.lib.gk_keys_rsa_key_generate_background();
    final startTime = DateTime.now();
    return Future.doWhile(() {
      if (GitKebab.lib.gk_keys_key_generation_in_progress() == 0) return Future.value(false);
      if (DateTime.now().difference(startTime).inSeconds >= timeout) throw 'Error generating RSA key, timeout of ${timeout}s expired';
      return Future.delayed(const Duration(milliseconds: 20), () => true);
    }).then((value) {
      if (GitKebab.lib.gk_keys_has_errors() == 1) {
        String errors = GitKebab.lib.gk_keys_errors().toDartString();
        GitKebab.lib.gk_keys_rsa_key_free();
        throw errors;
      }

      String privateKey = GitKebab.lib.gk_keys_generated_private_key().toDartString();
      String publicKey = GitKebab.lib.gk_keys_generated_public_key().toDartString();
      GitKebab.lib.gk_keys_rsa_key_free();
      return RSAKey(privateKey, publicKey);
    });
  }
}
