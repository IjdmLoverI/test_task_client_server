#include "api.h"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "fs_service.h"
#include "path_resolver.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace fb {
namespace {

constexpr const char* kJsonMime = "application/json";

// Единый формат ошибки для всего API. RESTful — это в том числе то, что
// ошибка тоже JSON, а не кусок HTML или пустое тело.
void send_error(httplib::Response& res, int status, const std::string& message) {
    json body{
        {"error", message},
        {"status", status},
    };
    res.status = status;
    res.set_content(body.dump(2), kJsonMime);
}

void send_json(httplib::Response& res, const json& body, int status = 200) {
    res.status = status;
    res.set_content(body.dump(2), kJsonMime);
}

void handle_list(const fs::path& root, const httplib::Request& req, httplib::Response& res) {
    
    if (!req.has_param("path")) {
        send_error(res, 400, "missing required parameter: path");
        return;
    }

    const std::string requested = req.get_param_value("path");

    const ResolveResult resolved = resolve_under_root(root, requested);

    if (resolved.status != ResolveStatus::Ok) {
        send_error(res, http_status_for(resolved.status), resolved.message);
        return;
    }

    std::error_code ec;
    if(!fs::is_directory(resolved.absolute, ec)) {
        send_error(res, 400, "not a directory");
        return;
    }


    json entries = json::array();

    for (const DirEntry& e : list_directory(resolved.absolute)) {
        entries.push_back({{"name", e.name}, {"type", e.type}});
    }

    send_json(res, json{{"path", requested}, {"entries", entries}});
}

void handle_file(const fs::path& root, const httplib::Request& req, httplib::Response& res) {
    
    if(!req.has_param("path")) {
        send_error(res, 400, "missing required parameter: path");
        return;
    }

    std::string requested = req.get_param_value("path");

    const ResolveResult resolved = resolve_under_root(root, requested);

    if (resolved.status != ResolveStatus::Ok) {
        send_error(res, http_status_for(resolved.status), resolved.message);
        return;
    }
    
    std::error_code ec;
    if (!fs::is_regular_file(resolved.absolute, ec)) {
        send_error(res, 400, "not a file");
        return;
    }
    

    FileInfo f_info = describe_file(resolved.absolute);

    send_json(res, json{
        {"path", requested},
        {"size", f_info.size},
        {"created", f_info.created},
        {"modified", f_info.modified},
        {"sha256", f_info.sha256}
    });
}

}  // namespace

void register_routes(httplib::Server& server, const fs::path& root) {
    // Не из ТЗ, но нужен для healthcheck в docker compose.
    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        send_json(res, json{{"status", "ok"}});
    });

    server.Get("/list", [root](const httplib::Request& req, httplib::Response& res) {
        handle_list(root, req, res);
    });

    server.Get("/file", [root](const httplib::Request& req, httplib::Response& res) {
        handle_file(root, req, res);
    });

    server.set_error_handler([](const httplib::Request&, httplib::Response& res) {
        if (res.body.empty()) {
            send_error(res, res.status, "unhandled request");
        }
    });

    // Любое исключение из обработчика -> 500 в JSON, а не разрыв соединения.
    server.set_exception_handler(
        [](const httplib::Request&, httplib::Response& res, std::exception_ptr ep) {
            std::string message = "internal error";
            try {
                std::rethrow_exception(ep);
            } catch (const std::exception& e) {
                message = e.what();
            } catch (...) {
            }
            send_error(res, 500, message);
        });
}

}  // namespace fb
