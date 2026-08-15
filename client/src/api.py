"""HTTP-клиент к серверу file browser.

Единственный модуль, который знает про HTTP. Меню работает уже
со словарями и ничего не знает ни про requests, ни про коды ответов.
"""

from __future__ import annotations

import time

import requests


class ApiError(Exception):
    """Сервер недоступен или вернул ошибку."""


class FileBrowserClient:
    def __init__(self, base_url: str, timeout: float = 5.0) -> None:
        self.base_url = base_url.rstrip("/")
        self.timeout = timeout

    def _get(self, endpoint: str, path: str) -> dict:

        url = self.base_url + endpoint

        try:
            resp = requests.get(url, params={"path": path}, timeout=self.timeout)
        except requests.RequestException as exc:
            # Только тип ошибки, без внутреннего трейса requests: пользователю
            # CLI нужна одна внятная строка, подробности остаются в __cause__.
            raise ApiError(f"cannot reach {self.base_url} ({type(exc).__name__})") from exc

        if resp.status_code != 200:
            try:
                message = resp.json().get("error", f"HTTP {resp.status_code}")
            except ValueError:
                message = f"HTTP {resp.status_code}"
            raise ApiError(message)

        return resp.json()

    def list_dir(self, path: str) -> dict:
        return self._get("/list", path)

    def file_info(self, path: str) -> dict:
        return self._get("/file", path)

    def wait_until_ready(self, attempts: int = 15, delay: float = 1.0) -> None:
        # Короткий таймаут именно здесь: на этапе ожидания сервер либо
        # отвечает почти мгновенно, либо не слушает вовсе, и длинный
        # таймаут только растягивает каждую неудачную попытку.
        probe_timeout = min(self.timeout, 1.0)

        for _ in range(attempts):
            try:
                resp = requests.get(f"{self.base_url}/health", timeout=probe_timeout)
                if resp.status_code == 200:
                    return
            except requests.RequestException:
                pass

            time.sleep(delay)

        raise ApiError(f"{self.base_url} did not respond after {attempts} attempts")
    