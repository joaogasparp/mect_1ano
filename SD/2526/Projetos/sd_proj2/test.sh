#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
compose_file="$repo_root/docker/compose.yml"

cleanup() {
  local exit_code=$?

  if [[ -n "${gui_pid:-}" ]] && kill -0 "$gui_pid" 2>/dev/null; then
    kill "$gui_pid" 2>/dev/null || true
  fi

  docker compose -f "$compose_file" down >/dev/null 2>&1 || true
  exit "$exit_code"
}

wait_for_port() {
  local host="$1"
  local port="$2"
  local label="$3"
  local attempts=60

  while (( attempts > 0 )); do
    if (echo > "/dev/tcp/${host}/${port}") >/dev/null 2>&1; then
      return 0
    fi

    sleep 1
    attempts=$((attempts - 1))
  done

  echo "Timed out waiting for ${label} on ${host}:${port}" >&2
  return 1
}

trap cleanup EXIT INT TERM

cd "$repo_root"

echo "Starting backend stack..."
docker compose -f "$compose_file" up -d --build manager volume-0 volume-1 object

echo "Waiting for manager and object services..."
wait_for_port 127.0.0.1 4081 manager
wait_for_port 127.0.0.1 4281 object

echo "Running client smoke test..."
./run moss -r localhost:4281 list -b sd

echo "Launching GUI..."
./mvnw javafx:run &
gui_pid=$!
wait "$gui_pid"