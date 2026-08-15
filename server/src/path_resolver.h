#pragma once

#include <filesystem>
#include <string>

namespace fb {

enum class ResolveStatus {
    Ok,
    InvalidRequest,  // path parameter missing or unusable  -> 400
    OutsideRoot,     // path escapes the served root        -> 403
    NotFound,        // path does not exist                 -> 404
};

struct ResolveResult {
    ResolveStatus status = ResolveStatus::InvalidRequest;
    std::filesystem::path absolute;  // only set when status == Ok
    std::string message;             // error text for the JSON response
};

// Turns a client-supplied path (for example "/docs/readme.txt") into an
// absolute path inside root, rejecting anything that points outside it.
ResolveResult resolve_under_root(const std::filesystem::path& root,
                                 const std::string& request_path);

// Maps a status onto its HTTP code.
int http_status_for(ResolveStatus status);

}  // namespace fb
