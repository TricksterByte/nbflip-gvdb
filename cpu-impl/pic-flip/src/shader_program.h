#pragma once

#include <string>
#include <map>

#include <gl/glew.h>

class ShaderProgram
{
private:

	struct ShaderInfo
	{
		GLuint id;
	};
	std::map<GLenum, ShaderInfo> _shaderIds;

	struct AttributeInfo
	{
		GLuint id;
	};
	std::map<std::string, AttributeInfo> _attributeIds;

	struct UniformInfo
	{
		GLint id;
	};
	std::map<std::string, UniformInfo> _uniformIds;

	struct UboInfo
	{
		GLuint id;
		GLuint binding_point;
	};
	std::map<std::string, UboInfo> _ubos;

	const std::string readFile(const std::string& path) const;

public:
	GLuint _programId;

	ShaderProgram();
	~ShaderProgram();

	void addShader(const GLenum shaderType, const std::string& path);

	void addAttribute(const std::string& name, const GLuint id);
	bool isAttribute(const std::string& name) const;
	GLuint getAttributeId(const std::string& name) const;

	void addUniform(const std::string& name);
	bool isUniform(const std::string& name) const;
	GLint getUniformId(const std::string& name) const;

	void addUniformBlock(const std::string& name, const GLuint bindingPoint);
	bool isUniformBlock(const std::string& name) const;

	void createProgram();
	void bind() const;
	void unbind() const;
};
