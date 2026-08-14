#include "path_resolver.h"
#include <system_error>

namespace fs = std::filesystem;

namespace fb {

int http_status_for(ResolveStatus status) {
    switch (status) {
        case ResolveStatus::Ok:             return 200;
        case ResolveStatus::InvalidRequest: return 400;
        case ResolveStatus::OutsideRoot:    return 403;
        case ResolveStatus::NotFound:       return 404;
    }
    return 500;
}

ResolveResult resolve_under_root(const fs::path& root, const std::string& request_path) {
    ResolveResult result;

    fs::path requested(request_path);

    if (requested.is_absolute()) {
        requested = requested.relative_path();
    }
    
    fs::path combined = root / requested;


    std::error_code ec;

    fs::path canonical = fs::weakly_canonical(combined, ec);

    if (ec) {
        result.status = ResolveStatus::InvalidRequest;
        result.message = "invalid path";
        return result;
    }

    
    auto rit = root.begin();
    auto rend = root.end();
    auto cit = canonical.begin();
    auto cend = canonical.end();

    for (; rit != rend; ++rit, ++cit) {
        if (cit == cend || *cit != *rit) {
            result.status = ResolveStatus::OutsideRoot;
            result.message = "path is outside the served root";
            return result;
        }
    }

    if (!fs::exists(canonical, ec)) {
        result.status = ResolveStatus::NotFound;
        result.message = "path does not exist";
        return result;
    }
   
    result.absolute = canonical;
    result.status = ResolveStatus::Ok;
   

    return result;
}

}  // namespace fb
