#!/usr/bin/env bash
export OMP_PLACES=threads
export OMP_PROC_BIND=close
#export KMP_AFFINITY=verbose
unset KMP_AFFINITY
