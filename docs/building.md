
# Gitkebab Build Workspace

A folder structure and set of scripts that can be used to build gitkebab

## Folder structure

    ├── build
    │   ├── tmp                     # contains temp build folders where builds actually take place
    │   │   └── cmocka-1.1.5        # temp folder for cmocka builds (by platform/arch)
    │   └── cmocka-1.1.5            # finished (installed) builds of cmocka, by platform, arch and release type
    │
    ├── scripts                     # build scripts by library
    │   └── cmocka-1.1.5            # scripts for cmocka v1.1.5 (by platform and architecture)
    │       ├── source              # scripts for cloning the cmocka source code (by platform/arch)
    │       └── build               # scripts for building the cmocka source code (by platform/arch)
    └── source                      # cloned sources by component
        ├── tmp                     # temporary source folder for libraries that need to e.g. download and unzip a zipfile
        └── cmocka-1.1.5            # source of library

The scripts for each component follow the following pattern, e.g. in `scripts/cmocka-1.1.5/`:

    scripts/cmocka-1.1.5/
      ├── build
      │   └── linux
      │       └── x86_64-release
      │           ├── check.sh       # check if the build succeeded
      │           ├── delete.sh      # delete the build
      │           ├── run-direct.sh  # build the componnet (intended to be run from "run.sh")
      │           └── run.sh         # setup build env and execute "run-direct.sh" (e.g. start a docker container)
      └── source
          ├── check.sh               # check if the source is in place: exit-code: 0 => yes
          ├── clone.sh               # clone (or otherwise download/copy) the source into the "source/" folder
          └── delete.sh              # delete the source code from the "source/" folder


## Source/Build scripts

The source and build scripts are intended to be usable manually from
the commandline. To this end they return 0 on success and != 0 on
failure.

Note that at the moment `pcre2-10.35` is not used at all (libgit2
depends on `pcre-8.45` and doesn't seem to be compatible with `pcre2`).

Similarly, `iconv-1.16` is not used on linux (needed by libgit2 for
macOS/ios only).

Note that the sources for all projects either clone directly from
github or download from the various project sites (e.g. pcre,
zlib). The sources for gitkebab are copied from the parent folder.


## Building

Currently building from scratch looks something like this:

    ./scripts/cmocka-1.1.5/source/clone.sh
    ./scripts/zlib-1.2.12/source/clone.sh
    ./scripts/pcre-8.45/source/clone.sh
    ./scripts/openssl-1.1.1n/source/clone.sh
    ./scripts/libssh2-1.9.0/source/clone.sh
    ./scripts/libgit2-1.4.2/source/clone.sh
    ./scripts/gitkebab-head/source/clone.sh
    
    # Optionally verify using the source/check.sh scripts
    
    ./scripts/cmocka-1.1.5/build-linux-debug.sh
    ./scripts/zlib-1.2.11/build-linux-debug.sh
    ./scripts/pcre-8.45/build-linux-debug.sh
    ./scripts/openssl-1.1.1k/build-linux-debug.sh
    ./scripts/libssh2-1.9.0/build-linux-debug.sh
    ./scripts/libgit2-1.1.1/build-linux-debug.sh
    ./scripts/gitkebab-head/build-linux-debug.sh
    
    # Optionally verify using the build/<platform>/<arch>/check.sh scripts

The final results are in

    build/gitkebab-head/
    
e.g.:

    build/gitkebab-head/linux/x86_64-debug/
    ├── bin
    │   ├── example1
    │   └── ...
    ├── include
    │   ├── gitkebab.h
    │   ├── ...
    │   └── gk_status.h
    ├── lib
    │   ├── libgitkebab.so
    │   └── libgitkebab_static.a
    └── test
        ├── fixtures
        ├── run-all-tests.sh
        ├── test_clone
        ├── ...
        └── test_sync

To run the tests:

    cd build/gitkebab-head/linux/x86_64-debug/test
    # run an individual test
    ./test_index
    # run all the tests
    ./run-all-tests.sh    # this is created by src/test/CMakeLists.txt

Note that running the tests using cmake/ctest may fail, e.g.:

    ctest --test-dir build/gitkebab-head/linux/debug/test/ -V
    # fails

# Debugging 

## Debugging cmake builds

To debug Cmake `find_package` or `find_path` issues the `--debug-find`
argument can be appended to the cmake command in the
`build-linux-debug.sh` script. Make sure to append it to the
configuration part of the command, e.g.:

    docker run -v${PWD}:/tmp/workspace --user ${USER_ID} rikorose/gcc-cmake /bin/bash -c "cd /tmp/workspace/${TMP_BUILD_FOLDER}; cmake ../../../../../source/gitkebab-head -DCMAKE_PREFIX_PATH=\"${CUSTOM_SEARCH_PATH}\" -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=/tmp/workspace/${BUILD_FOLDER} --debug-find; cmake --build .; cmake --build . --target install"
    
To debug compiler/linker issues, the `-- VERBOSE=1` flag can be added
to the build part of the command, e.g.:

    docker run -v${PWD}:/tmp/workspace --user ${USER_ID} rikorose/gcc-cmake /bin/bash -c "cd /tmp/workspace/${TMP_BUILD_FOLDER}; cmake ../../../../../source/gitkebab-head -DCMAKE_PREFIX_PATH=\"${CUSTOM_SEARCH_PATH}\" -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=/tmp/workspace/${BUILD_FOLDER}; cmake --build . -- VERBOSE=1; cmake --build . --target install"

If necessary, the build or source files can be inspected in the respective temp folders, e.g. 

    build/tmp/pcre-8.45
    source/tmp/pcre-8.45
    
(But note that not all libraries use a tmp folder for the sources,
some just git-clone it directly into the `sources/` folder).

Note that

    message(FATAL_ERROR "<msg>") 
    
will print a message from within a CMakeLists file and stop the
build, whereas

    message(ERROR "<msg>")
    
will print the message but not stop.


## Debugging binaries with gdb

One issue with debug symbols is that the debug-info set by the
compiler can have the wrong paths, since they were built inside a
docker image. To examine the paths:

    objdump -g ./test_merge  | less
    # Search for ".debug_info section"
    
e.g. `/tmp/workspace/...` may not exist on the host machine

     <0><c>: Abbrev Number: 26 (DW_TAG_compile_unit)
        <d>   DW_AT_producer    : (indirect string, offset: 0x81): GNU C17 11.1.0 -mtune=generic -march=x86-64 -g
        <11>   DW_AT_language    : 29       (C11)
        <12>   DW_AT_name        : (indirect line string, offset: 0x3c): /tmp/workspace/source/gitkebab-head/src/test/test_merge.c
        <16>   DW_AT_comp_dir    : (indirect line string, offset: 0x0): /tmp/workspace/build/tmp/gitkebab-head/linux/debug/src/test
        
To fix that one can issue the `directory <path>` command to gdb to
help it find the sources, e.g. if it has a hard time finding the libgit2 sources:

    directory ~/Projects/gitkebab/build-workspace/build/tmp/libgit2-1.1.1/linux/debug/src/

To debug e.g. `test_merge`:

    gdb test_merge
    directory ~/Projects/gitkebab/build-workspace/source/libgit2-1.1.1/src/
    b git_remote_push  # breakpoint on git_remote_push
    r                  # run the binary, will break on git_remote_push
    
