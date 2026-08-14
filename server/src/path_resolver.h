#pragma once

#include <filesystem>
#include <string>

namespace fb {

enum class ResolveStatus {
    Ok,
    InvalidRequest,  // параметр path отсутствует или некорректен -> 400
    OutsideRoot,     // путь уводит за пределы корня            -> 403
    NotFound,        // такого пути не существует               -> 404
};

struct ResolveResult {
    ResolveStatus status = ResolveStatus::InvalidRequest;
    std::filesystem::path absolute;  // заполнено только при status == Ok
    std::string message;             // текст ошибки для JSON-ответа
};

// Превращает пришедший от клиента path (например "/docs/readme.txt")
// в абсолютный путь внутри root, отказывая во всём, что выходит наружу.
ResolveResult resolve_under_root(const std::filesystem::path& root,
                                 const std::string& request_path);

// Отображение статуса на HTTP-код.
int http_status_for(ResolveStatus status);

}  // namespace fb
