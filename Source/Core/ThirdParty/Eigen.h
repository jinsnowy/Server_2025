#pragma once

#include "Core/ThirdParty/WarningMacros.h"

EIGEN_IGNORE_WARNINGS_PUSH

#include <Eigen/Core>       // For Vector, Matrix types
#include <Eigen/Geometry>   // For Quaternion, AngleAxis, Rotation matrices
#include <cmath>            // For M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_F
#define M_PI_F 3.14159265358979323846f
#endif

EIGEN_IGNORE_WARNINGS_POP