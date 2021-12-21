
import 'package:test/test.dart';
import '../gitkebab.dart' as gk;
import 'common.dart';

void main() {
  initGitkebab();

  test('generate an ssh key', () {
    final key = gk.GitKebab.generateSshKey();
    print("DBG generated key: ${[key]}");
    expect(key.startsWith('-----BEGIN RSA PRIVATE KEY-----\n'), true);
    expect(key.endsWith('-----END RSA PRIVATE KEY-----\n'), true);
  });

}
