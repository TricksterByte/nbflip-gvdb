#pragma once
#ifndef GRID_H_
#define GRID_H_

#define AIRCELL 0
#define FLUIDCELL 1
#define SOLIDCELL 2

#include "util.h"
#include "array3d.h"
#include "sparse_matrix.h"
#include "unconditioned_cg_solver.h"

struct Grid
{
	int Nx, Ny, Nz;
	float h, overh, gravity, rho;

	Array3f u, v, w, du, dv, dw; // Staggered u, v, w velocities
	Array3c marker; // Voxel classification
	Sparse_Matrix poisson; // The matrix for pressure stage
	Sparse_Matrix precond; // The matrix for pressure stage
	VectorN rhs; // Right hand side of the poisson equation
	VectorN pressure; // Right hand side of the poisson equation

	Uncondioned_CG_Solver cg;

	Grid();
	Grid(int Nx_, int Ny_, int Nz_, float h_, float gravity_, float rho_);
	void init(int Nx_, int Ny_, int Nz_, float h_, float gravity_, float rho_);

	void zero();
	void bary_x(float x, int &i, float &fx);
	void bary_x_centre(float x, int &i, float &fx);
	void bary_y(float y, int &j, float &fy);
	void bary_y_centre(float y, int &j, float &fy);
	void bary_z(float z, int &k, float &fz);
	void bary_z_centre(float z, int &k, float &fz);

	void save_velocities();
	void get_velocity_update();
	void add_gravity(float dt);
	void classify_voxel();
	void apply_boundary_conditions();
	float CFL();

	void form_poisson(float dt);
	void calc_divergence();
	void project(float dt);
	void solve_pressure(int maxiterations, double tolerance);
	void form_precond();
};

#endif
