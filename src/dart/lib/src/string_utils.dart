
String shortCommitId(String commitId) {
  return commitId.length < 8 ? commitId : commitId.substring(0, 8);
}