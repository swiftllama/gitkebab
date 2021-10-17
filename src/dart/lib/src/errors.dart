
class GitKebabException implements Exception {
  final int code;
  final String message;
  GitKebabException(this.code, this.message);

  String toString() => "GitKebabException: $message";
}