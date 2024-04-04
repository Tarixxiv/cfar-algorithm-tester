The goal of this project is to test and compare different variations of CFAR (Constant false alarm rate) algorithm in different environments. 

## CFAR

#### Overview:
CFAR is a detection algorithm, used in radars. 
CFAR calculates such a detection treshold that the false alarm rate is constant, regardless of noise. Each cell has its' own treshold value derived from ```training_cells``` amount of neighbouring cells, excluding ```guard_cells``` of closest ones. 

Guards cells are used due to object not being one single spike among the noise.

#### Implemented CFAR variations:
* CA - cell averaging 
* GOCA - greatest-of cell averaging 
* SOCA - smallest-of cell averaging


Names of these algorithms refers to system of choosing threshold.



## Project notes

* Project was first written in Python for simplicity. Later it was rewritten to C++ for notably better performance, more appropriate for algorithm that could be used in real-time systems.
