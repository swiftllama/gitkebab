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
    final logPointer = logPath == null ? Pointer<Int8>.fromAddress(0) : logPath.toFfiPtr();
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
        print("DBG found existing log file has size [${stats.size}] vs threshold [$logFileThreshold]");
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
}