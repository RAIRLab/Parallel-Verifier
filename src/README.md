# Parallel Verifier
This folder contains the source code for the Parallel Verifier Project.
The source is split into a few sub-libraries which depend on each other
as well as some external libraries. 

A png dependency graph of these libraries can be generated with CMake's 
--graphviz flag and converting the dot file to a png.
```bash
cmake -B build --graphviz=build/ParallelVerifier.dot
dot -Tpng -o build/ParallelVerifier.png build/ParallelVerifier.dot
```

### Libs
* LibParallelVerifier - The API for MPI based parallel verifier
* Proof - Library containing class for representing and modifying graph based
proofs
* ProofIO - API for creating Proof data structures from Hyperslate and
Lazyslate proof files.
* SExpression - Class for representing and manipulating S-Expressions, which we
represent all of our formulae with.
* SharedVerifier - Utility API containing definitions and utility functions
used by both parallel verifier and serial verifier.

### Executables 
* ParallelVerifier.cpp - A command line wrapper for LibParallelVerifier 
(LibParallelVerifier does the actual verification)
* SerialVerifier.cpp - A command line program for serial verification 
(Not an API wrapper, does the actual verification) 