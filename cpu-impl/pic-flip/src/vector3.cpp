#include "vector3.h"

vec3f::vec3f() { v[0] = v[1] = v[2] = 0; }

vec3f::vec3f(float val) { v[0] = v[1] = v[2] = val; }

vec3f::vec3f(float x, float y, float z) { v[0] = x; v[1] = y; v[2] = z; }

vec3f::vec3f(const vec3f &source) { v[0] = source.v[0]; v[1] = source.v[1]; v[2] = source.v[2]; }

vec3f &vec3f::operator=(const vec3f &source)
{
	if (this == &source)
		return *this;

	v[0] = source.v[0]; v[1] = source.v[1]; v[2] = source.v[2];

	return *this;
}

float &vec3f::operator[](size_t index) { return v[index]; }

vec3f &vec3f::operator+=(const vec3f &rhs)
{
	v[0] += rhs.v[0];
	v[1] += rhs.v[1];
	v[2] += rhs.v[2];
	return *this;
}

vec3f &vec3f::operator-=(const vec3f &rhs)
{
	v[0] -= rhs.v[0];
	v[1] -= rhs.v[1];
	v[2] -= rhs.v[2];
	return *this;
}

vec3f &vec3f::operator*=(const float &scalar)
{
	v[0] *= scalar;
	v[1] *= scalar;
	v[2] *= scalar;
	return *this;
}

vec3f &vec3f::operator/=(const float &scalar)
{
	v[0] /= scalar;
	v[1] /= scalar;
	v[2] /= scalar;
	return *this;
}

float mag2(const vec3f &a)
{
	return a.v[0] * a.v[0] + a.v[1] * a.v[1] + a.v[2] * a.v[2];
}

float mag(const vec3f &a)
{
	return std::sqrtf(mag2(a));
}

float dist2(const vec3f &a, const vec3f &b)
{
	return sqr(a.v[0] - b.v[0]) + sqr(a.v[1] - b.v[1]) + sqr(a.v[2] - b.v[2]);
}

float dist(const vec3f &a, const vec3f &b)
{
	return sqrtf(dist2(a, b));
}

bool operator==(const vec3f &a, const vec3f &b)
{
	return a.v[0] == b.v[0] && a.v[1] == b.v[1] && a.v[2] == b.v[2];
}

bool operator!=(const vec3f &a, const vec3f &b)
{
	return a.v[0] != b.v[0] || a.v[1] != b.v[1] || a.v[2] != b.v[2];
}

vec3f operator-(const vec3f &a)
{
	return vec3f(-a.v[0], -a.v[1], -a.v[2]);
}

vec3f operator+(const vec3f &a, const vec3f &b)
{
	return vec3f(a.v[0] + b.v[0], a.v[1] + b.v[1], a.v[2] + b.v[2]);
}

vec3f operator-(const vec3f &a, const vec3f &b)
{
	return vec3f(a.v[0] - b.v[0], a.v[1] - b.v[1], a.v[2] - b.v[2]);
}

vec3f operator*(const vec3f &a, float scalar)
{
	return vec3f(scalar * a.v[0], scalar * a.v[1], scalar * a.v[2]);
}

vec3f operator*(float scalar, const vec3f &a)
{
	return vec3f(scalar * a.v[0], scalar * a.v[1], scalar * a.v[2]);
}

vec3f operator/(const vec3f &a, float scalar)
{
	return vec3f(a.v[0] / scalar, a.v[1] / scalar, a.v[2] / scalar);
}

float dot(const vec3f &a, const vec3f &b)
{
	return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2];
}

vec3f cross(const vec3f &a, const vec3f &b)
{
	return vec3f(a.v[1] * b.v[2] - b.v[1] * a.v[2], b.v[0] * a.v[2] - a.v[0] * b.v[2], a.v[0] * b.v[1] - b.v[0] * a.v[1]);
}

void normalize(vec3f &a)
{
	a /= mag(a);
}

vec3f normalized(const vec3f &a)
{
	return a / mag(a);
}

std::ostream &operator<<(std::ostream &out, const vec3f &a)
{
	return out << a.v[0] << ' ' << a.v[1] << ' ' << a.v[2];
}
