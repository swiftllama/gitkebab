import 'dart:ffi';

import 'gitkebab_lib.dart';
import 'pointer_casting.dart';

/*
const char *local_path;
const char *source_url;
gk_repository_source_url_type source_url_type;
const char *user;
const char *main_branch_name;
const char *remote_ref_name;
const char *remote_name;
const char *push_refspec;
*/
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
