#!/usr/bin/env bash
set -euo pipefail

repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source_directory="${repository_root}/PstechProject"
build_directory="${source_directory}/out/build/linux-x64"

cmake -S "${source_directory}" -B "${build_directory}" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build "${build_directory}" --parallel

library="${build_directory}/libPstechNative.so"
if [[ ! -f "${library}" ]]; then
  echo "Build completed but the expected library was not found: ${library}" >&2
  exit 1
fi

echo "Build succeeded: ${library}"
