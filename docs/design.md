
# Gitkebab Architecture

## Source code layout

In `src/lib`:

    gitkebab.h     # main gitkebab header, includes all other necessary headers
    
    gk_types.h     # contains enum, structs and typedefs

    rxi_log.h
    rxi_log.c      # minimally modified copy of https://github.com/rxi/log.c
    
    gk_logging.h   # logging header for import internally, imports rxi_log and the log components
    
    gk_log_components.h
    gk_log_components.c  # define the various log components and sets their default levels
    
    gk_lg2_private.h     # only header that includes libgit2, only used internally and not exposed
    gk_lg2.c             # main libgit2 includer, tries to contain most of the libgit2-related machinery
    
    gk_filesystem.h
    gk_filesystem.c      # filesystem helper functions
    
    # other sources loosely organized by theme, e.g.
    # gk_index.c/gk_index.h contains functions that
    # deal with the index, whereas gk_merge.c/gk_merge.h
    # contains functions that deal with merging.
    
In general GitKebab does NOT expose libgit2 headers or functions, so
any includes of libgit2 either happen in `gk_lg2_private.h` or are
hidden inside `*.c` files. 

## Overall design of the code

### Structs

The primary structs used are:

    gk_result
    
        represents the final result of an operation with an int code and a message
    

    gk_repository_spec
    
        contains descriptive data about a repository (no state)

    
    gk_repository
    
        contains a gk_repository_spec and various state: repository
        state, status summary, conflict summary, internal libgit2 state
        
        
    gk_lg2_resources
    
        contains all libgit2 long-lived resources (index, repository, etc)
        

    gk_session
    
        represents a repository plus all state necessary to carry out
        operations on it: repository, libgit2 data, credentials, etc.
        Also stores success/error information for the last operation


    gk_session_progress
    
        a single structure representing progress data for events coming
        from libgit2 - it bridges over libgit2's separate progress events
        for fetch, push, checkout, etc
        
    
    gk_session_credential
    
        used by gk_session to store credentials for operations that require
        authentication, e.g. pushing to a remote server over SSH
        
    
    gk_repository_state
        
        represents the current state of an open repo (as part of a session)
        
        
    gk_status_summary
    
       represents the status of the index/worktree, e.g. which files were 
       added, removed etc
       
       
    gk_merge_conflict_summary
    
        represents the status of any conflicts that occurred durnig a merge
        

### Sessions
    
The central element is that of a session, represented by `gk_session`:

    typedef struct {
        char id[16];                              # Unique ID generated at runtime
        char *id_ptr; // dart string access
        gk_execution_context *context;            # execution context for last executed session function
        gk_repository *repository;
        gk_session_credential credential;
        gk_session_callbacks callbacks;
        gk_result *internal_last_result;          # result of last executed session function
    } gk_session;

The session groups together the various state and data variables
necessary to carry out git opertions. It is intended to represent a
repository in addition to ongoing state relevant to the current set of
operations. Its indended use is like so:

    gk_session *session = gk_session_new("git@github.com:swiftllama/gitkebab.git", GK_REPOSITORY_SOURCE_URL_SSH, "./gitkebab", "git", &session_progress_callback, &session_state_callback, &merge_conflict_query_callback);
    
    gk_clone(session);
    ...
    gk_commit(session, "a commit message");
    ..
    gk_pull(session);

The functions `gk_clone()`, `gk_commit()`, `gk_pull()` are examples of
"session functions" - functions that perform git operations and act
on/using the session. Each such function stores its result in
`internal_last_result` and its execution contxt in `context`. The last
result is available via

    gk_session_last_result_code(session);
    gk_session_last_result_message(session);


#### Execution contexts

An execution context is meant to track the code path of a function as
it executes and record meaningful information in the case of a
failure. It is a recursive structure represented by
`gk_execution_context`:

    struct gk_execution_context {
        const char *purpose;
        gk_result *result;
        gk_execution_context *child_context;
        log_Component *log_component;
    };

It has a "purpose", which is a string describing the current
"operation", a child context and a result (both initially `NULL`). An
"operation" is intended to capture a single logical task, e.g. "clone"
or "fetch", which may be further divided into sub-operations, each
with their own context. 

A general description of the "flow" would be:

    when starting a new operation (or sub-operation) - push a new context

    when an operation (or sub-operation) finishes successfully - pop the context
    when an operation fails - don't pop the context and bubble up the failure
    
    if all operations succeeded, set the session's last result code to GK_SUCCESS
    if an operation failed, set the session's last result code to GK_ERR* and use the context backtrace as the message
    
Given a psuedo-code function that defines the following contexts/operations:

    start operation A (push new context with purpose "operation A")
      start sub-operation 1 (push new context with purpose "sub-operation 1")
      sub-operation 1 ended - pop "sub-operation 1" if successful or return failure
      
      start sub-operation 2 (push new context with purpose "sub-operation 2")
      sub-operation 2 ended - pop "sub-operation 2" if successful or return failure
      
      start sub-operation 3 (push new context with purpose "sub-operation 3")
      sub-operation 3 ended - pop "sub-operation 3" if successful or return failure
    operation A ended - pop "operation A" if successful or return failure
      
Then the result on full success of all operations is that the context
chain is "empty" and the last result code is `GK_SUCCESS`.

On the other hand if for example `operation 2` failed then the context
chain should look like:

    Operation A
      - sub-operation 2
          - error message from sub-operation 2

And the session's last result code will reflect the error. 
    
A new context may be pushed onto the current context - to represent a
sub-operation - using:

    gk_session_context_push(session, purpose, logComponent, verifyCondition) != GK_SUCCESS)
    
When an operation completes successfully it must call:

    gk_session_success(session, purpose);
    
This pops the last execution context of the session's context
chain. When a failure is encountered the operation must call one of:

    gk_session_failure_ex(session, purpose, errCode, errorMessage);
    gk_session_failure(session, purpose);
    
The difference being that the first records an error code and a
message (often useful for a direct failure, e.g. error to read a file)
whereas the second records a generic failure (often useful when a
sub-operation failed and it is not necessary to add any more
information).

There are also convience `gk_session_g2_failure*` variants that grab error
information directly out of libgit2.

At the implementation level, each session has a "root" context as part
of the gk_session struct whose purpose is "root context" and which is
never deallocated. It is essentially invisible and serves only as a
first element in the linked list of execution contexts for that
session.

#### Libgit2 state

Long-lasting libgit2 state is currently kept in the

    gk_lg2_resources
    
struct. This struct is intended to store e.g. the `git_index`
currently open. It has related functions like:

    gk_lg2_index_load(session)
    gk_lg2_index_free(session->repository)
    
which will load or free (respectively) the index resource in the
sessions' `lg2_resources` struct. 

### logging

Logging is currently handled by a minimally-modified copy of
[rxi-log](https://github.com/rxi/log.c) which works with
components. Log components currently log to stdout. Although rxi-log
supports logging to a file this is not currently implemented. There is
also currently no way to get alog callback outside of gitkebab
(however this is supported by rxi-log).
