# Sparse Incomplete Cholesky Decomposition

Implementation of the Incomplete Cholesky Decomposition with few methods.  
The project includes a `C` implementation with a MATLAB MEX wrapper.

The aim is to have 3 variants of the incomplete decomposition:

 1.	Threshold (`IC(\tau)`)  
	Using a threshold, $ \tau $ to define which elements will be kept from the decomposition.  
	It can be using global threshold or by a column.  
	**Implemented**
 2.	Pattern (`IC(l)`)
	Filling elements which are up to `l` steps in the graph of the matrix `A`.
	For `l = 0` called *Zero Fill* where filling zeros in elements not defined by the pattern.  
	Also could be filled by a given pattern of sparsity (So given `A` as the pattern it matches `l = 0`).  
	**Not Implemented**
 3. Number of Non Zero Elements (`IC(p)`)  
	Keeps the largest `p` elements per column.  
	**Not Implemented**

## Generating MATLAB MEX

 1.	Download the repository.
 2. Run `MakeMex` in MATLAB with pre defined MATLAB MEX Compiler.
 3.	Go through the Unit Tests and the Run Time Analysis.
 
The MEX Wrapper supports only Sparse Real Matrices of Type Double.

## Performance

Comparing the performance with MATLAB's functions.

## Decomposition

![](https://i.imgur.com/zYKNq9o.png)

The MEX file and MATLAB's `ICT` were the most memory efficient.

## Pre Conditioning (Solving the Linear System)
 
![](https://i.imgur.com/iwoVKdM.png)

## To Do

 *	Move the array sorting related code to a dedicated repository with complete run time analysis.

## References

 * 	[PyMatting](https://github.com/pymatting/pymatting).  
	The `C` code is basically a redo of the Pre Conditioner in the Python package.
 *	[MATLAB `ichol()`](https://www.mathworks.com/help/matlab/ref/ichol.html).
 *	[Support Preconditioning Materials and Publications](https://www.tau.ac.il/~stoledo/Support/).
 *	[An Incomplete Cholesky Factorization for Dense Symmetric Positive Definite Matrices](https://link.springer.com/article/10.1023/A:1022323931043).
 *	[A Survey of Incomplete Factorization Preconditioners](https://www.cc.gatech.edu/~echow/pubs/pims_talk.pdf).
 *	[Experimental Study of ILU Preconditioners for Indefinite Matrices](https://www.cc.gatech.edu/~echow/pubs/stab.pdf).
 *	[A Robust Limited Memory Incomplete Cholesky Factorization](https://www.docdroid.net/HxEyRab).
 *	[Incomplete Cholesky Factorizations with Limited Memory](https://epubs.siam.org/doi/abs/10.1137/s1064827597327334).
 *	[Modified Incomplete Cholesky Factorization for Solving Electromagnetic Scattering Problems](http://www.jpier.org/PIERB/pier.php?paper=08112407).
 *	