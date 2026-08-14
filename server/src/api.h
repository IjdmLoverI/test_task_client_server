#pragma once

#include <filesystem>

namespace httplib {
class Server;
}

namespace fb {

// Вешает все маршруты API на переданный сервер.
void register_routes(httplib::Server& server, const std::filesystem::path& root);

}  // namespace fb
