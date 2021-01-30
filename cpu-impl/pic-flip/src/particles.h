#pragma once
#ifndef PARTICLES_H_
#define PARTICLES_H_

#include <vector>

#include "vector3.h"
#include "grid.h"
#include "array3D.h"

struct Particles
{
	// maximum nr of particles and the number of particles in use
	int maxnp, currnp;

	std::vector<vec3f> vel, pos;
	Array3f weightsumx, weightsumy, weightsumz;

	Particles();
	Particles(int maxParticles, Grid &grid);
	void init(int maxParticles, Grid &grid);

	void clear();
	void remove(int i);
};

void move_particles_in_grid(Particles &particles, Grid &grid, float dt);
void update_from_grid(Particles &particles, Grid &grid);
void accumulate(Array3f &macvel, Array3f &sum, float &pvel, int i, int j, int k, float fx, float fy, float fz);
void transfer_to_grid(Particles &particles, Grid &grid);
void add_particle(Particles &particles, vec3f &pos, const vec3f &vel);

#endif
