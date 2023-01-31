
//Timings file provided by Christopher Carothers in his Parallel Computing Class

#pragma once

#define clockFreq 512000000

#ifndef __cplusplus
#include <stdio.h>
#include <stdint.h>
#else
#include <cstdio>
#include <cstdint>
extern "C" {
#endif // __cplusplus


//#ifdef __clang__
//AiMOS does not like this, for some reason __clang__ is defined for the xlrc compiler
/*static inline uint64_t clock_now() {
	return __builtin_readcyclecounter();
}*/
#if defined(__GNUC__)
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
#include <x86intrin.h>
static inline uint64_t clock_now() {
	return __rdtsc();
}
#else
uint64_t clock_now(void)
{
  unsigned int tbl, tbu0, tbu1;

  do {
    __asm__ __volatile__ ("mftbu %0" : "=r"(tbu0));
    __asm__ __volatile__ ("mftb %0" : "=r"(tbl));
    __asm__ __volatile__ ("mftbu %0" : "=r"(tbu1));
  } while (tbu0 != tbu1);

  return (((uint64_t)tbu0) << 32) | tbl;
}
#endif
#endif
  
#ifdef __cplusplus
}
#endif // __cplusplus

uint64_t startTime;

inline void startClock(){
    startTime = clock_now();
}

inline void endClock(){
    uint64_t endTime = clock_now();
    uint64_t totalCycles = endTime-startTime;
    double secs = totalCycles/(double)clockFreq;
    printf("%lf Seconds, %ld Clock Cycles\n", secs, totalCycles);
}

