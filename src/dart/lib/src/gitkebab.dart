import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart' as ffip;

import 'gitkebab_lib.dart';
import 'pointer_casting.dart';

class GitKebab {
  static int logFileThreshold = 1024*1024;
  static GitKebabLib? _lib = null;

  static String? logPath;
  static String? rotatedLogPath;

  static void load(String libraryPath, {String? logPath, int logLevel = LOG_INFO, void Function(String error)? onLogInitError}) {
    _lib = GitKebabLib(DynamicLibrary.open(libraryPath));
    final logPointer = logPath == null ? Pointer<Char>.fromAddress(0) : logPath.toFfiPtr();
    GitKebab.logPath = logPath;

    if (logPath != null) {
      final rotatedLogPath = '$logPath-1';
      GitKebab.rotatedLogPath = rotatedLogPath;
      rotateLogFile(logPath, rotatedLogPath, onLogInitError);
    }

    _lib!.gk_init(logPointer, logLevel);
  }

  static void rotateLogFile(String logPath, String rotatedLogPath, void Function(String error)? onLogInitError) {
    try {
      final logFile = File(logPath);;
      if (logFile.existsSync()) {
        final stats = logFile.statSync();
        if (stats.size > logFileThreshold) {
          logFile.rename(rotatedLogPath);
        }
      }
    }
    catch (exc, stackTrace) {
      final msg = 'Cannot rotate log: $exc\n $stackTrace';
      if (onLogInitError != null) {
        onLogInitError(msg);
      }
      else {
        print(msg);
      }
    }

  }
  static GitKebabLib get lib {
    if (_lib == null) {
      throw "Cannot access GitKebab library, GitKebab not initialized";
    }
    return _lib!;
  }

  /// Overrides the process HOME environment variable. libgit2's SSH
  /// transport reads `$HOME/.ssh/known_hosts` and aborts the
  /// connection if HOME isn't set (Android), so the embedder calls
  /// this once with a writable directory it owns and guarantees
  /// `.ssh/known_hosts` exists there.
  static int setHome(String path) {
    final ptr = path.toFfiPtr();
    try {
      return lib.gk_set_home(ptr);
    } finally {
      ffip.calloc.free(ptr);
    }
  }
}
