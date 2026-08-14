#include "path_resolver.h"

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

    // ------------------------------------------------------------------
    // TODO 1. Отсечь пустой request_path.
    //   Считаем, что пустая строка и "/" означают корень — это удобно клиенту.
    //   Всё остальное пустое/мусорное -> ResolveStatus::InvalidRequest.
    // ------------------------------------------------------------------

    // ------------------------------------------------------------------
    // TODO 2. Склеить root с request_path.
    //   Осторожно: оператор fs::path::operator/ при АБСОЛЮТНОМ правом
    //   операнде ОТБРАСЫВАЕТ левый. То есть  root / "/etc/passwd"  даст
    //   "/etc/passwd", а не то, что ты ждёшь. Это дыра, а не мелочь.
    //   Значит: сначала сделать request_path относительным (снять ведущие '/'),
    //   и только потом склеивать.
    // ------------------------------------------------------------------

    // ------------------------------------------------------------------
    // TODO 3. Нормализовать полученный путь.
    //   fs::weakly_canonical() схлопывает "..", "." и разворачивает симлинки,
    //   и, в отличие от fs::canonical(), не бросает исключение на
    //   несуществующем пути. Не забудь про перегрузку с std::error_code —
    //   бросающая версия в обработчике HTTP-запроса тебе не нужна.
    // ------------------------------------------------------------------

    // ------------------------------------------------------------------
    // TODO 4. Главная проверка безопасности: убедиться, что результат
    //   действительно лежит ВНУТРИ канонического root.
    //
    //   Наивное сравнение строк через rfind(root_str, 0) == 0 — ЛОВУШКА:
    //   путь "/data-secret/x" пройдёт проверку на root "/data", потому что
    //   строка совпадает по префиксу. Правильный способ — сравнивать
    //   покомпонентно: пройтись итераторами по root и по кандидату и
    //   проверить, что все компоненты root совпали.
    //
    //   Не прошло -> ResolveStatus::OutsideRoot.
    //
    //   Проверь это в конце дня руками:
    //     curl -i "http://localhost:9001/list?path=../../etc"
    //     curl -i "http://localhost:9001/list?path=/../../etc"
    //   Оба должны дать 403, а не список /etc.
    // ------------------------------------------------------------------

    // ------------------------------------------------------------------
    // TODO 5. Проверить существование через fs::exists(..., ec).
    //   Нет -> ResolveStatus::NotFound. Есть -> заполнить result.absolute
    //   и выставить ResolveStatus::Ok.
    // ------------------------------------------------------------------

    (void)root;
    (void)request_path;
    result.status = ResolveStatus::InvalidRequest;
    result.message = "path_resolver is not implemented yet";
    return result;
}

}  // namespace fb
