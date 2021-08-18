import 'dart:ffi';

import 'gitkebab.dart';
import 'gitkebab_lib.dart';
import 'pointer_casting.dart';

class RepositorySpec {
  final gk_repository_spec spec;

  RepositorySpec(gk_repository_spec repositorySpec): spec  = repositorySpec;

  String get localPath => spec.local_path.toDartString();
  String get sourceUrl => spec.source_url.toDartString();
  int get sourceUrlType => spec.source_url_type;
  String get user => spec.user.toDartString();
  String get mainBranchName => spec.main_branch_name.toDartString();
  String get remoteRefName => spec.remote_ref_name.toDartString();
  String get remoteName => spec.remote_name.toDartString();
  String get pushRefspec => spec.push_refspec.toDartString();
}

enum MergeConflictType {
  IncompatibleTwoSidedEdit,
  IncompatibleTwoSidedCreate,
  LocalEditRemoteDelete,
  LocalDeleteRemoteEdit
}

MergeConflictType MergeConflictTypeFromInt(int conflictType) {
  if (conflictType == MergeConflictEntryType.INCOMPATIBLE_TWOSIDED_EDIT) { return MergeConflictType.IncompatibleTwoSidedEdit; }
  if (conflictType == MergeConflictEntryType.INCOMPATIBLE_TWOSIDED_CREATE) { return MergeConflictType.IncompatibleTwoSidedEdit; }
  if (conflictType == MergeConflictEntryType.LOCAL_EDIT_REMOTE_DELETE) { return MergeConflictType.LocalEditRemoteDelete; }
  if (conflictType == MergeConflictEntryType.LOCAL_DELETE_REMOTE_EDIT) { return MergeConflictType.LocalDeleteRemoteEdit; }
  throw "Unknown merge conflict entry type [$conflictType], expected one " +
      " of [INCOMPATIBLE_TWO_SIDED_EDIT: ${MergeConflictEntryType.INCOMPATIBLE_TWOSIDED_EDIT}], " +
      " of [INCOMPATIBLE_TWO_SIDED_CREATE: ${MergeConflictEntryType.INCOMPATIBLE_TWOSIDED_CREATE}], " +
      " of [LOCAL_EDIT_REMOTE_DELETE: ${MergeConflictEntryType.LOCAL_EDIT_REMOTE_DELETE}], " +
      " of [LOCAL_DELETE_REMOTE_EDIT: ${MergeConflictEntryType.LOCAL_DELETE_REMOTE_EDIT}] ";
}

class MergeConflict {
  final String path;
  final MergeConflictType conflictType;
  final String ancestorCommitId;
  final String oursCommitId;
  final String theirsCommitId;
  MergeConflict(this.path, this.conflictType, this.ancestorCommitId, this.oursCommitId, this.theirsCommitId);
}

class MergeConflictSummary {
  String repositoryHeadCommitId = "";
  String fetchHeadCommitId = "";
  List<MergeConflict> conflicts = [];

  static MergeConflictSummary forRepository(gk_repository repository) {
    MergeConflictSummary summary = MergeConflictSummary();
    for (var i = 0; i < repository.conflict_summary.num_conflicts; i += 1) {
      String path = repository.conflict_summary.conflicts[i].ref.path.toDartString();
      MergeConflictType conflictType = MergeConflictTypeFromInt(repository.conflict_summary.conflicts[i].ref.conflict_type);
      String ancestorCommitId = GitKebab.lib.gk_merge_conflict_entry_ancestor_oid_id(repository.conflict_summary.conflicts[i]).toDartString();
      String oursCommitId = GitKebab.lib.gk_merge_conflict_entry_ours_oid_id(repository.conflict_summary.conflicts[i]).toDartString();
      String theirsCommitId = GitKebab.lib.gk_merge_conflict_entry_theirs_oid_id(repository.conflict_summary.conflicts[i]).toDartString();
      summary.conflicts.add(MergeConflict(path, conflictType, ancestorCommitId, oursCommitId, theirsCommitId));
    }
    return summary;
  }
}