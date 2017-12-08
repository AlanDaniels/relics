#pragma once

#include "stdafx.h"
#include "my_math.h"

// The standard versions.
double simplex_noise_2(double xin, double yin);
double simplex_noise_3(double xin, double yin, double zin);
double simplex_noise_4(double x, double y, double z, double w);

// Overloaded versions.
GLfloat simplex_noise_2(const MyVec2 &val);
GLfloat simplex_noise_3(const MyVec4 &val);
GLfloat simplex_noise_4(const MyVec4 &val);
