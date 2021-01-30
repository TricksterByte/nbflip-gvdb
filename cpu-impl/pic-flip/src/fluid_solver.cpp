#include "fluid_solver.h"

FluidSolver::FluidSolver(int dimx, int dimy, int dimz, float h, float timestep, float gravity, float rho, int max_particles)
	: dimx(dimx), dimy(dimy), dimz(dimz), timestep(timestep)
{
	grid.init(dimx, dimy, dimz, h, gravity, rho);
	particles.init(max_particles, grid);
}

void FluidSolver::reset()
{
	particles.clear();
	init_box();

}

void FluidSolver::init_box()
{
	srand(time(NULL));
	float r1, r2, r3;
	float subh = grid.h / 2.0f;
	vec3f pos(0);

	for (float k = 1; k < 10; ++k)
		for (float j = 1; j < 10; ++j)
			for (float i = 1; i < 10; ++i)
			{
				for (int kk = -1; kk < 1; ++kk)
					for (int jj = -1; jj < 1; ++jj)
						for (int ii = -1; ii < 1; ++ii)
						{
							r1 = float(rand()) / RAND_MAX - 0.5f; //[-0.5, 0.5]
							r2 = float(rand()) / RAND_MAX - 0.5f;
							r3 = float(rand()) / RAND_MAX - 0.5f;
							pos[0] = (i + 0.5f) * grid.h + (ii + 0.5f + 0.95f * r1) * subh;
							pos[1] = (j + 0.5f) * grid.h + (jj + 0.5f + 0.95f * r2) * subh;
							pos[2] = (k + 0.5f) * grid.h + (kk + 0.5f + 0.95f * r3) * subh;
							add_particle(particles, pos, vec3f(0.f));
						}
			}

	for (float k = 20; k < 30; ++k)
		for (float j = 20; j < 30; ++j)
			for (float i = 40; i < 50; ++i)
			{
				for (int kk = -1; kk < 1; ++kk)
					for (int jj = -1; jj < 1; ++jj)
						for (int ii = -1; ii < 1; ++ii)
						{
							r1 = float(rand()) / RAND_MAX - 0.5f; //[-0.5, 0.5]
							r2 = float(rand()) / RAND_MAX - 0.5f;
							r3 = float(rand()) / RAND_MAX - 0.5f;
							pos[0] = (i + 0.5f) * grid.h + (ii + 0.5f + 0.95f * r1) * subh;
							pos[1] = (j + 0.5f) * grid.h + (jj + 0.5f + 0.95f * r2) * subh;
							pos[2] = (k + 0.5f) * grid.h + (kk + 0.5f + 0.95f * r3) * subh;
							add_particle(particles, pos, vec3f(0.0f));
						}
			}

	for (float k = 43; k < 53; ++k)
		for (float j = 1; j < 10; ++j)
			for (float i = 79; i < 89; ++i)
			{
				for (int kk = -1; kk < 1; ++kk)
					for (int jj = -1; jj < 1; ++jj)
						for (int ii = -1; ii < 1; ++ii)
						{
							r1 = float(rand()) / RAND_MAX - 0.5f; //[-0.5, 0.5]
							r2 = float(rand()) / RAND_MAX - 0.5f;
							r3 = float(rand()) / RAND_MAX - 0.5f;
							pos[0] = (i + 0.5f) * grid.h + (ii + 0.5f + 0.95f * r1) * subh;
							pos[1] = (j + 0.5f) * grid.h + (jj + 0.5f + 0.95f * r2) * subh;
							pos[2] = (k + 0.5f) * grid.h + (kk + 0.5f + 0.95f * r3) * subh;
							add_particle(particles, pos, vec3f(0.0f));
						}
			}
}

void FluidSolver::step_frame()
{
	static int frame = 0;

	for (float elapsed = 0; elapsed < timestep;)
	{

		float dt = grid.CFL();
		if (dt > timestep - elapsed)
			dt = timestep - elapsed;

		elapsed += dt;

		step(dt);
	}
	frame++;
}

void FluidSolver::step(float dt)
{

	// grid.extend_velocity();
	for (int i = 0; i < 5; i++)
		move_particles_in_grid(particles, grid, 0.2f * dt);

	grid.zero();

	grid.classify_voxel();

	transfer_to_grid(particles, grid);

	grid.save_velocities();
	grid.add_gravity(dt);

	grid.apply_boundary_conditions();

	// Pressure
	grid.form_poisson(dt);
	grid.calc_divergence();
	grid.solve_pressure(100, 1e-6);
	grid.project(dt);

	grid.apply_boundary_conditions();
	grid.get_velocity_update();
	update_from_grid(particles, grid);
}
