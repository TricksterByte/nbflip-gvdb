#include "sparse_matrix.h"
#include "array3d.h"

#include <cmath>
#include <cstring>

VectorN::VectorN() : size(0) {}

VectorN::VectorN(int dimx_, int dimy_, int dimz_)
{
	init(dimx_, dimy_, dimz_);
}

void VectorN::init(int dimx_, int dimy_, int dimz_)
{
	size = dimx_ * dimy_ * dimz_;
	dimx = dimx_;
	dimy = dimy_;
	dimz = dimz_;
	data = new double[size];
	zero();
}

double VectorN::infnorm() const
{
	double r = 0;
	for (int i = 0; i < size; ++i)
		if (!(std::fabs(data[i]) <= r))
			r = std::fabs(data[i]);
	return r;
}

void VectorN::copy_to(VectorN &vec) const
{
	std::memcpy(vec.data, data, size * sizeof(double));
}

VectorN::~VectorN()
{
	delete[]data;
}

double &VectorN::operator()(int i, int j, int k)
{
	return data[i + dimx * (j + dimy * k)];
}

const double &VectorN::operator()(int i, int j, int k) const
{
	return data[i + dimx * (j + dimy * k)];
}

void VectorN::zero()
{
	std::memset(data, 0, size * sizeof(double));
}

Sparse_Matrix::Sparse_Matrix() : data(NULL)
{

}

Sparse_Matrix::Sparse_Matrix(int dimx, int dimy, int dimz)
{
	init(dimx, dimy, dimz);
}

void Sparse_Matrix::init(int dimx_, int dimy_, int dimz_)
{
	dimx = dimx_; dimy = dimy_; dimz = dimz_;
	size = 4 * dimx * dimy * dimz;
	stride_y = 4 * dimx;
	stride_z = stride_y * dimy;
	data = new double[size];
	zero();
}

Sparse_Matrix::~Sparse_Matrix()
{
	delete[]data;
}

double &Sparse_Matrix::operator()(int i, int j, int k, int offset)
{
	return data[i * 4 + j * stride_y + k * stride_z + offset];
}

const double &Sparse_Matrix::operator()(int i, int j, int k, int offset) const
{
	return data[i * 4 + j * stride_y + k * stride_z + offset];
}

void Sparse_Matrix::zero()
{
	std::memset(data, 0, size * sizeof(double));
}

void mtx_mult_vectorN(const Sparse_Matrix &A, const VectorN &d, VectorN &Adj, Array3c &marker)
{
	// All boundary cells are SOLIDS
	// Thus the boundary cell rows in the Poisson matrix are ZERO: No need to operate on them
	Adj.zero();
	for (int k = 1; k < A.dimz - 1; ++k)
		for (int j = 1; j < A.dimy - 1; ++j)
			for (int i = 1; i < A.dimx - 1; ++i)
			{
				if (marker(i, j, k) == FLUIDCELL)
				{
					// No need to zero Adj since we assign the value on the first entry
					Adj(i, j, k) = A(i, j, k, 0) * d(i, j, k);     // i, j, k

					Adj(i, j, k) += A(i, j, k, 1) * d(i + 1, j, k); // i + 1, j, k
					Adj(i, j, k) += A(i, j, k, 2) * d(i, j + 1, k); // i, j + 1, k
					Adj(i, j, k) += A(i, j, k, 3) * d(i, j, k + 1); // i, j, k + 1

					Adj(i, j, k) += A(i - 1, j, k, 1) * d(i - 1, j, k); // i - 1, j, k
					Adj(i, j, k) += A(i, j - 1, k, 2) * d(i, j - 1, k); // i, j - 1, k
					Adj(i, j, k) += A(i, j, k - 1, 3) * d(i, j, k - 1); // i, j, k - 1
				}
			}
}

void vectorN_add(VectorN &lhs, const VectorN &rhs)
{
	for (int i = 0; i < lhs.size; ++i)
		lhs.data[i] += rhs.data[i];
}

void vectorN_add_scale(VectorN &lhs, const VectorN &rhs, double scale)
{
	for (int i = 0; i < lhs.size; ++i)
		lhs.data[i] += scale * rhs.data[i];
}

void vectorN_scale_add(VectorN &d, const VectorN &r, double beta)
{
	for (int i = 0; i < d.size; ++i)
		d.data[i] = r.data[i] + beta * d.data[i];
}

void vectorN_sub_scale(VectorN &r, const VectorN &Adj, double alpha)
{
	for (int i = 0; i < r.size; ++i)
		r.data[i] -= alpha * Adj.data[i];
}

double vectorN_dot(const VectorN &lhs, const VectorN &rhs)
{
	double *rhsitr = rhs.data;
	double sum = 0.0;
	for (double *lhsitr = lhs.data; lhsitr < lhs.data + lhs.size; ++lhsitr)
	{
		sum += (*lhsitr) * (*rhsitr++);
	}
	return sum;
}

double vectorN_norm2(const VectorN &lhs)
{
	double sum = 0;
	for (double *lhsitr = lhs.data; lhsitr < lhs.data + lhs.size; ++lhsitr)
	{
		sum += (*lhsitr) * (*lhsitr);
	}
	return sum;
}
