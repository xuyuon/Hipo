# Hipo

> [!WARNING]
> Hipo is still under heavy development. Use at your own risk!

Hipo is a Geant4-based simulator for high-energy particle in atmosphere. It supports:

- Simulating cosmic ray (CR) cascade in the atmosphere
- Simulating geomagnetic shielding effect on CR particles


## Requirements

- CMake 3.10 or newer
- a C++17 compiler
- [Geant4](https://github.com/Geant4/geant4) (tested on version 11.3.2)

The input data files in `input/` include example magnetic-field tables,
atmospheric density profiles, and configuration files.

## Build

From the repository root, create a `build` folder, and inside it compile
the code:

```bash
mkdir build
cd build
cmake ..
make
```

If Geant4 is not installed in a standard location, load its environment or
provide its CMake package location before configuring. For example:

```bash
source /path/to/geant4-install/bin/geant4.sh
```

The build copies files from `input/` into `build/` automatically.

## Run a simulation

Compiling the code will give an executable `run`. Run the executable with exactly
one configuration file argument. For example, to run the executable with the
configurations in `example_bt_2008_06_12.txt`:

```bash
cd build
./run example_bt_2008_06_12.txt
```

## Configuration

Each configuration file contains one `key=value` entry per line. Values may
include Geant4 units such as `km`, `eV`, `degree`, and `radian`. A blank space
must be inserted between the value and the Geant4 units.

### Common settings

A value must be provided to each of the following keys:
| Key | Description |
| --- | --- |
| `ProjectName` | Project name, used to name the output filenames. |
| `nThreads` | Number of Geant4 worker threads. |
| `batch_mode` | Must be `TRUE` for a simulation run. |
| `OutputDir` | Root directory for generated output. |
| `nEvents` | Number of particles injected. |
| `Seed` | Random-number-generator seed. |
| `Enable_interaction` | `TRUE` enables `FTFP_BERT` physics list; `FALSE` keeps magnetic transport but disables interaction physics. |
| `disable_geant4_verbose` | Suppresses most Geant4 output when `TRUE`. |


### Atmosphere models

Set `AtmosphereModel` to `SphericalEarth` or `Magnetosphere`.

`SphericalEarth` builds an atmosphere around the Earth. To use it, set:

```text
AtmosphereModel=SphericalEarth
```

It takes the following arguments:
| Key | Description |
| --- | --- |
| `nSensitiveLayer` | Number of sensitive layer for recording particle hit. |
| `R_sens_1` | The radius of sensitive layer (set also `R_sens_2`, `R_sens_3`, ... if you set `nSensitiveLayer`>1). |
| `R_earth` | Radius of the Earth. |
| `DetectingParticleName` | The type of particle to record when it crosses a sensitive layer (set "gamma" for photon). |


`Magnetosphere` does not build atmosphere around the Earth. To use it, set:

```text
AtmosphereModel=Magnetosphere
```

It takes the following arguments:
| Key | Description |
| --- | --- |
| `nSensitiveLayer` | Number of sensitive layer for recording particle hit. |
| `R_sens_1` | The radius of sensitive layer (set also `R_sens_2`, `R_sens_3`, ... if you set `nSensitiveLayer`>1). |
| `R_inner` | Inner radius of the volume. |
| `R_outer` | Outer radius of the volume. |
| `DetectingParticleName` | The type of particle to record when it crosses a sensitive layer (set "gamma" for photon). |

`R_outer` is the escape sphere: primaries whose back-traced trajectories leave this
sphere are discarded. A typical back-tracing run uses `R_outer` of about 10 Earth
radii.

### Density models

Currently we only support `TabulatedDensity` or `None`.


`TabulatedDensity` is paired with `AtmosphereModel=SphericalEarth`.
To use it, set:

```text
DensityModel=TabulatedDensity
```

It requires:

| Key | Description |
| --- | --- |
| `r_min` | Inner radius of the density grid, i.e. the lowest altitude at which the atmosphere is built. |
| `r_pause` | Radius at which the radial spacing changes from `delta_r1` (below) to `delta_r2` (above). |
| `r_max` | Outer radius of the density grid, i.e. the top of the atmosphere. |
| `delta_r1` | Radial spacing of the grid below `r_pause`. |
| `delta_r2` | Radial spacing of the grid above `r_pause`. |
| `delta_theta` | Spacing in polar angle of the grid. |
| `delta_phi` | Spacing in azimuth of the grid. |
| `DensityProfileFile` | Path to the tabulated atmospheric density profile. |

A 100 km atmosphere resolved with 0.5 km shells corresponds to `r_min` = 6371.2 km,
`r_max` = 7371.2 km, `delta_r1` = `delta_r2` = 0.5 km; an 18 × 18 grid with equal
latitude and longitude intervals corresponds to `delta_theta` = 10 degree and
`delta_phi` = 20 degree. Finer `delta_r1` in the lower atmosphere resolves the
region where the cascade develops; `delta_r2` may be coarser above it.

`None` is paired with `AtmosphereModel=Magnetosphere`, you only need to set:
```text
DensityModel=None
```

### Magnetic-field models

Set `FieldModel` to one of:

- `IGRF`;
- `TA16`;
- `NoField`.

`IGRF` takes the following argument:
| Key | Description |
| --- | --- |
| `IGRFDataFile` | Path to the IGRF coefficient table. |
| `Year` | Year. |
| `Month` | Month. |
| `Day` | Day. |
| `n_order_max` | Maximum degree of the spherical-harmonic expansion. IGRF-14 is defined up to degree 13; a smaller value truncates the field to a smoother one. |

`TA16` will build an combined field, which has two components: An internal field modeled by
the IGRF model, and an external field modeled by the TA16 model. It takes the following argument:
| Key | Description |
| --- | --- |
| `IGRFDataFile` | Path to the IGRF coefficient table. |
| `Year` | Year. |
| `Month` | Month. |
| `Day` | Day. |
| `n_order_max` | Maximum degree of the IGRF expansion. |
| `pdyn` | solar wind dynamic pressure $[\textrm{nPa}]$. |
| `symhc` | average corrected $\textrm{SYM-H}$ index. |
| `xind` | `N-index`. |
| `by_imf` | $y$-component of IMF magnetic field in GSW coordinates. |
| `ps` | dipole tilt angle, in radian. |
| `vxgse` | $x$-component of solar wind speed in GSE coordinates $[\textrm{km}/\textrm{s}]$. |
| `vygse` | $y$-component of solar wind speed in GSE coordinates $[\textrm{km}/\textrm{s}]$. |
| `vzgse` | $z$-component of solar wind speed in GSE coordinates $[\textrm{km}/\textrm{s}]$. |

See the [TA16](https://geo.phys.spbu.ru/~tsyganenko/empirical-models/magnetic_field/ta16) model webpage for
more information. TA16 is validated for `x_GSM >= -15 R_E`; results beyond
that region should be treated with caution.

`NoField` disables the magnetic field entirely and takes no arguments.

### Primary generators

`SphericalSurfaceSampling` generates CR primaries on a spherical surface from
distributions. A typical back-tracing configuration uses:

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

`UpwardLambertian` samples the nadir angle with a cosine-weighted distribution, so
that the injected flux is isotropic across the surface. The sampled distributions
must match the injection PDF used when the flux is later normalised; changing
`EnergySpectrum` or `NadirAngle` without updating that normalisation biases the
result.

`GenerateFromTable` replays primary particles from a CSV file. Its header and
columns must be:

```text
EventID,Energy,Position_mag,Position_phi,Position_theta,Nadir_phi,Nadir_theta
```

Energy is in eV, position magnitude is in km, and angular values are in radians.
Set `PrimariesDataFile` to the CSV path.

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
ctest --output-on-failure
```

This run the unit tests and the integration tests.


## Attribution

If you use Hipo in your research, please cite the accompanying paper:

```bibtex
@article{

}
```

and the Geant4 paper:
```bibtex
@article{Allison_2016,
title = {Recent developments in Geant4},
journal = {Nuclear Instruments and Methods in Physics Research Section A: Accelerators, Spectrometers, Detectors and Associated Equipment},
volume = {835},
pages = {186-225},
year = {2016},
issn = {0168-9002},
doi = {https://doi.org/10.1016/j.nima.2016.06.125},
author = {J. Allison and others},
keywords = {High energy physics, Nuclear physics, Radiation, Simulation, Computing},
}
```

If you use IGRF model, please cite the IGRF paper:
```bibtex
@article{Beggan_2026,
  author  = {Beggan, C.D. and Kloss, C. and Amblard, P. and others},
  title   = {International geomagnetic reference field: the fourteenth generation},
  journal = {Earth, Planets and Space},
  volume  = {78},
  number  = {1},
  pages   = {127},
  year    = {2026},
  month   = jun,
  doi     = {10.1186/s40623-025-02360-0},
  issn    = {1880-5981},
  url     = {https://doi.org/10.1186/s40623-025-02360-0}
}
```

If you use TA16 model, please cite the IGRF paper together with the two TA16 papers:
```bibtex
@article{Andreeva_2016,
author = {Tsyganenko, N. A. and Andreeva, V. A.},
title = {An empirical RBF model of the magnetosphere parameterized by interplanetary and ground-based drivers},
journal = {Journal of Geophysical Research: Space Physics},
volume = {121},
number = {11},
pages = {10,786-10,802},
keywords = {magnetosphere, modeling, geomagnetic field, radial basis functions},
doi = {https://doi.org/10.1002/2016JA023217},
year = {2016}
}
```

```bibtex
@article{Andreeva_2016_b,
author = {Andreeva, Varvara A. and Tsyganenko, Nikolai A.},
title = {Reconstructing the magnetosphere from data using radial basis functions},
journal = {Journal of Geophysical Research: Space Physics},
volume = {121},
number = {3},
pages = {2249-2263},
keywords = {magnetospheric modeling, current systems, polar cusps},
doi = {https://doi.org/10.1002/2015JA022242},
url = {https://agupubs.onlinelibrary.wiley.com/doi/abs/10.1002/2015JA022242},
year = {2016}
}
```

## License

Hipo is distributed under the MIT License. See [LICENSE](LICENSE).
