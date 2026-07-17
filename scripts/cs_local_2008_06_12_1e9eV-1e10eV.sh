#!/usr/bin/env bash
set -euo pipefail

# Optional Geant4 setup if Geant4_DIR is not already configured.
if [[ -n "${GEANT4_DIR:-}" && -f "${GEANT4_DIR}/geant4make/geant4make.sh" ]]; then
  # shellcheck disable=SC1090
  source "${GEANT4_DIR}/geant4make/geant4make.sh"
fi

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${REPO_ROOT}/build}"

cmake -S "${REPO_ROOT}" -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}" -j"$(nproc)"

cd "${BUILD_DIR}"
./run cs_local_2008_06_12_1e9eV-1e10eV.config.txt
