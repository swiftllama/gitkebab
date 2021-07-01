
#include "gk_status.h"


git_status_t

Status flags for a single file.

A combination of these values will be returned to indicate the status of a file. Status compares the working directory, the index, and the current HEAD of the repository. The GIT_STATUS_INDEX set of flags represents the status of file in the index relative to the HEAD, and the GIT_STATUS_WT set of flags represent the status of the file in the working directory relative to the index.

GIT_STATUS_CURRENT
GIT_STATUS_INDEX_NEW
GIT_STATUS_INDEX_MODIFIED
GIT_STATUS_INDEX_DELETED
GIT_STATUS_INDEX_RENAMED
GIT_STATUS_INDEX_TYPECHANGE
GIT_STATUS_WT_NEW
GIT_STATUS_WT_MODIFIED
GIT_STATUS_WT_DELETED
GIT_STATUS_WT_TYPECHANGE
GIT_STATUS_WT_RENAMED
GIT_STATUS_WT_UNREADABLE
GIT_STATUS_IGNORED
GIT_STATUS_CONFLICTED
