#pragma once

#include <filesystem>

namespace httplib {
class Server;
}

namespace fb {

// Registers every API route on the given server.
void register_routes(httplib::Server& server, const std::filesystem::path& root);

}  // namespace fb
