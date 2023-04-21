# Parallel Verifier

Parallel Verifier is an optimized parallel verifier for graph based natural deduction proofs, with support for proofs
imported from [Hyperslate](http://www.logicamodernapproach.com/) and [Lazyslate](https://github.com/James-Oswald/lazyslate).

## Logic Support
Just natural deduction propositional calculus for now.

## Usage

#### Serial Verifier
```
    ./SerialVerifier [proofFilePath]
```

#### Parallel Verifier
```
    Commands:
    ./ParallelVerifier [proofFilePath] [VerifierAlg] [LayerMapAlg]
    With MPI:
    mpirun -N [NumRanks] ./ParallelVerifier [proofFilePath] [VerifierAlg] [LayerMapAlg] 

    Args:
    VerifierAlg and LayerMapAlg are optional but if one is included,
    the other must be as well
    VerifierAlg values
        Alpha - The Sp2022 version that uses markings and marking matrices. A layer mapper must be passed in, but it will not be used.
        Original - a worse version that passes extra data rather than using an MPI reduce
        NoOpt - non-optimized MPI implementation with no balancing
        LoadBalance - load balancing optimizations implementation
    LayerMapAlg values
        OzSerial - Recursive DP O(n) layer mapping algo run on each rank
        MPI - MPI parallel D(n) layer mapping algo
```

## Dependencies

* [MPI](https://www.open-mpi.org/) (We use Open-MPI for local debugging and MPICH for benchmarks)
* [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

## Notes
* Include Path Syntax as option 2 from https://stackoverflow.com/a/52144130/6342516.
