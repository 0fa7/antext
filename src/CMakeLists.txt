set(ANTEXT_CLI_CLIENT_SOURCES
    CliClientMain.cpp)

add_executable(cli_client
    ${ANTEXT_CLI_CLIENT_SOURCES})

set(ANTEXT_IDENTITY_SERVER_SOURCES
    IdentityServerMain.cpp)

add_executable(identity_server
    ${ANTEXT_IDENTITY_SERVER_SOURCES})

target_link_libraries(identity_server
    sqlite3)