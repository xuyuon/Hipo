#!/bin/bash 
#SBATCH -N 1 -c 32
#SBATCH --mail-user=1155157363@link.cuhk.edu.hk

module purge
module load cmake
module load gcc
source /users/s1155157363/software/geant4/geant4-v11.3.2-install/share/Geant4/geant4make/geant4make.sh

module list

cd $SLURM_SUBMIT_DIR
cd build
ldd --version
./run bt_2020_06_12_1e12ev_1e13ev_1.config.txt