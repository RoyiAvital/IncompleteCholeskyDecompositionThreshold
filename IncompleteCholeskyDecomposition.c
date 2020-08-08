/*
 * Incomplete Cholesky Decomposition
 * This module implements the Cholesky Decomposition. It uses upper boundary to the number of 
 * elements to prevent recurring allocations. It is implemented with the preconditioned conjugate
 * gradient in mind hence the preconditioning step is implemented as well.  
 *
 * References:
 *	1. 	https://github.com/pymatting/pymatting/blob/master/pymatting/preconditioner/ichol.py
 *	2.	Incomplete Cholesky Decomposition: https://en.wikipedia.org/wiki/Incomplete_Cholesky_factorization
 * Remarks:
 *	1.	The Sparse Matrices are given in Compressed Sparse Column (CSC) format.
 * TODO:
 *	1.	Add "Zero Fill" variant of the decomposition.
 * Release Notes:
 *	-	1.0.000	10/07/2020	Royi Avital
 *		*	First release version.
 */

#define ELEMENT_TYPE_IDX 1

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "ArraySorting.c"

#define LINKED_LIST_DEF_VAL UINT_MAX
#define FALSE ((unsigned char)(0))
#define TRUE ((unsigned char)(1))

int _CmpUnsignedInt(const void* a, const void* b)
{
	if (*(unsigned int*)a > *(unsigned int*)b)
	{
		return 1;
	}
	else if (*(unsigned int*)a < *(unsigned int*)b)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

void _SortArray( unsigned int* vA, unsigned int* vB, unsigned int numElements )
{
	memcpy(vA, vB, numElements * sizeof(unsigned int));
	qsort(vA, numElements, sizeof(unsigned int), _CmpUnsignedInt);
}

void _BackSubstitutionL( double * vX, double * vData, unsigned int * vIndices, unsigned int * vIndicesPtr, unsigned int numCols )
{
	double tmpVal, Ljj, Lij;
	unsigned int ii, jj, kk;

	for (jj = 0; jj < numCols; jj++)
	{
		kk = vIndicesPtr[jj];
		Ljj = vData[kk];
		tmpVal = vX[jj] / Ljj;

		vX[jj] = tmpVal;

		for (kk = vIndicesPtr[jj] + 1; kk < vIndicesPtr[jj + 1]; kk++)
		{
			ii = vIndices[kk];
			Lij = vData[kk];
			
			vX[ii] -= Lij * tmpVal;
		}
	}
}

void _BackSubstitutionLT( double * vX, double * vData, unsigned int * vIndices, unsigned int * vIndicesPtr, unsigned int numCols )
{
	double tmpVal, Lii, Lji;
	unsigned int ii, jj, kk;

	for (ii = numCols - 1; ii + 1 > 0; ii--)
	{
		tmpVal = vX[ii];

		for (kk = vIndicesPtr[ii] + 1; kk < vIndicesPtr[ii + 1]; kk++)
		{
			jj = vIndices[kk];
			Lji = vData[kk];

			tmpVal -= Lji * vX[jj];
		}

		kk = vIndicesPtr[ii];
		Lii = vData[kk];

		vX[ii] = tmpVal / Lii;
	}
}

int _IncompleteCholDec( double * vData, unsigned int * vIndices, unsigned int * vIndicesPtr, double * vDataA, unsigned int * vIndicesA, unsigned int * vIndicesPtrA, unsigned int numCols, double discardThr, double shiftVal, unsigned int maxNumNz )
{
	unsigned int numNz, c_n, *vS, *vT, *vL, *vC, * vCSort, ii, jj, idx, kk, k0, k1, k2;
	unsigned char* vB;
	double *vA, *vD, Lij, Lik, Ljk;

	numNz	= 0; // The current number of non zeros
	c_n		= 0; // Number of elements in a column

	vS = (unsigned int*)calloc(numCols, sizeof(unsigned int)); // Next non zero row index i in column j of L
	vT = (unsigned int*)calloc(numCols, sizeof(unsigned int)); // First sub diagonal index i in column j of A
	vL = (unsigned int*)malloc(numCols * sizeof(unsigned int)); // Linked list of non zero columns in row k of L
	memset(vL, 0xff, numCols * sizeof(unsigned int)); // The function `memset()` writes a single byte at a time (`0xff`)
	vA = (double*)calloc(numCols, sizeof(double)); // Values of column j
	vB = (unsigned char*)calloc(numCols, sizeof(unsigned char)); // b[i] indicates if the i-th element of column j is non zero
	vC = (unsigned int*)malloc(numCols * sizeof(unsigned int)); // Row indices of non zero elements in column j
	vCSort = (unsigned int*)malloc(numCols * sizeof(unsigned int)); // Sorting buffer for vC
	vD = (double*)malloc(numCols * sizeof(double)); // Diagonal elements of mA (Of the decomposition: shiftVal + diag(mA))
	//for (ii = 0; ii < numCols; ii++)
	//{
	//	vD[ii] = shiftVal;
	//}

	for (jj = 0; jj < numCols; jj++)
	{
		for (idx = vIndicesPtrA[jj]; idx < vIndicesPtrA[jj + 1]; idx++)
		{
			ii = vIndicesA[idx];
			if (ii == jj)
			{
				vD[jj] = shiftVal + vDataA[idx];
				vT[jj] = idx + 1;
			}
		}
	}

	for (jj = 0; jj < numCols; jj++) // For each column j
	{
		for (idx = vT[jj]; idx < vIndicesPtrA[jj + 1]; idx++) // For each L_ij
		{
			ii = vIndicesA[idx];
			Lij = vDataA[idx];
			if (Lij != 0.0 && ii > jj)
			{
				vA[ii] += Lij; // Assign non zero value to L_ij in sparse column
				if (vB[ii] == FALSE)
				{
					vB[ii] = TRUE; // Mark it as non zero
					vC[c_n] = ii; // Remember index for later deletion
					c_n++;
				}
			}
		}
		kk = vL[jj]; // Find index k of column with non zero element in row j

		while (kk != LINKED_LIST_DEF_VAL) // For each column of that type
		{
			k0 = vS[kk]; // Start index of non zero elements in column k
			k1 = vIndicesPtr[kk + 1]; // End index
			k2 = vL[kk]; // Remember next column index before it is overwritten

			Ljk = vData[k0]; // Value of non zero element at start of column
			k0++; // Advance to next non zero element in column

			if (k0 < k1) // If there is a next non zero element
			{
				vS[kk] = k0; // Advance start index in column k to next non zero element
				ii = vIndices[k0]; // Row index of next non zero element in column k
				vL[kk] = vL[ii]; // Remember old list i index in list k
				vL[ii] = kk; // Insert index of non zero element into list i

				for (idx = k0; idx < k1; idx++) // For each non zero L_ik in column k
				{
					ii = vIndices[idx];
					Lik = vData[idx];
					vA[ii] -= Lik * Ljk; // Update element L_ij in sparse column
					if (vB[ii] == FALSE) // Check if sparse column element was zero
					{
						vB[ii] = TRUE; // Mark as non zero in sparse column
						vC[c_n] = ii; // Remember index for later deletion
						c_n++;
					}
				}
			}
			kk = k2; // Advance to next column k
		}
		if (vD[jj] <= 0.0)
		{
			return -1;
		}
		if (numNz + 1 + c_n > maxNumNz)
		{
			return -2;
		}

		vD[jj] = sqrt(vD[jj]); // Update diagonal element L_ii
		vData[numNz] = vD[jj]; // Add diagonal element L_ii to L
		vIndices[numNz] = jj; // Add row index of L_ii to L
		numNz++;
		vS[jj] = numNz; // Set first non zero index of column j

		if (c_n > 0) // Only if there are actually indices to handle
		{
			//_SortArray(vCSort, vC, c_n); // Sort row indices of column j for correct insertion order into L
			memcpy(vCSort, vC, c_n * sizeof(unsigned int));
			ArrayQuickSort(vCSort, (unsigned int)0, c_n - 1);

			for (idx = 0; idx < c_n; idx++)
			{
				ii = vCSort[idx];
				Lij = vA[ii] / vD[jj]; // Get non zero element from sparse column j
				vD[ii] -= Lij * Lij; // Update diagonal element L_ii
				if (fabs(Lij) > discardThr) // If element is sufficiently non zero (Above threshold)
				{
					vData[numNz] = Lij; // Add element L_ij to L
					vIndices[numNz] = ii; // Add row index of L_ij
					numNz++;
				}
				vA[ii] = 0.0; // Set element i in column j to zero
				vB[ii] = FALSE; // Mark element as zero
			}
		}
		
		c_n = 0; // Discard row indices of non zero elements in column j
		vIndicesPtr[jj + 1] = numNz; // Update count of non zero elements up to column j
		if (vIndicesPtr[jj] + 1 < vIndicesPtr[jj + 1]) // If column j has a non zero element below diagonal
		{
			ii = vIndices[vIndicesPtr[jj] + 1]; // Row index of first off-diagonal non zero element
			vL[jj] = vL[ii]; // Remember old list i index in list j
			vL[ii] = jj; // Insert index of non zero element into list i
		}
	}

	free(vS);
	free(vT);
	free(vL);
	free(vA);
	free(vB);
	free(vC);
	free(vCSort);
	free(vD);

	return numNz;

}

void IncompleteCholeskyDecomposition( double * vData, unsigned int * vIndices, unsigned int * vIndicesPtr, double * vDataA, unsigned int * vIndicesA, unsigned int * vIndicesPtrA, unsigned int numCols, double discardThr, double* vShifts, unsigned int numShifts, unsigned int maxNumNz )
{
	unsigned int ii, numNz;
	double shiftVal;

	for (ii = 0; ii < numShifts; ii++)
	{
		shiftVal = vShifts[ii];
		numNz = _IncompleteCholDec(vData, vIndices, vIndicesPtr, vDataA, vIndicesA, vIndicesPtrA, numCols, discardThr, shiftVal, maxNumNz);
	}

}