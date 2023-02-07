# Parallel Verifier

Parallel Verifier is an optimized parallel verifier for graph based natural deduction proofs, with support for proofs
imported from [Hyperslate](http://www.logicamodernapproach.com/) and [Lazyslate](https://github.com/James-Oswald/lazyslate).

## Logic Support
Just natural deduction propositional calculus for now.

## Dependencies

* [MPI](https://www.open-mpi.org/) (We use Open-MPI for local debugging and MPICH for benchmarks)
* [jsoncpp](https://github.com/open-source-parsers/jsoncpp)