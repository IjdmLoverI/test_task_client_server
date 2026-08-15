"""HTTP client for the file browser server.

The only module aware of HTTP. The menu works with plain dictionaries and
knows nothing about requests or status codes.
"""

from __future__ import annotations

import time

import requests


class ApiError(Exception):
    """The server is unreachable or answered with an error."""


class FileBrowserClient:
    def __init__(self, base_url: str, timeout: float = 5.0) -> None:
        self.base_url = base_url.rstrip("/")
        self.timeout = timeout

    def _get(self, endpoint: str, path: str) -> dict:

        url = self.base_url + endpoint

        try:
            resp = requests.get(url, params={"path": path}, timeout=self.timeout)
        except requests.RequestException as exc:
            # Only the exception type, not the full requests traceback: a CLI
            # user needs one readable line, the details stay in __cause__.
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
        # A short timeout specifically here: while waiting, the server either
        # answers almost instantly or is not listening at all, so a long
        # timeout only stretches out every failed attempt.
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
    