#pragma once

#include <cstdint>
#include <ctime>
#include <filesystem>
#include <string>
#include <vector>

namespace fb {

struct DirEntry {
    std::string name;
    std::string type;  // "file" | "dir"
};

struct FileInfo {
    std::uintmax_t size = 0;
    std::string created;   // ISO 8601 UTC, e.g. "2026-08-14T09:30:00Z"
    std::string modified;  // ISO 8601 UTC
    std::string sha256;    // 64 lowercase hex characters
};

// Contents of a directory, non-recursive.
std::vector<DirEntry> list_directory(const std::filesystem::path& dir);

// Metadata for a single file.
FileInfo describe_file(const std::filesystem::path& file);

// Streamed SHA-256 of a file.
std::string sha256_file(const std::filesystem::path& file);

// time_t -> ISO 8601 in UTC.
std::string to_iso8601_utc(std::time_t t);

}  // namespace fb
