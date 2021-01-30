#include <iostream>

#define GLEW_STATIC

#include "glapp.h"
#include "array3D.h"
#include "fluid_solver.h"

const int perCell = 8;
const int box = 10;
const int Nparticles = 30000 * perCell;
const int dimx = 100, dimy = 78, dimz = 64;
const float gridh = 0.1f;

void initVoxels(float * voxelPositions, int dx, int dy, int dz)
{
	float * posItr = voxelPositions;
	for (int z = 0; z < dz; ++z) {
		for (int y = 0; y < dy; ++y) {
			for (int x = 0; x < dx; ++x) {
				*posItr++ = (float)x;
				*posItr++ = (float)y;
				*posItr++ = (float)z;	  
			}
		}
	}
}

void update_voxel_flags(Grid &grid, Array3f &flags)
{
	for (int k = 1; k < grid.Nz-1; ++k)
		for (int j = 1; j < grid.Ny-1; ++j)
			for (int i = 1; i < grid.Nx-1; ++i)
			{
				flags(i, j, k) = (float)grid.marker(i, j, k);
			}
}

int main()
{
	GLApp app(600, 600, 0, 0, dimx, dimy, dimz, gridh);
	FluidSolver fluid_solver(dimx, dimy, dimz, gridh, 1.0f / 30.0f, 9.82f, 1.0f, Nparticles);
	fluid_solver.init_box();

	app.initParticles(&fluid_solver.particles.pos[0], &fluid_solver.particles.vel[0], sizeof(vec3f) * fluid_solver.particles.currnp, fluid_solver.particles.currnp);	
	
	std::cout << "Number of particles: " << fluid_solver.particles.currnp << std::endl;

	int Nvoxels = dimx * dimy * dimz;
	Array3f voxelFlags(dimx, dimy, dimz);	
	float * voxelPositions  = new float[3 * dimx * dimy * dimz];	
	initVoxels(voxelPositions,dimx,dimy,dimz);

	app.initWireframeCubes(voxelPositions,voxelFlags.data,Nvoxels);
	update_voxel_flags(fluid_solver.grid, voxelFlags);
	
	while (running)
	{
		if (reset)
			fluid_solver.reset();
		reset = false;

		if (showgrid)
		{		
			update_voxel_flags(fluid_solver.grid, voxelFlags);
			app.updateVoxels(voxelPositions, voxelFlags, Nvoxels);
		}

		app.display();

		if (step || play)
		{
			fluid_solver.step_frame();
		}

		app.updateParticles(fluid_solver.particles);
	}
}
