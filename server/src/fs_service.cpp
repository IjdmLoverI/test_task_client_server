#include "fs_service.h"

#include <openssl/evp.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <fcntl.h>


namespace fs = std::filesystem;

namespace fb {

std::string to_iso8601_utc(std::time_t t) {

    struct tm tm_utc;
    char buf[32];

    gmtime_r(&t, &tm_utc);

    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm_utc);

    return buf;
}

std::vector<DirEntry> list_directory(const fs::path& dir) {
    std::vector<DirEntry> entries;
    std::error_code ec;

    for (const fs::directory_entry& entry : fs::directory_iterator(dir, ec)) {
        DirEntry item;
        item.name = entry.path().filename().string();
        item.type = entry.is_directory(ec) ? "dir" : "file";
        entries.push_back(item);
    }
    std::sort(
        entries.begin(), entries.end(), 
        [](const DirEntry& a, const DirEntry& b) {
            if (a.type != b.type) return a.type == "dir";
            return a.name < b.name;
        }
    );
    return entries;
}

std::string sha256_file(const fs::path& file) {


    std::ifstream in(file, std::ios::binary);
    if (!in) return "";
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (ctx == nullptr) return "";
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    
    char buf[65536];

    while(in.read(buf, sizeof(buf)) || in.gcount() > 0) {
        EVP_DigestUpdate(ctx, buf, in.gcount());
    }

    std::ostringstream oss;
    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int md_len = 0;

    EVP_DigestFinal_ex(ctx, md, &md_len);
    EVP_MD_CTX_free(ctx);

    for (unsigned int i = 0; i < md_len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(md[i]);
    }

    return oss.str();
}

FileInfo describe_file(const fs::path& file) {
    FileInfo info;


    struct stat st;
    struct statx stx;
    
    if (::stat(file.c_str(), &st) != 0) return info;

    info.size = st.st_size;
    info.modified = to_iso8601_utc(st.st_mtime);
    
    if (statx(AT_FDCWD, file.c_str(), 0, STATX_BTIME, &stx) == 0 && (stx.stx_mask & STATX_BTIME)) {
        info.created = to_iso8601_utc(stx.stx_btime.tv_sec);
    } else {
        info.created = to_iso8601_utc(st.st_ctime);
    }
    

    info.sha256 = sha256_file(file);

    return info;
}

}  // namespace fb
