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
    std::string created;   // ISO 8601 UTC, напр. "2026-08-14T09:30:00Z"
    std::string modified;  // ISO 8601 UTC
    std::string sha256;    // 64 hex-символа в нижнем регистре
};

// Содержимое директории (без рекурсии).
std::vector<DirEntry> list_directory(const std::filesystem::path& dir);

// Метаданные одного файла.
FileInfo describe_file(const std::filesystem::path& file);

// Потоковый SHA-256 файла.
std::string sha256_file(const std::filesystem::path& file);

// time_t -> ISO 8601 в UTC.
std::string to_iso8601_utc(std::time_t t);

}  // namespace fb
