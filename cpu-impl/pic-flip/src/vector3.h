#pragma once
#ifndef VECTOR3_H_
#define VECTOR3_H_

#include <iostream>
#include <cmath>
#include "util.h"

struct vec3f
{
	float v[3];
	
	vec3f();
	vec3f(float val);
	vec3f(float x, float y, float z);
	vec3f(const vec3f &source);

	vec3f &operator=(const vec3f &source);
	float &operator[](size_t index);
	vec3f &operator+=(const vec3f &rhs);
	vec3f &operator-=(const vec3f &rhs);
	vec3f &operator*=(const float &scalar);
	vec3f &operator/=(const float &scalar);
}; 

float mag2(const vec3f &a);
float mag(const vec3f &a);
float dist2(const vec3f &a, const vec3f &b);
float dist(const vec3f &a, const vec3f &b);
bool operator==(const vec3f &a, const vec3f &b);
bool operator!=(const vec3f &a, const vec3f &b);
vec3f operator-(const vec3f &a);
vec3f operator+(const vec3f &a, const vec3f &b);
vec3f operator-(const vec3f &a, const vec3f &b);
vec3f operator*(const vec3f &a, float scalar);
vec3f operator*(float scalar, const vec3f &a);
vec3f operator/(const vec3f &a, float scalar);
float dot(const vec3f &a, const vec3f &b);
vec3f cross(const vec3f &a, const vec3f &b);
void normalize(vec3f &a);
vec3f normalized(const vec3f &a);
std::ostream &operator<<(std::ostream &out, const vec3f &a);

#endif
