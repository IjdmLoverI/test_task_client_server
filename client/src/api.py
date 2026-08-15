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
            raise ApiError(f"сервер недоступен: {exc}") from exc           

        if resp.status_code != 200:
            message = resp.json().get("error", f"HTTP {resp.status_code}")
            raise ApiError(message)
        return resp.json()

    def list_dir(self, path: str) -> dict:
        return self._get("/list", path)

    def file_info(self, path: str) -> dict:
        return self._get("/file", path)

    def wait_until_ready(self, attempts: int = 15, delay: float = 1.0) -> None:
        for _ in range(attempts):
            try:
                resp = requests.get(f"{self.base_url}/health", timeout=self.timeout)
                if resp.status_code == 200:
                    return
            except requests.RequestException:
                pass

            time.sleep(delay)
        raise ApiError(f"сервер {self.base_url} не отвечает после {attempts} попыток")
    