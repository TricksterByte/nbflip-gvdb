#pragma once
#ifndef FLUID_SOLVER_H_
#define FLUID_SOLVER_H_

#include "particles.h"
#include "grid.h"
#include "vector3.h"
#include "unconditioned_cg_solver.h"

struct FluidSolver
{
	Particles particles;
	Grid grid;
	
	int dimx, dimy, dimz;
	float timestep;

	FluidSolver(int dimx, int dimy, int dimz, float h, float timestep, float gravity, float rho, int max_particles);

	void reset();

	void step_frame();
	void step(float dt);

	void init_box();
};

#endif
