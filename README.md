# Parallel Verifier

Parallel Verifier is an optimized parallel verifier for graph based natural deduction proofs, with support for proofs
imported from [Hyperslate](http://www.logicamodernapproach.com/) and [Lazyslate](https://github.com/James-Oswald/lazyslate).

## Logic Support
Just natural deduction propositional calculus for now.

## Usage

#### Serial Verifier
```bash
    ./SerialVerifier [proofFilePath]
```

#### OpenMP Verifier
Set the environmental var to specify how many threads OMP can use, then run
the program with a path to a proof file.
```bash
    export OMP_NUM_THREADS=[numThreads]
    ./OMPVerifier [proofFilePath]
```

#### MPI Verifier
```bash
    mpirun -N [NumRanks] ./MPIVerifier [proofFilePath] [VerifierAlg] [LayerMapAlg] 
```
More on Args:
* VerifierAlg and LayerMapAlg are optional but if one is included, the other must be as well
* VerifierAlg values
  * Alpha - The Sp2022 version that uses markings and marking matrices. A layer mapper must be passed in, but it will not be used.
  * Original - a worse version that passes extra data rather than using an MPI reduce
  * NoOpt - non-optimized MPI implementation with no balancing
  * LoadBalance - load balancing optimizations implementation
* LayerMapAlg values
* OzSerial - Recursive DP O(n) layer mapping algo run on each rank
* MPI - MPI parallel D(n) layer mapping algo

## Project Layout
* `.github` : github actions for testing proofs
* `.vscode` : vscode recommended extensions and settings for the project
* `benchmarks` : quick benchmarking utilities to measure performance
* `proofs` : Proof files to verify on
  * `hyperslate` : hyperslate slt proofs
  * `lazyslate` : lazyslate json proofs
* `sandbox` : code to test features to implement 
* `scripts` : utility scripts for debugging, building, etc
* `src` : Source Code
* `tests` : CTest suite

## Dependencies
* [MPI](https://www.open-mpi.org/) (We use Open-MPI for local debugging and MPICH for benchmarks)
* [jsoncpp](https://github.com/open-source-parsers/jsoncpp)
* [OpenMP](https://www.openmp.org/)

## Notes
* Include Path Syntax as option 2 from https://stackoverflow.com/a/52144130/6342516.
