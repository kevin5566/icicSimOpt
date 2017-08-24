# icicSimOpt: A ICIC Solution by Optimization on RB Allocation Command
@ Kevin Cheng a.k.a. kevin5566, NTUEE WMNLab 2017

Different from previous project: [icicSim](https://github.com/kevin5566/icicSim/), *icicSimOpt* need only input scenario,and it will show optimal RB allocation command based on three different criteria.
*  Maximize overall throughput
*  All type users throughput achieve the similar level
*  Edge Throughput Guaranteed

## File Description
1. `main.cpp`: Main program to call all executing procedure.
1. `Def.h`: Define all parameters and function.
1. `Def.cpp`: Implement of all function.
1. `makefile`: Compile.
1. `input.txt`: Sample input.

## Compile
* Download ALL files include `main.cpp`, `Def.h`, `Def.cpp`, `makefile`, `input.txt`
* `make clean` to remove previous compile result
* `make` to generate executable file: `icicSimOpt`

## Execute
* Download sample input `input.txt` 
* `./icicSimOpt input.txt`
