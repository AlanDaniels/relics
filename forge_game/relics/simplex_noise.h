#pragma once

#include "stdafx.h"
#include "my_math.h"


// The standard versions.
double simplex_noise_2(double xin, double yin);
double simplex_noise_3(double xin, double yin, double zin);
double simplex_noise_4(double x, double y, double z, double w);


// Here is one of the few times I'll tolerate C++ function overloading.
inline GLfloat simplex_noise_2(const MyVec2 &val) {
    return static_cast<GLfloat>(simplex_noise_2(val.x(), val.y()));
}

inline GLfloat simplex_noise_3(const MyVec4 &val) {
    return static_cast<GLfloat>(simplex_noise_3(val.x(), val.y(), val.z()));
}

inline GLfloat simplex_noise_4(const MyVec4 &val) {
    return static_cast<GLfloat>(simplex_noise_4(val.x(), val.y(), val.z(), val.w()));
}
