#pragma once
#ifndef GLAPP_H_
#define GLAPP_H_

#include <string>
#include <vector>
#include <memory>

#include <gl/glew.h>
#include <glfw/glfw3.h>
#include <glm/mat4x4.hpp>

#include "shader_program.h"
#include "array3d.h"

extern bool running, step, reset, showgrid, play;

struct Particles;

class GLApp
{
public:
	GLApp(int winx, int winy, int fullscreen, int vsync, int dimx, int dimy, int dimz, int gridh);
	~GLApp();

	void checkOpenGLInfo() const;

	void initParticles(void *vertices, void *velocities, size_t size, int nrOfParticles);
	void initWireframeCubes(void *positions, void *flags, int nrOfVoxels);
	void updateParticles(Particles &p);
	void updateVoxels(void *positions, Array3f &data, int nrOfVoxels);

	void display();

private:
	mutable GLFWwindow *_win = nullptr;

	std::unique_ptr<ShaderProgram> _particleShader;
	std::unique_ptr<ShaderProgram> _voxelShader;

	GLuint _wfCubeVAO, _wfCubeVBO, _wfCubeFlagsVBO, _wfCubeIndicesVBO;
	GLuint _particleVAO, _particleVBO;

	int _nrOfParticles, _nrOfVoxels;

	void setupErrorCallbacks() const;
	void setupWindowCallbacks() const;
	void setupGLFW() const;
	void setupWindow() const;
	void setupGLEW() const;
	void setupOpenGL() const;

	void createShaders();

	void drawParticles() const;
	void drawVoxels() const;
};

void window_close_callback(GLFWwindow *win);
void window_size_callback(GLFWwindow *win, int winx, int winy);
void key_callback(GLFWwindow *win, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *win, double xpos, double ypos);
void mouse_button_callback(GLFWwindow *win, int button, int action, int mods);
void mouse_scroll_callback(GLFWwindow *win, double xpos, double ypos);
void glfw_error_callback(int error, const char *description);
void error(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);

#endif
