
import 'package:test/test.dart';
import '../gitkebab.dart' as gk;
import 'common.dart';

const privateKeyBeginHeader = '-----BEGIN RSA PRIVATE KEY-----';
const privateKeyEndHeader = '-----END RSA PRIVATE KEY-----';
const publicKeyBeginHeader = '-----BEGIN RSA PUBLIC KEY-----';
const publicKeyEndHeader = '-----END RSA PUBLIC KEY-----';

void main() {
  initGitkebab();

  test('generate an ssh key synchronously', () {
    final then = DateTime.now();
    final key = gk.RSAKeyGenerator.instance.generateRsaKeySync();
    //final delta = DateTime.now().difference(then);
    //print("DBG ssh key generation took: [${delta.inMilliseconds}ms]");
    //print("DBG generated private key: ${[key.privateKey]}");
    //print("DBG generated public key: ${[key.publicKey]}");
    expect(key.privateKey.startsWith(privateKeyBeginHeader), true);
    expect(key.privateKey.endsWith('$privateKeyEndHeader\n'), true);
    expect(key.publicKey.startsWith(publicKeyBeginHeader), true);
    expect(key.publicKey.endsWith('$publicKeyEndHeader\n'), true);
  });

  test('generate an ssh key synchronously', () async {
    final keyFuture = gk.RSAKeyGenerator.instance.generateRsaKey();
    await expectLater(keyFuture, completion(
        isA<gk.RSAKey>(
        ).having(
                (key) => key.privateKey.split('\n').first, 'private key begin header', equals(privateKeyBeginHeader)
        ).having(
                (key) => key.privateKey.split('\n').lastWhere((element) => element != ''), 'private key end header', equals(privateKeyEndHeader)
        ).having(
                (key) => key.publicKey.split('\n').first, 'public key begin header', equals(publicKeyBeginHeader)
        ).having(
                (key) => key.publicKey.split('\n').lastWhere((element) => element != ''), 'public key end header', equals(publicKeyEndHeader)
        )
    ));
  });
}
