"""Точка входа клиента.

Запуск:
    python -m src --url http://localhost:9001
    SERVER_URL=http://server:9001 python -m src
"""

from __future__ import annotations

import argparse
import os
import sys

from .api import ApiError, FileBrowserClient
from .menu import Browser

DEFAULT_URL = "http://localhost:9001"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="file-browser-client",
        description="CLI-клиент для просмотра файлов на сервере.",
    )
    parser.add_argument(
        "--url",
        default=os.environ.get("SERVER_URL", DEFAULT_URL),
        help=f"адрес сервера (по умолчанию {DEFAULT_URL} или $SERVER_URL)",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=5.0,
        help="таймаут запроса в секундах",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    client = FileBrowserClient(args.url, timeout=args.timeout)

    print(f"Подключение к {args.url} ...")
    try:
        client.wait_until_ready()
    except ApiError as exc:
        print(f"Не удалось подключиться: {exc}", file=sys.stderr)
        return 1

    browser = Browser(client)
    try:
        browser.refresh()
    except ApiError as exc:
        print(f"Не удалось получить список файлов: {exc}", file=sys.stderr)
        return 1

    browser.run()
    return 0


if __name__ == "__main__":
    sys.exit(main())
