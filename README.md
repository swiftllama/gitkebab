
# Gitkebab - A high-level libgit2 wrapper

    gk_session *session = gk_session_new(repo_url, url_type, dest_path, "git", &session_state_changed, NULL);
    gk_session_initialize(session); 
    gk_session_credential_ssh_key_file_init(session, "git", key_path, NULL, key_passphrase);
    gk_clone(session);

See [src/example/main.c](src/example/main.c) for a working example.

## Project layout

The project is laid out like so:

    ├── CMakeLists.txt    # root CMake build script
    ├── README.md
    │
    ├── src
    │   ├── example       # example executable
    │   ├── lib           # source code for libgitkebab.so and libgitkebab_static.a
    │   └── test          # cmocka tests
    │
    └── build-workspace   # folder for cloning and building

The `src` folder is laid out like so:

    src/
    │
    ├── example                           # c-based gitkebab example
    │   ├── CMakeLists.txt
    │   └── main.c
    │
    ├── lib                               # c-based gitkebab static/dynamic libraries
    │   ├── CMakeLists.txt                # CMake script that defines libgitkebab.so, libgitkebab_static.so
    │   ├── ...
    │   └── gk_sync.c
    │
    └── test                              # c-based gitkebab tests (uses cmocka)
        ├── CMakeLists.txt                # CMake script that defines test binaries
        ├── fixtures
        │   ...
        └── test_status.c

## Building

See [design/building.md](design/building.md).

## Running the example

    /example1 git@github.com:swiftllama/gitkebab.git ./gitkebab /home/user/.ssh/id_ed25519
