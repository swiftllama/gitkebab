import 'dart:ffi';
import 'gitkebab.dart';
import 'gitkebab_lib.dart';
import 'pointer_casting.dart';

enum FileStatus {
  current,
  indexNew,
  indexModified,
  indexDeleted,
  indexRenamed,
  indexTypechange,
  worktreeNew,
  worktreeModified,
  worktreeDeleted,
  worktreeTypechange,
  worktreeRenamed,
  worktreeUnreadable,
  ignored,
  conflicted,
  unimplementedUnknown
}

FileStatus FileStatusFromInt(int status) {
  if (status == GitKebab.lib.GK_FILE_STATUS_CURRENT) {
    return FileStatus.current;
  }
  if (status == GitKebab.lib.GK_FILE_STATUS_CURRENT) {
    return FileStatus.current;
  }
  if (status == GitKebab.lib.GK_FILE_STATUS_INDEX_NEW) {
    return FileStatus.indexNew;
  }
  if (status == GitKebab.lib.GK_FILE_STATUS_INDEX_MODIFIED) {
    return FileStatus.indexModified;
  }
  if (status == GitKebab.lib.GK_FILE_STATUS_INDEX_DELETED) {
    return FileStatus.indexDeleted;
  }
  if (status == GitKebab.lib.GK_FILE_STATUS_INDEX_RENAMED) {
    return FileStatus.indexRenamed;
  }
  if (status == GitKebab.lib.GK_FILE_STATUS_INDEX_TYPECHANGE) {
    return FileStatus.indexTypechange;
  }
  if (status == GitKebab.lib.GK_FILE_STATUS_WT_NEW) {
    return FileStatus.worktreeNew;
  }
  if (status == GitKebab.lib.GK_FILE_STATUS_WT_MODIFIED) {
    return FileStatus.worktreeModified;
  }
  if (status == GitKebab.lib.GK_FILE_STATUS_WT_DELETED) {
    return FileStatus.worktreeDeleted;
  }
  if (status == GitKebab.lib.GK_FILE_STATUS_WT_TYPECHANGE) {
    return FileStatus.worktreeTypechange;
  }
  if (status == GitKebab.lib.GK_FILE_STATUS_WT_RENAMED) {
    return FileStatus.worktreeRenamed;
  }
  if (status == GitKebab.lib.GK_FILE_STATUS_WT_UNREADABLE) {
    return FileStatus.worktreeUnreadable;
  }
  if (status == GitKebab.lib.GK_FILE_STATUS_IGNORED) {
    return FileStatus.ignored;
  }
  if (status == GitKebab.lib.GK_FILE_STATUS_CONFLICTED) {
    return FileStatus.conflicted;
  }
  print("ERROR: unknown file status [$status], expected one of the GitKEbab.lib.GK_FILE_STATUS_* constants, returning unimplementedUnknown");
  return FileStatus.unimplementedUnknown;
}

class RepositoryStatusListEntry {
  final String path;
  final FileStatus status;
  const RepositoryStatusListEntry(this.path, this.status);

  String toString() {
    return "<RepositoryStatusListEntry [$path] [$status]>";
  }
}

class RepositoryStatusList {
  List<RepositoryStatusListEntry> entries = [];

  RepositoryStatusList() {
  }

  static forQueriedSession(Pointer<gk_session> session_ptr) {
    RepositoryStatusList status = RepositoryStatusList();
    int entryCount = GitKebab.lib.gk_status_summary_entrycount(session_ptr);
    for (var i = 0; i < entryCount; i += 1) {
      String path = GitKebab.lib.gk_status_summary_path_at(session_ptr, i).toDartString();
      FileStatus fileStatus = FileStatusFromInt(GitKebab.lib.gk_status_summary_status_at(session_ptr, i));
      status.entries.add(RepositoryStatusListEntry(path, fileStatus));
    }
    return status;
  }

  String toString() {
    String str = "<RepositoryStatusList \n";
    for (var entry in entries) {
      str += "  $entry\n";
    }
    str += ">";
    return str;
  }
}


