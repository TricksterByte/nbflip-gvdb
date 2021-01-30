#include <iostream>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/scalar_constants.hpp>

#include "glapp.h"
#include "particles.h"
#include "array3d.h"

int _winx, _winy;
int _fullscreen, _vsync;

int lastMouseX, lastMouseY;
float posDx, posDy, zoom, rotDx, rotDy;

double t0;
int frames;
int nZ;
float h;
float edge;

glm::mat4 _modelView;
glm::mat4 _proj;

bool running = true, step = false, reset = false, showgrid = false, play = false;

GLApp::GLApp(int winx, int winy, int fullscreen, int vsync, int dimx, int dimy, int dimz, int gridh)
{
	_winx = winx; _winy = winy;
	_fullscreen = fullscreen; _vsync = vsync;

	nZ = dimz;
	h = gridh;

	lastMouseX = 0; lastMouseY = 0;
	posDx = 0.f; posDy = 0.f; zoom = 0.f; rotDx = 0.f; rotDy = 0.f;

	edge = 3.f;

	running = true; step = false; reset = false; showgrid = false; play = false;

	setupGLFW();
	setupGLEW();
	setupOpenGL();
	setupErrorCallbacks();

	createShaders();
}

GLApp::~GLApp()
{
	glDeleteVertexArrays(1, &_particleVAO);
	glDeleteBuffers(1, &_particleVBO);
}

void GLApp::createShaders()
{
	_particleShader = std::make_unique<ShaderProgram>();
	_particleShader->addShader(GL_VERTEX_SHADER, "src/shaders/particle_VertexShader.glsl");
	_particleShader->addShader(GL_FRAGMENT_SHADER, "src/shaders/particle_FragmentShader.glsl");
	_particleShader->addAttribute("velocity", 0);
	_particleShader->addAttribute("vertex", 1);
	_particleShader->addUniform("modelViewMatrix");
	_particleShader->addUniform("projectionMatrix");
	_particleShader->addUniform("dimz");
	_particleShader->addUniform("h");
	_particleShader->addUniform("edge");
	_particleShader->createProgram();

	_voxelShader = std::make_unique<ShaderProgram>();
	_voxelShader->addShader(GL_VERTEX_SHADER, "src/shaders/instancedVoxel_VertexShader.glsl");
	_voxelShader->addShader(GL_FRAGMENT_SHADER, "src/shaders/instancedVoxel_FragmentShader.glsl");
	_voxelShader->addAttribute("vertex", 0);
	_voxelShader->addAttribute("position", 1);
	_voxelShader->addAttribute("isFluid", 2);
	_voxelShader->addUniform("modelViewMatrix");
	_voxelShader->addUniform("projectionMatrix");
	_voxelShader->createProgram();
}

void GLApp::initParticles(void *vertices, void *velocities, size_t size, int nrOfParticles)
{
	_nrOfParticles = nrOfParticles;
	glGenVertexArrays(1, &_particleVAO);
	glGenBuffers(1, &_particleVBO);
	glBindVertexArray(_particleVAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, _particleVBO);
		glBufferData(GL_ARRAY_BUFFER, 2 * size, 0, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertices);
		glBufferSubData(GL_ARRAY_BUFFER, size, size, velocities);
		glEnableVertexAttribArray(_particleShader->getAttributeId("vertex"));
		glEnableVertexAttribArray(_particleShader->getAttributeId("velocity"));
	}
	glBindVertexArray(0);
}

void GLApp::initWireframeCubes(void *positions, void *flags, int nrOfVoxels)
{
	static GLfloat unitWFCubeVertices[] =
	{
		1.0f,1.0f,1.0f,  0.0f,1.0f,1.0f,  0.0f,0.0f,1.0f,  1.0f,0.0f,1.0f,
		1.0f,0.0f,0.0f,  1.0f,1.0f,0.0f,	0.0f,1.0f,0.0f,  0.0f,0.0f,0.0f
	};

	static GLuint unitWFCubeindices[] = { 0,1, 1,2, 2,3, 0,3, 3,4, 4,7, 2,7, 4,5, 5,6, 7,6, 0,5, 1,6 };

	_nrOfVoxels = nrOfVoxels;
	size_t positionSize = 3 * sizeof(float) * nrOfVoxels;
	size_t flagSize = sizeof(float) * nrOfVoxels;

	glGenVertexArrays(1, &_wfCubeVAO);
	glGenBuffers(1, &_wfCubeVBO);
	glGenBuffers(1, &_wfCubeIndicesVBO);
	glGenBuffers(1, &_wfCubeFlagsVBO);

	glBindVertexArray(_wfCubeVAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, _wfCubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(unitWFCubeVertices), unitWFCubeVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(_voxelShader->getAttributeId("vertex"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(_voxelShader->getAttributeId("vertex"));

		glBindBuffer(GL_ARRAY_BUFFER, _wfCubeFlagsVBO);
		glBufferData(GL_ARRAY_BUFFER, positionSize + flagSize, 0, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, positionSize, positions);
		glBufferSubData(GL_ARRAY_BUFFER, positionSize, flagSize, flags);
		glVertexAttribPointer(_voxelShader->getAttributeId("position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(_voxelShader->getAttributeId("isFluid"), 1, GL_FLOAT, GL_FALSE, 0, (GLvoid *)positionSize);
		glEnableVertexAttribArray(_voxelShader->getAttributeId("position"));
		glEnableVertexAttribArray(_voxelShader->getAttributeId("isFluid"));
		glVertexAttribDivisor(_voxelShader->getAttributeId("position"), 1);
		glVertexAttribDivisor(_voxelShader->getAttributeId("isFluid"), 1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _wfCubeIndicesVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unitWFCubeindices), unitWFCubeindices, GL_STATIC_DRAW);
	}
	glBindVertexArray(0);
}

void GLApp::updateParticles(Particles &p)
{
	_nrOfParticles = p.pos.size();
	int size = p.pos.size() * sizeof(vec3f);
	glBindVertexArray(_particleVAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, _particleVBO);
		glBufferData(GL_ARRAY_BUFFER, 2.f * size, 0, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, &p.pos[0]);
		glBufferSubData(GL_ARRAY_BUFFER, size, size, &p.vel[0]);
		glVertexAttribPointer(_particleShader->getAttributeId("vertex"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(_particleShader->getAttributeId("velocity"), 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)size);
		glEnableVertexAttribArray(_particleShader->getAttributeId("vertex"));
		glEnableVertexAttribArray(_particleShader->getAttributeId("velocity"));
	}
	glBindVertexArray(0);
}

void GLApp::updateVoxels(void *positions, Array3f &data, int nrOfVoxels)
{
	_nrOfVoxels = nrOfVoxels;
	size_t PosSize = 3 * sizeof(float) * nrOfVoxels;
	size_t FlagSize = sizeof(float) * nrOfVoxels;
	glBindBuffer(GL_ARRAY_BUFFER, _wfCubeFlagsVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, PosSize, positions);
	glBufferSubData(GL_ARRAY_BUFFER, PosSize, FlagSize, data.data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#pragma region setup

void GLApp::setupGLFW() const
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	setupWindow();
	setupWindowCallbacks();

#if _DEBUG
	std::cout << "GLFW " << glfwGetVersionString() << std::endl;
#endif
}

void GLApp::setupWindow() const
{
	GLFWmonitor *monitor = _fullscreen ? glfwGetPrimaryMonitor() : 0;
	_win = glfwCreateWindow(_winx, _winy, "PIC/FLIP", monitor, 0);
	if (!_win)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(_win);
	glfwSwapInterval(_vsync);
}

void GLApp::setupWindowCallbacks() const
{
	glfwSetWindowCloseCallback(_win, window_close_callback);
	glfwSetWindowSizeCallback(_win, window_size_callback);
	glfwSetKeyCallback(_win, key_callback);
	glfwSetCursorPosCallback(_win, mouse_callback);
	glfwSetMouseButtonCallback(_win, mouse_button_callback);
	glfwSetScrollCallback(_win, mouse_scroll_callback);
}

void glfw_error_callback(int error, const char *description)
{
	std::cerr << "GLFW Error: " << description << std::endl;
}

void GLApp::setupGLEW() const
{
	glewExperimental = GL_TRUE;
	// Allow extension entry points to be loaded even if the extension isn't 
	// present in the driver's extensions string.
	GLenum result = glewInit();
	if (result != GLEW_OK)
	{
		std::cerr << "ERROR glewInit: " << glewGetString(result) << std::endl;
		exit(EXIT_FAILURE);
	}
	GLenum err_code = glGetError();
	// You might get GL_INVALID_ENUM when loading GLEW.
}

void GLApp::setupOpenGL() const
{
#if _DEBUG
	checkOpenGLInfo();
#endif
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glDepthRange(0.0, 1.0);
	glClearDepth(1.0);
	glDisable(GL_CULL_FACE);
	glViewport(0, 0, _winx, _winy);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SMOOTH);
}

void GLApp::checkOpenGLInfo() const
{
	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *vendor = glGetString(GL_VENDOR);
	const GLubyte *version = glGetString(GL_VERSION);
	const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	std::cerr << "OpenGL Renderer: " << renderer << " (" << vendor << ")" << std::endl;
	std::cerr << "OpenGL version " << version << std::endl;
	std::cerr << "GLSL version " << glslVersion << std::endl;
}

void GLApp::setupErrorCallbacks() const
{
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(error, 0);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, GL_FALSE);
	// params: source, type, severity, count, ids, enabled
}

const std::string errorSource(GLenum source)
{
	switch (source) {
	case GL_DEBUG_SOURCE_API:				return "API";
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		return "window system";
	case GL_DEBUG_SOURCE_SHADER_COMPILER:	return "shader compiler";
	case GL_DEBUG_SOURCE_THIRD_PARTY:		return "third party";
	case GL_DEBUG_SOURCE_APPLICATION:		return "application";
	case GL_DEBUG_SOURCE_OTHER:				return "other";
	default:								exit(EXIT_FAILURE);
	}
}

const std::string errorType(GLenum type)
{
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:				return "error";
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:	return "deprecated behavior";
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:	return "undefined behavior";
	case GL_DEBUG_TYPE_PORTABILITY:			return "portability issue";
	case GL_DEBUG_TYPE_PERFORMANCE:			return "performance issue";
	case GL_DEBUG_TYPE_MARKER:				return "stream annotation";
	case GL_DEBUG_TYPE_PUSH_GROUP:			return "push group";
	case GL_DEBUG_TYPE_POP_GROUP:			return "pop group";
	case GL_DEBUG_TYPE_OTHER_ARB:			return "other";
	default:								exit(EXIT_FAILURE);
	}
}

const std::string errorSeverity(GLenum severity)
{
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:			return "high";
	case GL_DEBUG_SEVERITY_MEDIUM:			return "medium";
	case GL_DEBUG_SEVERITY_LOW:				return "low";
	case GL_DEBUG_SEVERITY_NOTIFICATION:	return "notification";
	default:								exit(EXIT_FAILURE);
	}
}

void error(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
	std::cerr << "GL ERROR:" << std::endl;
	std::cerr << "  source:     " << errorSource(source) << std::endl;
	std::cerr << "  type:       " << errorType(type) << std::endl;
	std::cerr << "  severity:   " << errorSeverity(severity) << std::endl;
	std::cerr << "  debug call: " << std::endl << message << std::endl;
	std::cerr << "Press <return>.";
	std::cin.ignore();
}

#pragma endregion setup

void window_close_callback(GLFWwindow *win)
{

}

void window_size_callback(GLFWwindow *win, int winx, int winy)
{
	_winx = winx; _winy = winy;
	glViewport(0, 0, winx, winy);

	_proj = glm::perspective(glm::pi<float>() * .25f, (float)winx / (float)winy, 1.f, 10000000.f);
}

void key_callback(GLFWwindow *win, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_P)
	{
		if (action == GLFW_PRESS)
			play = !play;
	}

	if (key == GLFW_KEY_G)
	{
		if (action == GLFW_PRESS)
			showgrid = !showgrid;
	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		reset = true;

	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		step = true;
	if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		step = false;

	if (key == GLFW_KEY_C && action == GLFW_PRESS) // Cam reset
	{
		rotDy = 0;
		rotDx = 0;
		posDx = 0;
		posDy = 0;
		zoom = 0;
	}

	if (key == GLFW_KEY_ESCAPE)
	{
		running = false;
	}
}

void mouse_callback(GLFWwindow *win, double xpos, double ypos)
{
	if (glfwGetKey(win,GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		rotDy += (lastMouseX - xpos) * 0.5;
		rotDx += (lastMouseY - ypos) * 0.5;
	}
	else if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		posDy += (lastMouseY - ypos) * 0.05;
		posDx -= (lastMouseX - xpos) * 0.05;
	}
	lastMouseX = xpos;
	lastMouseY = ypos;
}

void mouse_button_callback(GLFWwindow *win, int button, int action, int mods)
{

}

void mouse_scroll_callback(GLFWwindow *win, double xpos, double ypos)
{
	zoom += ypos;
}

void GLApp::drawParticles() const
{
	_particleShader->bind();
	glBindVertexArray(_particleVAO);
	{
		glUniformMatrix4fv(_particleShader->getUniformId("modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(_modelView));
		glUniformMatrix4fv(_particleShader->getUniformId("projectionMatrix"), 1, GL_FALSE, glm::value_ptr(_proj));
		glUniform1iv(_particleShader->getUniformId("dimz"), 1, &nZ);
		glUniform1fv(_particleShader->getUniformId("h"), 1, &h);
		glUniform1fv(_particleShader->getUniformId("edge"), 1, &edge);
		glPointSize(4.0);
		glDrawArrays(GL_POINTS, 0, _nrOfParticles);
	}
	glBindVertexArray(0);
	_particleShader->unbind();
}

void GLApp::drawVoxels() const
{
	_voxelShader->bind();
	glBindVertexArray(_wfCubeVAO);
	{
		glUniformMatrix4fv(_voxelShader->getUniformId("modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(_modelView));
		glUniformMatrix4fv(_voxelShader->getUniformId("projectionMatrix"), 1, GL_FALSE, glm::value_ptr(_proj));
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawElementsInstanced(GL_LINES, 2 * 12, GL_UNSIGNED_INT, 0, _nrOfVoxels);
		glDisable(GL_BLEND);
	}
	glBindVertexArray(0);
	_voxelShader->unbind();
}

void GLApp::display()
{
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	running = !glfwGetKey(_win, GLFW_KEY_ESCAPE) && !glfwWindowShouldClose(_win);

	if (!running)
		return;

	glClearColor(.4f, .8f, .9f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_modelView = glm::lookAt(glm::vec3(0.f, 2.f, -10.f), glm::vec3(5.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	_modelView = glm::translate(_modelView, glm::vec3(posDx, posDy, -zoom));
	_modelView = glm::rotate(_modelView, rotDx * 0.01f, glm::vec3(1.f, 0.f, 0.f));
	_modelView = glm::rotate(_modelView, rotDy * 0.01f, glm::vec3(0.f, 1.f, 0.f));

	drawParticles();

	if (showgrid)
		drawVoxels();

	glfwSwapBuffers(_win);
	glfwPollEvents();
}
