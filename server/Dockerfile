# ---------- build stage ----------
FROM debian:bookworm-slim AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        git \
        ca-certificates \
        libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

# CMakeLists is copied separately from the sources, so editing a .cpp file
# does not invalidate the cached layer holding the fetched dependencies.
COPY CMakeLists.txt ./
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release || true

COPY src ./src
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel "$(nproc)"

# ---------- runtime stage ----------
FROM debian:bookworm-slim AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
        libssl3 \
        curl \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

RUN useradd --system --create-home --uid 10001 appuser

COPY --from=build /src/build/file_server /usr/local/bin/file_server

USER appuser
EXPOSE 9001

ENTRYPOINT ["/usr/local/bin/file_server"]
CMD ["--root", "/data", "--port", "9001"]
