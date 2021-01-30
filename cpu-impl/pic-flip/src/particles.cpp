#include "particles.h"

Particles::Particles() {}

Particles::Particles(int maxParticles, Grid &grid)
{
	init(maxParticles, grid);
}

void Particles::init(int maxParticles, Grid &grid)
{
	maxnp = maxParticles;
	currnp = 0;
	weightsumx.init(grid.u.nx, grid.u.ny, grid.u.nz);
	weightsumy.init(grid.v.nx, grid.v.ny, grid.v.nz);
	weightsumz.init(grid.w.nx, grid.w.ny, grid.w.nz);
}

void Particles::clear()
{
	pos.clear();
	vel.clear();
	currnp = 0;
}

void Particles::remove(int i)
{
	std::swap(vel[i], vel.back());
	std::swap(pos[i], pos.back());
	vel.pop_back();
	pos.pop_back();
	currnp = vel.size();
}

void move_particles_in_grid(Particles &particles, Grid &grid, float dt)
{
	vec3f vel;
	int ui, i, vj, j, wk, k;
	float ufx, fx, vfy, fy, wfz, fz;

	float xmax = (float)((grid.Nx - 1.001) * grid.h), xmin = (float)(1.001 * grid.h);
	float ymax = (float)((grid.Ny - 1.001) * grid.h), ymin = (float)(1.001 * grid.h);
	float zmax = (float)((grid.Nz - 1.001) * grid.h), zmin = (float)(1.001 * grid.h);

	for (size_t p = 0; p < particles.pos.size(); p++)
	{
		// Trilerp from grid
		grid.bary_x(particles.pos[p][0], ui, ufx);
		grid.bary_x_centre(particles.pos[p][0], i, fx);

		grid.bary_y(particles.pos[p][1], vj, vfy);
		grid.bary_y_centre(particles.pos[p][1], j, fy);

		grid.bary_z(particles.pos[p][2], wk, wfz);
		grid.bary_z_centre(particles.pos[p][2], k, fz);

		vel = vec3f(grid.u.trilerp(ui, j, k, ufx, fy, fz), grid.v.trilerp(i, vj, k, fx, vfy, fz), grid.w.trilerp(i, j, wk, fx, fy, wfz));

		// Move particle one step with forward euler
		if (grid.marker(ui, vj, wk) == SOLIDCELL)
			continue;

		vec3f newpos = particles.pos[p] + dt * vel;

		grid.bary_x(newpos[0], ui, ufx);
		grid.bary_y(newpos[1], vj, vfy);
		grid.bary_z(newpos[2], wk, wfz);

		if (ui < 0 || vj < 0 || wk < 0)
			continue;

		// Push particle out
		float scale = 1.0;
		bool moveX = true, moveY = true, moveZ = true;
		vec3f movevec(0.0);
		if (grid.marker(ui, vj, wk) == SOLIDCELL)
		{
			// X - AXIS
			if (grid.marker(ui + 1, vj, wk) != SOLIDCELL) // Push left
			{
				movevec[0] += 1.0;
			}
			else if (grid.marker(ui - 1, vj, wk) != SOLIDCELL) // Push right
			{
				movevec[0] -= 1.0;
			}
			else
			{
				moveX = false;
			}

			// Y - AXIS 

			if (grid.marker(ui, vj + 1, wk) != SOLIDCELL) // Push up
			{
				movevec[1] += 1.0;
			}
			else if (grid.marker(ui, vj - 1, wk) != SOLIDCELL) // Push down
			{
				movevec[1] -= 1.0;
			}
			else
			{
				moveY = false;
			}

			// Z - AXIS 
			if (grid.marker(ui, vj, wk + 1) != SOLIDCELL) // Push backwards
			{
				movevec[2] += 1.0;
			}
			else if (grid.marker(ui, vj, wk - 1) != SOLIDCELL) // Push forward
			{
				movevec[2] += 1.0;
			}
			else
			{
				moveZ = false;
			}

		}

		//if surrounded by solid
		if (!moveZ && !moveY && !moveX)
		{
			newpos = particles.pos[p];
		}
		else
		{
			newpos += movevec * grid.h * scale;
		}

		particles.pos[p] = newpos;
	}
}

void update_from_grid(Particles &particles, Grid &grid)
{
	int i, ui, j, vj, k, wk;
	float fx, ufx, fy, vfy, fz, wfz;

	for (size_t p = 0; p < particles.pos.size(); ++p) //Loop over all particles
	{
		grid.bary_x(particles.pos[p][0], ui, ufx);
		grid.bary_x_centre(particles.pos[p][0], i, fx);

		grid.bary_y(particles.pos[p][1], vj, vfy);
		grid.bary_y_centre(particles.pos[p][1], j, fy);

		grid.bary_z(particles.pos[p][2], wk, wfz);
		grid.bary_z_centre(particles.pos[p][2], k, fz);

		// PIC
		//particles.vel[p] = vec3f(grid.u.trilerp(ui, j, k, ufx, fy, fz), grid.v.trilerp(i, vj, k, fx, vfy, fz), grid.w.trilerp(i, j, wk, fx, fy, wfz)); 
		// FLIP
		//particles.vel[p] += vec3f(grid.du.trilerp(ui, j, k, ufx, fy, fz), grid.dv.trilerp(i, vj, k, fx, vfy, fz), grid.dw.trilerp(i, j, wk, fx, fy, wfz));

		// PIC/FLIP
		float alpha = 0.05f;
		particles.vel[p] = alpha * vec3f(grid.u.trilerp(ui, j, k, ufx, fy, fz), grid.v.trilerp(i, vj, k, fx, vfy, fz), grid.w.trilerp(i, j, wk, fx, fy, wfz))
			+ (1.0f - alpha) * (particles.vel[p] + vec3f(grid.du.trilerp(ui, j, k, ufx, fy, fz), grid.dv.trilerp(i, vj, k, fx, vfy, fz), grid.dw.trilerp(i, j, wk, fx, fy, wfz)));
	}
}

void accumulate(Array3f &macvel, Array3f &sum, float &pvel, int i, int j, int k, float fx, float fy, float fz)
{
	float weight = 0;

	weight = (1 - fx) * (1 - fy) * (1 - fz);
	macvel(i, j, k) += weight * pvel;
	sum(i, j, k) += weight;

	weight = fx * (1 - fy) * (1 - fz);
	macvel(i + 1, j, k) += weight * pvel;
	sum(i + 1, j, k) += weight;

	weight = (1 - fx) * fy * (1 - fz);
	macvel(i, j + 1, k) += weight * pvel;
	sum(i, j + 1, k) += weight;

	weight = fx * fy * (1 - fz);
	macvel(i + 1, j + 1, k) += weight * pvel;
	sum(i + 1, j + 1, k) += weight;

	weight = (1 - fx) * (1 - fy) * fz;
	macvel(i, j, k + 1) += weight * pvel;
	sum(i, j, k + 1) += weight;

	weight = fx * (1 - fy) * fz;
	macvel(i + 1, j, k + 1) += weight * pvel;
	sum(i + 1, j, k + 1) += weight;

	weight = (1 - fx) * fy * fz;
	macvel(i, j + 1, k + 1) += weight * pvel;
	sum(i, j + 1, k + 1) += weight;

	weight = fx * fy * fz;
	macvel(i + 1, j + 1, k + 1) += weight * pvel;
	sum(i + 1, j + 1, k + 1) += weight;
}

void transfer_to_grid(Particles &particles, Grid &grid)
{
	int ui, vj, wk, i, j, k;
	float fx, ufx, fy, vfy, fz, wfz;
	int tmpi, tmpj, tmpk;

	particles.weightsumx.zero();
	particles.weightsumy.zero();
	particles.weightsumz.zero();

	std::vector< int > removeIndices;

	for (size_t p = 0; p < particles.pos.size(); ++p) //Loop over all particles
	{
		grid.bary_x(particles.pos[p][0], ui, ufx); tmpi = ui;
		grid.bary_y(particles.pos[p][1], vj, vfy); tmpj = vj;
		grid.bary_z(particles.pos[p][2], wk, wfz); tmpk = wk;

		if (grid.marker(ui, vj, wk) == SOLIDCELL)
		{
			removeIndices.push_back(p);
			continue;
		}
		else
			grid.marker(ui, vj, wk) = FLUIDCELL;


		grid.bary_y_centre(particles.pos[p][1], j, fy);
		grid.bary_z_centre(particles.pos[p][2], k, fz);
		accumulate(grid.u, particles.weightsumx, particles.vel[p][0], ui, j, k, ufx, fy, fz);


		grid.bary_x_centre(particles.pos[p][0], i, fx);
		grid.bary_z_centre(particles.pos[p][2], k, fz);
		accumulate(grid.v, particles.weightsumy, particles.vel[p][1], i, vj, k, fx, vfy, fz);


		grid.bary_x_centre(particles.pos[p][0], i, fx);
		grid.bary_y_centre(particles.pos[p][1], j, fy);
		accumulate(grid.w, particles.weightsumz, particles.vel[p][2], i, j, wk, fx, fy, wfz);
	}

	for (size_t j = 0; j < removeIndices.size(); ++j)
	{
		particles.remove(removeIndices[j]);
	}

	//Scale u velocities with weightsumx

	for (int i = 0; i < grid.u.size; i++)
	{
		if (grid.u.data[i] != 0)
			grid.u.data[i] /= particles.weightsumx.data[i];
	}

	//Scale v velocities with weightsumy
	for (int i = 0; i < grid.v.size; i++)
	{
		if (grid.v.data[i] != 0)
			grid.v.data[i] /= particles.weightsumy.data[i];
	}

	//Scale w velocities with weightsumz
	for (int i = 0; i < grid.w.size; i++)
	{
		if (grid.w.data[i] != 0)
			grid.w.data[i] /= particles.weightsumz.data[i];
	}
}

//----------------------------------------------------------------------------//
// Adds a particle to the particles struct
//----------------------------------------------------------------------------//
void add_particle(Particles &particles, vec3f &pos, const vec3f &vel)
{
	particles.pos.push_back(pos);
	particles.vel.push_back(vel);
	++particles.currnp;
}
