# File Browser

A client–server application for browsing files on a remote host.

The **server** is written in C++ and exposes a small RESTful JSON API over a
single directory tree chosen at startup. The **client** is a Python CLI that
connects to the server and lets you walk that tree from a numbered menu.
Both run in Docker and are started together with Docker Compose.

## Requirements

- Docker with Compose v2
- git, to obtain the sources

Nothing else. The C++ toolchain, CMake, OpenSSL and Python all live inside the
images — no local compiler or interpreter is needed.

## Quick start

Clone the repository and enter it:

```bash
git clone https://github.com/IjdmLoverI/test_task_client_server.git
cd test_task_client_server
```

Start the server:

```bash
docker compose up --build -d server
```

Then run the client:

```bash
docker compose run --rm client
```

Use `run`, not `up`, for the client. `docker compose up` does not attach a
usable terminal, so the interactive menu would immediately read EOF and exit.

To stop everything:

```bash
docker compose down
```

## Repository layout

```
.
├── docker-compose.yml
├── sample-data/            demo tree, mounted read-only into the server as /data
├── server/                 C++ server
│   ├── CMakeLists.txt
│   ├── Dockerfile          multi-stage: build image → slim runtime
│   └── src/
│       ├── main.cpp            argument parsing, startup
│       ├── path_resolver.*     maps a request path to a real path inside the root
│       ├── fs_service.*        directory listing, metadata, hashing
│       └── api.*               HTTP routes and JSON serialisation
└── client/                 Python CLI client
    ├── Dockerfile
    ├── requirements.txt
    └── src/
        ├── __main__.py         entry point and argument parsing
        ├── api.py              HTTP layer
        └── menu.py             interactive menu
```

## Server

### Building and running

Under Compose the server is built and started by the quick-start command above.
It listens on port `9001` and serves the directory given by `--root`.

To run the image directly:

```bash
docker run --rm -p 9001:9001 -v /some/directory:/data:ro file-browser-server --root /data
```

### Command-line options

| Option | Description |
|---|---|
| `--root <dir>` | Directory to serve. Required. |
| `--port <port>` | TCP port. Defaults to `9001`. |
| `--help` | Usage summary. |

The root is canonicalised once at startup and every request is checked against
that canonical path.

## API

All responses are `application/json`, including errors. Paths are given in the
`path` query parameter and are interpreted relative to the served root, so the
client never sees the real location on the host.

### `GET /list?path=<path>`

Lists the contents of a directory. Entries are sorted with directories first,
then by name, so a client can number them stably.

```bash
curl "http://localhost:9001/list?path=/"
```

```json
{
  "entries": [
    {
      "name": "docs",
      "type": "dir"
    },
    {
      "name": "empty-dir",
      "type": "dir"
    },
    {
      "name": "images",
      "type": "dir"
    },
    {
      "name": "readme-root.txt",
      "type": "file"
    }
  ],
  "path": "/"
}
```

`type` is `dir` for directories and `file` for everything else.

### `GET /file?path=<path>`

Returns metadata for a single regular file.

```bash
curl "http://localhost:9001/file?path=/docs/guide.md"
```

```json
{
  "created": "2026-08-15T14:43:57Z",
  "modified": "2026-08-15T14:43:57Z",
  "path": "/docs/guide.md",
  "sha256": "10121295e4282912a6b35658426766e07506c45333304b5a92ab31e1c2281189",
  "size": 56
}
```

`size` is a number of bytes. Timestamps are ISO 8601 in UTC, so they do not
depend on the container timezone. See *Known limitations* for what `created`
really means. `sha256` is the hash of the file contents in lowercase hex and
matches `sha256sum`.

### `GET /health`

Returns `{"status": "ok"}`. Not part of the task; it exists so that Compose can
gate the client on the server actually being ready.

### Errors

| Status | When |
|---|---|
| `400` | `path` parameter missing, or the object is the wrong type — a file for `/list`, a directory for `/file` |
| `403` | the resolved path lies outside the served root |
| `404` | the path does not exist |
| `500` | unexpected internal error |

Error bodies carry the same shape:

```bash
curl "http://localhost:9001/list?path=../../etc"
```

```json
{
  "error": "path is outside the served root",
  "status": 403
}
```

Messages describe what is wrong with the request without revealing the
container's filesystem layout.

## Client

Start it with `docker compose run --rm client`. It waits for the server to
become healthy, fetches the root listing and shows the menu.

Directories and files are numbered **separately**, so directory `2` and file `2`
are different entries.

```
Current directory: /

Directories:
  1) docs
  2) empty-dir
  3) images

Files:
  1) readme-root.txt

  1) enter directory
  2) show file info
  3) go up
  4) refresh
  0) quit

> 1
directory number> 1
```

Choosing `2` asks for a file number and prints its metadata:

```
File: /docs/guide.md
  size:      56 B (56 bytes)
  created:   2026-08-15T14:43:57Z
  modified:  2026-08-15T14:43:57Z
  sha256:    10121295e4282912a6b35658426766e07506c45333304b5a92ab31e1c2281189
```

Out-of-range numbers, non-numeric input, server errors and `Ctrl+C` are all
handled without ending the session unexpectedly.

### Running the client outside Compose

```bash
cd client
pip install -r requirements.txt
python -m src --url http://localhost:9001
```

The address can also be supplied through the `SERVER_URL` environment variable,
which is how the Compose service is configured.

## Design notes

**Dependencies via CMake FetchContent.** `cpp-httplib` and `nlohmann/json` are
fetched at configure time at pinned tags (`v0.18.3` and `v3.11.3`) rather than
vendored or taken from system packages, so the build is reproducible and the
repository stays free of third-party sources. Both are header-only, which keeps
linking trivial. A heavier framework such as Boost.Beast or Drogon would add
build time and complexity out of proportion to two endpoints.

**Path traversal protection.** A request path is made relative before being
joined to the root — `root / "/etc/passwd"` would otherwise discard the root
entirely — then normalised with `std::filesystem::weakly_canonical`, which
collapses `..` and resolves symlinks. Containment is then checked **component by
component** rather than by string prefix: a prefix comparison would accept
`/data-secret/x` under the root `/data`, since the strings do match.

**Streaming hashes.** SHA-256 is computed through the OpenSSL EVP interface in
64 KiB blocks, so a file is never read into memory in full and the server's
footprint does not depend on file size.

**Separate `stat` and `statx` calls.** One `stat()` provides both size and
modification time. Creation time needs `statx()`, which is requested separately
so that its absence degrades gracefully rather than failing the request.

**Multi-stage Docker build.** The compiler, CMake and OpenSSL headers stay in
the build stage; the runtime image carries only the binary and the shared
libraries it needs. Both services run as a non-root user.

**Client layering.** `api.py` is the only module aware of HTTP; `menu.py`
consumes plain dictionaries. Query parameters are passed through `requests`'
`params=` rather than being interpolated into the URL, so names containing
spaces or non-ASCII characters are encoded correctly.

**Explicit `path` parameter.** `/list` and `/file` reject a missing `path` with
`400` instead of silently defaulting to the root. The client always sends one,
and an explicit error is easier to diagnose than an implicit default.

## Known limitations

**`created` is not always a real creation time.** POSIX does not record one:
`st_atime`, `st_mtime` and `st_ctime` are access, modification and inode-change
times respectively — despite the name, `st_ctime` is *not* a creation time. The
server therefore calls `statx()` with `STATX_BTIME` and uses the result only
when the kernel actually reports that field back in `stx_mask`, since the call
can succeed without filling it in. When birth time is unavailable it falls back
to `st_ctime`, which is the closest available approximation but can differ
substantially.

This matters in practice. Measured inside this project: files stored on the
container's own overlayfs do report a birth time, while files on a bind-mounted
host directory do not. Since `docker-compose.yml` serves `sample-data` as a bind
mount, `created` will normally be the fallback value in the default setup.

**Symlinks are followed.** A symlink to a directory is reported as `dir`, and a
symlink pointing outside the served root is rejected with `403` because
`weakly_canonical` resolves it before the containment check.

**Hashes are computed per request.** `/file` re-reads and re-hashes the file
every time it is called. There is no cache, so repeated requests for a large
file cost proportionally.

**Listings are not paginated.** A directory with a very large number of entries
is returned in a single response.
