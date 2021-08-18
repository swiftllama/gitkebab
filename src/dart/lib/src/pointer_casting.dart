import 'dart:ffi';
import 'package:ffi/ffi.dart' as ffip;

extension FfiUtf8Casting on String {
  Pointer<Int8> toFfiPtr() {
    return this.toNativeUtf8().cast<Int8>();
  }
}

extension PointerExtensions<T extends NativeType> on Pointer<T> {
  String toDartString() {
    if (T == Int8) {
      return this.cast<ffip.Utf8>().toDartString();
    }

    throw UnsupportedError('${T} unsupported');
  }
}