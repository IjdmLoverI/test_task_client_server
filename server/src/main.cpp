#include <httplib.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "api.h"

namespace fs = std::filesystem;

namespace {

struct Options {
    fs::path root;
    int port = 9001;
};

void print_usage(const char* argv0) {
    std::cerr << "Usage: " << argv0 << " --root <directory> [--port <port>]\n"
              << "\n"
              << "  --root  Directory to serve as the API root (required).\n"
              << "  --port  TCP port (default 9001).\n";
}

bool parse_args(int argc, char** argv, Options& out) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            return false;
        }
        if (arg == "--root" && i + 1 < argc) {
            out.root = argv[++i];
            continue;
        }
        if (arg == "--port" && i + 1 < argc) {
            out.port = std::atoi(argv[++i]);
            continue;
        }
        std::cerr << "Unknown or incomplete argument: " << arg << "\n";
        return false;
    }

    if (out.root.empty()) {
        std::cerr << "Error: --root is required\n";
        return false;
    }
    if (out.port <= 0 || out.port > 65535) {
        std::cerr << "Error: --port must be in 1..65535\n";
        return false;
    }
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    Options opts;
    if (!parse_args(argc, argv, opts)) {
        print_usage(argv[0]);
        return 2;
    }

    // Canonicalise the root once at startup: every later containment check
    // compares against this canonical path.
    std::error_code ec;
    const fs::path root = fs::canonical(opts.root, ec);
    if (ec) {
        std::cerr << "Error: cannot resolve root '" << opts.root.string() << "': " << ec.message()
                  << "\n";
        return 1;
    }
    if (!fs::is_directory(root, ec)) {
        std::cerr << "Error: root '" << root.string() << "' is not a directory\n";
        return 1;
    }

    httplib::Server server;
    fb::register_routes(server, root);

    std::cout << "file_server: root=" << root.string() << " port=" << opts.port << std::endl;

    // 0.0.0.0 rather than 127.0.0.1: otherwise the port is unreachable from
    // outside the container.
    if (!server.listen("0.0.0.0", opts.port)) {
        std::cerr << "Error: failed to bind 0.0.0.0:" << opts.port << "\n";
        return 1;
    }
    return 0;
}
