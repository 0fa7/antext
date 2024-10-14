#include <crow.h>
#include <crow/http_response.h>
#include <crow/json.h>
#include <iostream>
#include <memory>
#include <sqlite3.h>

bool IsJsonRequest(const crow::request &req);

std::string CONTENT_TYPE_KEY = "Content-Type";
std::string CONTENT_TYPE_VALUE_JSON = "application/json";
std::string ERRCODE_KEY = "errcode";
std::string ERROR_KEY = "error";
std::string M_UNRECOGNIZED = "M_UNRECOGNIZED";

crow::response ErrorResponse(const std::string &errcode,
                             const std::string &error) {
  crow::response resp;
  resp.add_header(CONTENT_TYPE_KEY, CONTENT_TYPE_VALUE_JSON);
  crow::json::wvalue jsonBody;
  jsonBody[ERRCODE_KEY] = errcode;
  jsonBody[ERROR_KEY] = error;
  resp.body = jsonBody.dump();
  return resp;
}

template <typename T> struct Db {
  Db() {}
  ~Db() {}
};

struct SqliteDb {
  SqliteDb(std::string_view dbPath) : dbPath_{dbPath} { InitDb(); }

  ~SqliteDb() {
    sqlite3_close(db_);
    db_ = nullptr;
  }

  void InitDb() {
    char *zErrMsg = 0;
    int rc = 0;

    rc = sqlite3_open(dbPath_.c_str(), &db_);
    
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db_));
        sqlite3_close(db_);
        return;
    }
  }

  sqlite3 *db_ = nullptr;

  std::mutex dbMutex_;
  std::string dbPath_;
};

struct Auth {
  static crow::response Register(const crow::request &req) {
    if (!IsJsonRequest(req)) {
      std::string error = "Bad http header.  Ensure " + CONTENT_TYPE_KEY +
                          ": " + CONTENT_TYPE_VALUE_JSON + " is set";
      return ErrorResponse(M_UNRECOGNIZED, error);
    }

    if (!IsValidRegisterJsonRequest(req.body)) {
      std::string error = "Bad http body.  Ensure JSON fields for "
                          "/account/register endpoint are set properly: "
                          "https://spec.matrix.org/v1.12/identity-service-api/"
                          "#post_matrixidentityv2accountregister";
      return ErrorResponse(M_UNRECOGNIZED, error);
    }

    crow::response resp = ProcessRegisterRequest(req);

    return resp;
  }

  static bool IsValidRegisterJsonRequest(const std::string &body) {
    auto reqJson = crow::json::load(body);

    if (!reqJson.has("access_token")) {
      return false;
    }

    if (!reqJson.has("token_type")) {
      return false;
    }

    if (!reqJson.has("matrix_server_name")) {
      return false;
    }

    if (!reqJson.has("expires_in")) {
      return false;
    }

    return true;
  }

  static crow::response ProcessRegisterRequest(const crow::request &req) {
    return {};
  }

  static void Account() {}

  static void AccountLogout() {}
};

bool IsJsonRequest(const crow::request &req) {
  auto it = req.headers.find(CONTENT_TYPE_KEY);

  if (it == req.headers.end()) {
    return false;
  }

  if (it->second != CONTENT_TYPE_VALUE_JSON) {
    return false;
  }

  return true;
}

int main() {
  std::cout << "identity server" << std::endl;
  crow::SimpleApp app;
  auto db = std::make_unique<SqliteDb>("test_users.db");

  CROW_ROUTE(app, "/account/register")
      .methods(crow::HTTPMethod::POST)(&Auth::Register);
  app.port(8080).multithreaded().run();
}
