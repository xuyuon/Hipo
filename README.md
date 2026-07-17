# Hipo

Hipo is a Geant4-based atmospheric and magnetospheric particle transport simulation tool. It supports configurable magnetic-field models, atmospheric density models, and particle generation modes for batch simulation workflows.

## Publication scope
This repository is prepared as both:
- a **research artifact** (reproducible sample inputs, deterministic seed configuration, metadata output), and
- a **reusable open-source toolkit** (documented build/run flow, install/export support, CI workflows, governance files).

## Supported matrix (v1)
- OS: Linux (Ubuntu 22.04+)
- Compiler: GCC 11+ (tested with GCC 13)
- CMake: 3.16+
- C++ standard: C++17
- Geant4: 11.x (requires `Geant4Config.cmake` available to CMake)
- Thread model: single-thread and multi-thread (`nThreads >= 1`)

## Build
```bash
cmake -S . -B build -DGeant4_DIR=/path/to/geant4/lib/cmake/Geant4
cmake --build build -j
```

## Test
```bash
ctest --test-dir build --output-on-failure
```

## Quickstart (sample run)
```bash
cmake -S . -B build -DGeant4_DIR=/path/to/geant4/lib/cmake/Geant4
cmake --build build -j
cd build
./run cs_local_2008_06_12_1e9eV-1e10eV.config.txt
```

## CLI
```bash
./run <config-file>
./run --help
```

The executable validates CLI arguments and config schema before running.

## Config schema highlights
Required top-level keys include:
- `ProjectName`, `nThreads`, `batch_mode`, `OutputDir`, `Enable_interaction`, `Seed`
- `DensityModel`, `FieldModel`, `AtmosphereModel`, `GeneratorModel`
- `ParticleName`, `DetectingParticleName`

Sensitive layers must be defined by either:
- `nSensitiveLayer` with `R_sens_1 ... R_sens_N`, or
- single-layer shorthand `R_sens`.

Bundled sample configs in `/input` are portable and use relative paths.

## Output layout
Given `OutputDir=<dir>`, Hipo writes:
- `<dir>/primaries/` per-thread primaries csv files
- `<dir>/<DetectingParticleName>_<layer>/` per-thread secondaries csv files
- `<dir>/summary/` merged csv outputs
- `<dir>/runtime/<ProjectName>_runtime.txt` timing report
- `<dir>/runtime/<ProjectName>_metadata.txt` reproducibility metadata

## Reproducibility checklist (v1 publishable)
- [x] Builds on a clean Linux machine with Geant4 installed
- [x] Documented quickstart and sample configs
- [x] Deterministic seed in config and run metadata capture
- [x] Unit test entrypoint (`ctest`)
- [x] License and repository governance files
- [x] CI workflows for build/test/security/release packaging

## Install and package
Install targets:
```bash
cmake -S . -B build -DGeant4_DIR=/path/to/geant4/lib/cmake/Geant4
cmake --build build -j
cmake --install build --prefix /tmp/hipo-install
```

Source package:
```bash
cmake --build build --target package_source
```

## Troubleshooting
- **CMake cannot find Geant4**: set `Geant4_DIR` or `CMAKE_PREFIX_PATH` to your Geant4 installation.
- **Config validation failed**: check missing/unknown keys and model-specific required fields in your config.
- **No output files**: ensure `OutputDir` is writable and input file paths are correct relative to runtime directory.

## Versioning and releases
Hipo follows semantic versioning (`MAJOR.MINOR.PATCH`).
- Tag releases as `vX.Y.Z`
- Update `CHANGELOG.md` for each release
- CI release workflow packages source artifacts on tags

## Security
Please report vulnerabilities using the policy in `SECURITY.md`.
