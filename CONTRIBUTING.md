# Contributing to Hipo

Thanks for contributing.

## Development setup
1. Install Geant4 11.x and expose `Geant4Config.cmake`.
2. Configure and build:
   ```bash
   cmake -S . -B build -DGeant4_DIR=/path/to/geant4/lib/cmake/Geant4
   cmake --build build -j
   ```
3. Run tests:
   ```bash
   ctest --test-dir build --output-on-failure
   ```

## Pull requests
- Keep changes focused and documented.
- Update tests and docs for behavior changes.
- Keep sample configs portable (no machine-specific absolute paths).

## Commit and release conventions
- Use concise, descriptive commit messages.
- Update `CHANGELOG.md` under `[Unreleased]`.
- Releases are tagged with semantic versions (`vX.Y.Z`).
