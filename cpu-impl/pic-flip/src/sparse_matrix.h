#pragma once
#ifndef SPARSE_MATRIX_H_
#define SPARSE_MATRIX_H_

#define AIRCELL 0
#define FLUIDCELL 1
#define SOLIDCELL 2

#include "array3d.h"

struct VectorN
{
	VectorN();
	VectorN(int dimx_, int dimy_, int dimz_);
	void init(int dimx_, int dimy_, int dimz_);
	~VectorN();

	double infnorm() const;
	void copy_to(VectorN &vec) const;

	double &operator()(int i, int j, int k);
	const double &operator()(int i, int j, int k) const;
	void zero();

	int size, dimx, dimy, dimz;
	double *data;
};

struct Sparse_Matrix
{
	Sparse_Matrix();
	Sparse_Matrix(int dimx, int dimy, int dimz);
	void init(int dimx_, int dimy_, int dimz_);
	~Sparse_Matrix();

	double &operator()(int i, int j, int k, int offset);
	const double &operator()(int i, int j, int k, int offset) const;
	void zero();

	int size, dimx, dimy, dimz;
	int stride_y, stride_z;
	double *data;
};

void mtx_mult_vectorN(const Sparse_Matrix &A, const VectorN &d, VectorN &Adj, Array3c &marker);
void vectorN_add(VectorN &lhs, const VectorN &rhs);
void vectorN_add_scale(VectorN &lhs, const VectorN &rhs, double scale);
void vectorN_scale_add(VectorN &d, const VectorN &r, double beta);
void vectorN_sub_scale(VectorN &r, const VectorN &Adj, double alpha);
double vectorN_dot(const VectorN &lhs, const VectorN &rhs);
double vectorN_norm2(const VectorN &lhs);

#endif
