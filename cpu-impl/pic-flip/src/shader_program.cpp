#include <string>
#include <map>
#include <fstream>
#include <iostream>

#include <gl/glew.h>
#include "shader_program.h"


ShaderProgram::ShaderProgram()
{
	_programId = glCreateProgram();

	_shaderIds = std::map<GLenum, ShaderInfo>();
	_attributeIds = std::map<std::string, AttributeInfo>();
	_uniformIds = std::map<std::string, UniformInfo>();
	_ubos = std::map<std::string, UboInfo>();
}

ShaderProgram::~ShaderProgram()
{
	glUseProgram(0);
	glDeleteProgram(_programId);
}

const std::string ShaderProgram::readFile(const std::string& path) const
{
	std::string source;
	std::string line;

	std::ifstream file(path);
	
	if (file.is_open())
	{
		while (getline(file, line))
			source += line + "\n";
		file.close();
	}
	else
		std::cerr << "Couldn't open file at location: " + path << std::endl;

	return source;
}

void ShaderProgram::addShader(const GLenum shaderType, const std::string& path)
{
	const std::string source = readFile(path);
	const GLchar* code = source.c_str();

	const GLuint shaderId = glCreateShader(shaderType);
	glShaderSource(shaderId, 1, &code, 0);
	glCompileShader(shaderId);
	glAttachShader(_programId, shaderId);

	_shaderIds[shaderType] = { shaderId };
}

void ShaderProgram::addAttribute(const std::string& name, const GLuint id)
{
	glBindAttribLocation(_programId, id, name.c_str());
	_attributeIds[name] = { id };
}

bool ShaderProgram::isAttribute(const std::string& name) const
{
	return _attributeIds.find(name) != _attributeIds.end();
}

GLuint ShaderProgram::getAttributeId(const std::string& name) const
{
	return _attributeIds.find(name)->second.id;
}

void ShaderProgram::addUniform(const std::string& name)
{
	_uniformIds[name] = { -1 };
}

bool ShaderProgram::isUniform(const std::string& name) const
{
	return _uniformIds.find(name) != _uniformIds.end();
}

GLint ShaderProgram::getUniformId(const std::string& name) const
{
	return _uniformIds.find(name)->second.id;
}

void ShaderProgram::addUniformBlock(const std::string& name, const GLuint bindingPoint)
{
	_ubos[name] = { 0, bindingPoint };
}

bool ShaderProgram::isUniformBlock(const std::string& name) const
{
	return _ubos.find(name) != _ubos.end();
}

void ShaderProgram::createProgram()
{
	glLinkProgram(_programId);

	for (auto& i : _shaderIds)
	{
		glDetachShader(_programId, i.second.id);
		glDeleteShader(i.second.id);
	}

	for (auto& i : _uniformIds)
	{
		i.second.id = glGetUniformLocation(_programId, i.first.c_str());
		if (i.second.id < 0) std::cerr << "WARNING: Uniform " << i.first << " not found." << std::endl;
	}

	for (auto& i : _ubos)
	{
		i.second.id = glGetUniformBlockIndex(_programId, i.first.c_str());
		if (i.second.id < 0) std::cerr << "WARNING: UBO " << i.first << " not found." << std::endl;
		glUniformBlockBinding(_programId, i.second.id, i.second.binding_point);
	}
}

void ShaderProgram::bind() const
{
	glUseProgram(_programId);
}

void ShaderProgram::unbind() const
{
	glUseProgram(0);
}
