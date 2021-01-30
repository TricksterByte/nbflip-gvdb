#pragma once
#ifndef UNCONDITIONED_CG_SOLVER_H_
#define UNCONDITIONED_CG_SOLVER_H_

#include "sparse_matrix.h"
#include "array3d.h"
#include <cmath>

#define AIRCELL 0
#define FLUIDCELL 1
#define SOLIDCELL 2

struct Uncondioned_CG_Solver
{
	VectorN d; // Search vector
	VectorN z;
	VectorN r;
	VectorN Adj;
	double beta, alpha;
		
	Uncondioned_CG_Solver();
	Uncondioned_CG_Solver(int dimx, int dimy, int dimz);

	void init(int dimx, int dimy, int dimz);

	void apply_precond(const Sparse_Matrix & A, const Sparse_Matrix & precond,const VectorN &r, VectorN &z,const Array3c & marker);
	void solve(const Sparse_Matrix & A,const VectorN & b,int maxiterations, double tol, VectorN & x, Array3c & marker);
	void solve_precond(const Sparse_Matrix & A,const VectorN & b,const Sparse_Matrix & precond,int maxiterations, double tol, VectorN & pressure, Array3c &marker);
};

#endif
