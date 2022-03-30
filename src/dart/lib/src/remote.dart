import 'dart:ffi';
import 'pointer_casting.dart';
import 'gitkebab_lib.dart' as gitkebab_lib;

class Remote {
  String name;
  String url;
  Remote({required this.name, required this.url});

  static Remote? fromPointer(Pointer<gitkebab_lib.gk_remote> remote_ptr) {
    if (remote_ptr.address == 0) return null;
    final name = remote_ptr.ref.name.toDartString();
    final url = remote_ptr.ref.url.toDartString();
    return Remote(name:name, url: url);
  }

  static List<Remote>? listFromPointer(Pointer<gitkebab_lib.gk_remote_list> remotes_ptr) {
    if (remotes_ptr.address == 0) return null;
    if (remotes_ptr.ref.remotes.address == 0) return [];
    final remotes = <Remote>[];
    for (var i = 0; i < remotes_ptr.ref.count; i += 1) {
      final next_remote_ptr = remotes_ptr.ref.remotes.elementAt(i);
      if (next_remote_ptr.address == 0) continue;
      final next_remote = Remote.fromPointer(next_remote_ptr.value);
      if (next_remote == null) continue;
      remotes.add(next_remote);
    }
    return remotes;
  }

  @override
  String toString() => 'Remote(name:$name, url:$url)';
}