import 'dart:ffi';
import 'package:ffi/ffi.dart' as ffip;

extension FfiUtf8Casting on String {
  Pointer<Char> toFfiPtr() {
    return this.toNativeUtf8().cast<Char>();
  }

  Pointer<Void> toVoidFfiPtr() {
    return this.toNativeUtf8().cast<Void>();
  }
}

extension PointerExtensions<T extends NativeType> on Pointer<T> {
  String toDartString() {
    if (T == Int8 || T == Char) {
      return this.cast<ffip.Utf8>().toDartString();
    }

    throw UnsupportedError('${T} unsupported');
  }
}
