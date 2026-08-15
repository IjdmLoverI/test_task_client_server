"""Client entry point.

Usage:
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
        description="CLI client for browsing files on the server.",
    )
    parser.add_argument(
        "--url",
        default=os.environ.get("SERVER_URL", DEFAULT_URL),
        help=f"server address (defaults to $SERVER_URL or {DEFAULT_URL})",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=5.0,
        help="request timeout in seconds",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    client = FileBrowserClient(args.url, timeout=args.timeout)

    print(f"Connecting to {args.url} ...")
    try:
        client.wait_until_ready()
    except ApiError as exc:
        print(f"Could not connect: {exc}", file=sys.stderr)
        return 1

    browser = Browser(client)
    try:
        browser.refresh()
    except ApiError as exc:
        print(f"Could not list the root directory: {exc}", file=sys.stderr)
        return 1

    browser.run()
    return 0


if __name__ == "__main__":
    sys.exit(main())
