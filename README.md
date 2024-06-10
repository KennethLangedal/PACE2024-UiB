# PACE2024-UiB

Submission for PACE2024 by the algorithms group at UiB.

The exact and heuristic solvers use a reduction to directed feedback arc set (DFAS).
From there, the exact solver creates an incremental MaxSAT formulation by adding cycles lazily.
The heuristic makes use of several known heuristic techniques for the DFAS problem.

The exact solver is also used for the parameterized track.

## Dependencies

* UWrMaxSat (only for exact solver)

## Build

The **compile_dep.sh** script clones and builds the necessary requirements. After that, both our exact and heuristic solvers can be built using the included **make** file.

```
./compile_dep.sh
make
```