set(ANTEXT_CLI_CLIENT_SOURCES
  cli_client.cpp
  mt_work_queue.cpp)

add_executable(cli_client
  ${ANTEXT_CLI_CLIENT_SOURCES})

set(ANTEXT_IDENTITY_SERVER_SOURCES
  ws_server.cpp)

add_executable(ws_server
  ${ANTEXT_IDENTITY_SERVER_SOURCES})

target_link_libraries(ws_server
  sqlite3)