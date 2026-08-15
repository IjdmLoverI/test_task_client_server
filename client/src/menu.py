"""Интерактивное меню навигации по дереву на сервере."""

from __future__ import annotations

import posixpath
from .api import ApiError, FileBrowserClient


def human_size(num: int) -> str:
    """1536 -> '1.5 KiB'. Косметика, но читать вывод сильно приятнее."""
    size = float(num)
    for unit in ("B", "KiB", "MiB", "GiB", "TiB"):
        if size < 1024 or unit == "TiB":
            return f"{size:.0f} {unit}" if unit == "B" else f"{size:.1f} {unit}"
        size /= 1024
    return f"{num} B"


class Browser:
    def __init__(self, client: FileBrowserClient) -> None:
        self.client = client
        self.path = "/"
        self.dirs: list[str] = []
        self.files: list[str] = []


    def refresh(self) -> None:
        data = self.client.list_dir(self.path)
        self.dirs = []
        self.files = []
        for entry in data["entries"]:
            if entry["type"] == "dir":
                self.dirs.append(entry["name"])
            else: 
                self.files.append(entry["name"])
            

    def render(self) -> None:
        print(f"\nТекущий каталог: {self.path}")

        print("\nКаталоги:")
        if not self.dirs:
            print("  (нет)")
        for i, name in enumerate(self.dirs, start=1):
            print(f"  {i}) {name}")

        print("\nФайлы:")
        if not self.files:
            print("  (нет)")
        for i, name in enumerate(self.files, start=1):
            print(f"  {i}) {name}")



    def enter_dir(self, number: int) -> None:
        if not (1 <= number <= len(self.dirs)):
            print(f"Нет каталога с номером {number}")
            return
        name = self.dirs[number - 1]
        self.path = posixpath.join(self.path, name)
        self.refresh()

    def go_up(self) -> None:
        if self.path == "/":
            print("Корневая папка")
            return
        self.path = posixpath.dirname(self.path)
        self.refresh()
        
    def show_file(self, number: int) -> None:
        if not (1 <= number <= len(self.files)):
            print("Out of range")
            return
        name = self.files[number -1]
        full_path = posixpath.join(self.path, name)
        info = self.client.file_info(full_path)
        print(f"\nФайл: {full_path}")
        print(f"  размер: {human_size(info['size'])} ({info['size']} байт)")
        print(f"  создан:   {info['created']}")
        print(f"  изменён:  {info['modified']}")
        print(f"  sha256:   {info['sha256']}")


    # ----------------------------------------------------------------------

    def run(self) -> None:
        while True:
            self.render()

            print("\n  1) enter directory")
            print("  2) show file info")
            print("  3) go up")
            print("  4) refresh")
            print("  0) quit")
            try:
                choice = input("\n> ").strip()

                if choice == "0":
                    return
                elif choice == "1":
                    number = int(input("directory number> ").strip())
                    self.enter_dir(number)
                elif choice == "2":
                    number = int(input("file number> ").strip())
                    self.show_file(number)
                elif choice == "3":
                    self.go_up()
                elif choice == "4":
                    self.refresh()
                else:
                    print("unknown command")
            except ValueError:
                print("a number is required")
            except ApiError as exc:
                print(f"error: {exc}")
            except (KeyboardInterrupt, EOFError):
                print()
                return