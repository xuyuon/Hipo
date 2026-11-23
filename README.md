# Hipo

Hipo is a Geant4-based simulator for high-energy atmospheric and magnetospheric
particle transport. It supports:

- particle cascades through a tabulated atmosphere around a spherical Earth;
- particle backtracing through a modelled magnetosphere;
- configurable magnetic-field, density, particle-generation, and interaction
  models;
- single-threaded and multi-threaded batch execution; and
- CSV output for generated primaries and detected secondary particles.

The executable is called `run`. Simulations are configured with a plain-text
`key=value` file and are currently intended to run in batch mode.

## Requirements

- CMake 3.10 or newer;
- a C++17 compiler;
- Geant4 with UI and visualization drivers (`ui_all` and `vis_all`); and
- a network connection during the first configure step, because GoogleTest is
  downloaded with CMake `FetchContent`.

The input data files in `input/` include example magnetic-field tables,
atmospheric density profiles, and configuration files. Some production
configurations refer to absolute paths and must be adapted to the local
machine.

## Build

From the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

If Geant4 is not installed in a standard location, load its environment or
provide its CMake package location before configuring. For example:

```bash
source /path/to/geant4-install/bin/geant4.sh
cmake -S . -B build
```

The build copies files from `input/` into `build/`, so running the executable
from `build/` makes the supplied example paths convenient.

## Run a simulation

Run the executable with exactly one configuration-file argument:

```bash
cd build
./run bt_local_2008_06_12_1e9eV-1e10eV_1.config.txt
```

The cascade example can be run in the same way:

```bash
./run cs_local_2008_06_12_1e9eV-1e10eV.config.txt
```

Configuration and data paths are passed directly to the file readers and are
therefore resolved relative to the process working directory unless they are
absolute paths. The executable does not provide an interactive mode; set
`batch_mode=TRUE`.

For SLURM environments, the scripts in `scripts/` show the expected pattern:
load CMake, GCC, and Geant4, change to the build directory, and invoke `run`
with a configuration file.

## Configuration

Each configuration file contains one `key=value` entry per line. Values may
include Geant4 units such as `km`, `eV`, `degree`, and `radian`.

### Common settings

| Key | Description |
| --- | --- |
| `ProjectName` | Name used in output filenames. |
| `nThreads` | Number of Geant4 worker threads. Use `1` for serial execution. |
| `batch_mode` | Must be `TRUE` for a simulation run. |
| `OutputDir` | Root directory for generated output. |
| `nEvents` | Number of events for sampled generation. |
| `Seed` | Random-number-generator seed. |
| `Enable_interaction` | `TRUE` enables `FTFP_BERT`; `FALSE` uses magnetic transport without interaction physics. |
| `disable_geant4_verbose` | Suppresses most Geant4 output when `TRUE`. |

### Density models

Set `DensityModel` to `TabulatedDensity` or `None`.
`TabulatedDensity` requires:

```text
r_min=6371.2 km
r_pause=6471.2 km
r_max=6971.2 km
delta_r1=1.0 km
delta_r2=20.0 km
delta_theta=10.0 degree
delta_phi=20.0 degree
DensityProfileFile=msis2.0_2008_06_14.txt
```

### Magnetic-field models

Set `FieldModel` to one of:

- `IGRF`, which uses `IGRFDataFile`, `Year`, `Month`, `Day`, and
  `n_order_max`;
- `TA16`, which additionally uses the solar-wind and IMF parameters
  (`pdyn`, `symhc`, `xind`, `by_imf`, `ps`, `vxgse`, `vygse`, `vzgse`) and
  `TA16ParamFile`; or
- `NoField`.

### Atmosphere models

Set `AtmosphereModel` to `SphericalEarth` or `Magnetosphere`. Both require
`nSensitiveLayer` and one radius per sensitive layer:

```text
nSensitiveLayer=1
R_sens_1=6936.2 km
```

`SphericalEarth` also requires `R_earth`. `Magnetosphere` requires `R_inner`
and `R_outer`. `DetectingParticleName` selects the particle recorded when it
crosses a sensitive layer.

### Primary generators

`SphericalSurfaceSampling` generates new events from distributions. A typical
backtracing configuration uses:

```text
GeneratorModel=SphericalSurfaceSampling
ParticleName=anti_proton
EnergySpectrum=LogUniform
EnergySpectrum_min=1e9 eV
EnergySpectrum_max=1e10 eV
InitialPosition=UniformSphericalSurface
InitialPosition_mag=6471.2 km
InitialPosition_theta_min=0 degree
InitialPosition_theta_max=180 degree
InitialPosition_phi_min=0 degree
InitialPosition_phi_max=360 degree
NadirAngle=UpwardLambertian
AzimuthAngle=Uniform
AzimuthAngle_min=0.0 radian
AzimuthAngle_max=6.28318530718 radian
```

`GenerateFromTable` replays primary particles from a CSV file. Its header and
columns must be:

```text
EventID,Energy,Position_mag,Position_phi,Position_theta,Nadir_phi,Nadir_theta
```

Energy is in eV, position magnitude is in km, and angular values are in
radians. Set `PrimariesDataFile` to the CSV path. The generator indexes this
table by event ID, so a complete row is required for every event that is
generated. Table-driven event-count handling in the current run manager is
still incomplete; verify this path with the supplied integration tests before
using it for production runs.

## Output

The directory named by `OutputDir` is created automatically:

```text
<OutputDir>/
├── raw/
│   ├── primaries/
│   │   └── <ProjectName>_<thread>.csv
│   └── <DetectingParticleName>_<layer>/
│       └── <ProjectName>_<thread>.csv
├── runtime/
├── summary/
└── tracks/
```

At the end of a batch run, per-thread CSV files are merged into
`summary/<ProjectName>_primaries.csv` and one summary file per sensitive layer.
The primary CSV contains event energy, position, and nadir angles. Secondary
CSV files also record the production process and production coordinates.

## Tests

Configure and build the project, then run:

```bash
ctest --test-dir build --output-on-failure
```

The test suite includes unit tests for coordinates, density models, fields,
particle generation, sensitive detectors, and utilities, plus integration
tests for cascade and backtracing configurations.

## Troubleshooting

- **`Could not open data file`**: check the path in the configuration and the
  directory from which `./run` is being launched.
- **Unsupported model errors**: compare `DensityModel`, `FieldModel`,
  `AtmosphereModel`, and `GeneratorModel` with the model names documented
  above.
- **Missing Geant4 libraries or headers**: load the Geant4 environment before
  configuring and building.
- **Missing table rows or invalid event access**: ensure a
  `GenerateFromTable` CSV has one complete data row per generated event.

## License

Hipo is distributed under the MIT License. See [LICENSE](LICENSE).
