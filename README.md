# Radpharm

Radpharm is a Geant4-based Monte Carlo simulation project for studying and optimizing radioisotope production processes using particle accelerators.

Developed by **Lê Tuấn Anh**.

## Overview

This project focuses on:

- Radioisotope production simulation
- Accelerator target optimization
- Nuclear reaction and particle transport modeling
- Yield estimation and isotope inventory analysis
- Automated rerun workflows for parameter studies

Radpharm is designed for research applications in nuclear medicine, accelerator physics, and radiopharmaceutical production.

## Key Features

- Geant4-based particle transport simulation
- Flexible geometry and material configuration
- Support for liquid and solid target studies
- Isotope production tally extraction
- Batch simulation and optimization workflow
- Export tools for data analysis

## Requirements

- Geant4
- CMake
- C++17 compatible compiler
- ROOT (optional)

## Build Instructions

```bash
git clone https://github.com/AnhLe263/Radpharm.git

cd Radpharm

mkdir build
cd build

cmake ..
make -j$(nproc)
```

## Run Example

```bash
./Radpharm run.mac
```

## Project Structure

```text
include/    Header files
src/        Source files
macros/      analysis scripts
build/      Build directory
```

## Author

**Lê Tuấn Anh**  
Monte Carlo Simulation and Accelerator Applications

## License

This project is intended for academic and research purposes.
