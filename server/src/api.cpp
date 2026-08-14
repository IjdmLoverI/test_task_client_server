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
    // ------------------------------------------------------------------
    // TODO 13. Достать параметр: req.has_param("path") / req.get_param_value("path").
    //   Нет параметра -> 400 с внятным текстом.
    //   Замечание по API-дизайну: реши, считать ли отсутствующий path
    //   корнем — и опиши решение в README. Я бы отдавал 400: явное лучше
    //   неявного, а клиент всё равно всегда шлёт path.
    // ------------------------------------------------------------------

    // ------------------------------------------------------------------
    // TODO 14. Прогнать через resolve_under_root(). Не Ok ->
    //   send_error(res, http_status_for(status), message).
    // ------------------------------------------------------------------

    // ------------------------------------------------------------------
    // TODO 15. Убедиться, что это именно директория. Если файл — 400
    //   ("not a directory"): просить список файлов у файла бессмысленно.
    // ------------------------------------------------------------------

    // ------------------------------------------------------------------
    // TODO 16. Собрать ответ через list_directory(). Форма:
    //   {
    //     "path": "/docs",
    //     "entries": [
    //       {"name": "guide.md", "type": "file"},
    //       {"name": "nested",   "type": "dir"}
    //     ]
    //   }
    //   Отдавай path тот, что запросил клиент (логический, от корня), а НЕ
    //   абсолютный путь внутри контейнера — иначе утечёт внутренняя ФС.
    // ------------------------------------------------------------------

    (void)root;
    (void)req;
    send_error(res, 501, "/list is not implemented yet");
}

void handle_file(const fs::path& root, const httplib::Request& req, httplib::Response& res) {
    // ------------------------------------------------------------------
    // TODO 17. Те же шаги, что в handle_list: параметр -> resolve -> проверка.
    //   Здесь проверка обратная: это должен быть обычный файл
    //   (fs::is_regular_file). Директория -> 400.
    // ------------------------------------------------------------------

    // ------------------------------------------------------------------
    // TODO 18. Ответ по describe_file():
    //   {
    //     "path": "/docs/guide.md",
    //     "size": 1234,
    //     "created": "2026-08-14T09:30:00Z",
    //     "modified": "2026-08-14T10:15:42Z",
    //     "sha256": "e3b0c442..."
    //   }
    //   size отдавай числом, а не строкой.
    // ------------------------------------------------------------------

    (void)root;
    (void)req;
    send_error(res, 501, "/file is not implemented yet");
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
