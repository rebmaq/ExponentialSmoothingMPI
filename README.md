# ExponentialSmoothingMPI

A tool written in C to perform Exponential Smoothing on LAMMPS datasets

## Getting Started

This program uses OpenMPI to parallelize and distribute the workload, so make sure OpenMPI is installed and correctly configured for your system. First download and unzip the codebase or clone it using `git`.

### Compiling the Program
Before running `make`, take a look at the macros in `./include/file_handler.h`, defining the maximum number of input files, `MAX_FILES`, to the macro defining the number of digits in the total number of atoms, `MAX_ATOM_DIGITS`. Feel free to configure these macros to cater your input files. You will probably only have to change the `MAX_FILES` macro, to the number of files in your dataset directory.

Then compile the program by running `make` in the same directory as `Makefile`, `ExponentialSmoothingMPI`.

### Running the Program
Run the following command or run the shell script `run.sh`
```
mpirun -np number_of_processes ./bin/main dataset [time_step=1] [smoothing_factor=0.5]
```
- number_of_processes: the number of separate MPI processes that will be running the program
- bin/main: the name of the compiled program
- dataset: the directory containing the LAMMPS dataset
- time_step: the iteration step over the data files, e.g. time_step=1 iterates over all files, time_step=2 iterates over every other file, etc.(*Optional argument*, default value is 1)
- smoothing_factor: the smoothing factor used in performing exponential smoothing (*Optional argument*, default value is 0.5)