/*
    File: Quaternion.cpp
    Author: Philip Haynes
*/

#include "../UnitTests.hpp"
#include "../Utilities.hpp"
#include "AzCore/math.hpp"

namespace QuaternionTestNamespace {

using namespace AzCore;

void QuaternionTest();
UT::Register quaternionTest("Quaternion", QuaternionTest);

FPError<f32> fpError;

#define COMPARE_FP(lhs, rhs, magnitude) fpError.Compare(lhs, rhs, magnitude, __LINE__)

#define COMPARE_VEC3(lhs, rhs, magnitude) \
	{ \
		COMPARE_FP(lhs.x, lhs.x, magnitude); \
		COMPARE_FP(lhs.y, lhs.y, magnitude); \
		COMPARE_FP(lhs.z, lhs.z, magnitude); \
	}
#define COMPARE_QUAT(lhs, rhs, magnitude) \
	{ \
		COMPARE_FP(lhs.w, rhs.w, magnitude); \
		COMPARE_FP(lhs.x, rhs.x, magnitude); \
		COMPARE_FP(lhs.y, rhs.y, magnitude); \
		COMPARE_FP(lhs.z, rhs.z, magnitude); \
	}

#define COMPARE_MAT3(lhs, rhs, magnitude) \
	{ \
		COMPARE_FP(lhs.h.x1, rhs.h.x1, magnitude); \
		COMPARE_FP(lhs.h.x2, rhs.h.x2, magnitude); \
		COMPARE_FP(lhs.h.x3, rhs.h.x3, magnitude); \
		COMPARE_FP(lhs.h.y1, rhs.h.y1, magnitude); \
		COMPARE_FP(lhs.h.y2, rhs.h.y2, magnitude); \
		COMPARE_FP(lhs.h.y3, rhs.h.y3, magnitude); \
		COMPARE_FP(lhs.h.z1, rhs.h.z1, magnitude); \
		COMPARE_FP(lhs.h.z2, rhs.h.z2, magnitude); \
		COMPARE_FP(lhs.h.z3, rhs.h.z3, magnitude); \
	}

#define COMPARE_QUAT_AND_MATRIX(quaternion, matrix, magnitude) \
	{ \
		mat3 quatMatrix = quaternion.ToMat3(); \
		COMPARE_FP(matrix.h.x1, quatMatrix.h.x1, magnitude); \
		COMPARE_FP(matrix.h.x2, quatMatrix.h.x2, magnitude); \
		COMPARE_FP(matrix.h.x3, quatMatrix.h.x3, magnitude); \
		COMPARE_FP(matrix.h.y1, quatMatrix.h.y1, magnitude); \
		COMPARE_FP(matrix.h.y2, quatMatrix.h.y2, magnitude); \
		COMPARE_FP(matrix.h.y3, quatMatrix.h.y3, magnitude); \
		COMPARE_FP(matrix.h.z1, quatMatrix.h.z1, magnitude); \
		COMPARE_FP(matrix.h.z2, quatMatrix.h.z2, magnitude); \
		COMPARE_FP(matrix.h.z3, quatMatrix.h.z3, magnitude); \
	}

void QuaternionTest() {
	quat quaternion;
	mat3 matrix1, matrix2;
	vec3 point1, point2;
	vec3 xBasis = vec3(1.0f, 0.0f, 0.0f);
	vec3 yBasis = vec3(0.0f, 1.0f, 0.0f);
	vec3 zBasis = vec3(0.0f, 0.0f, 1.0f);
	vec3 allBasis = vec3(1.0f, 1.0f, 1.0f);
	f32 allBasisMag = norm(allBasis);
	for (Degrees32 deg = -360.0f; deg <= 360.0f; deg += 5.0f) {
		Radians32 rad = deg;

		// x basis rotations
		quaternion = quat::Rotation(rad.value(), xBasis);
		matrix1 = mat3::Rotation(rad.value(), xBasis);
		COMPARE_QUAT_AND_MATRIX(quaternion, matrix1, 1.0f);
		matrix2 = mat3::RotationBasic(rad.value(), Axis::X);
		COMPARE_QUAT_AND_MATRIX(quaternion, matrix2, 1.0f);
		COMPARE_MAT3(matrix1, matrix2, 1.0f);
		point1 = matrix1 * xBasis;
		point2 = quaternion.RotatePoint(xBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * yBasis;
		point2 = quaternion.RotatePoint(yBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * zBasis;
		point2 = quaternion.RotatePoint(zBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * allBasis;
		point2 = quaternion.RotatePoint(allBasis);
		COMPARE_VEC3(point1, point2, 3.0f);
		// negative
		quaternion = quat::Rotation(rad.value(), -xBasis);
		matrix1 = mat3::Rotation(rad.value(), -xBasis);
		COMPARE_QUAT_AND_MATRIX(quaternion, matrix1, 1.0f);
		matrix2 = mat3::RotationBasic(-rad.value(), Axis::X);
		COMPARE_QUAT_AND_MATRIX(quaternion, matrix2, 1.0f);
		COMPARE_MAT3(matrix1, matrix2, 1.0f);
		point1 = matrix1 * xBasis;
		point2 = quaternion.RotatePoint(xBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * yBasis;
		point2 = quaternion.RotatePoint(yBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * zBasis;
		point2 = quaternion.RotatePoint(zBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * allBasis;
		point2 = quaternion.RotatePoint(allBasis);
		COMPARE_VEC3(point1, point2, 3.0f);

		// y basis rotations
		quaternion = quat::Rotation(rad.value(), yBasis);
		matrix1 = mat3::Rotation(rad.value(), yBasis);
		COMPARE_QUAT_AND_MATRIX(quaternion, matrix1, 1.0f);
		matrix2 = mat3::RotationBasic(rad.value(), Axis::Y);
		COMPARE_QUAT_AND_MATRIX(quaternion, matrix2, 1.0f);
		COMPARE_MAT3(matrix1, matrix2, 1.0f);
		point1 = matrix1 * xBasis;
		point2 = quaternion.RotatePoint(xBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * yBasis;
		point2 = quaternion.RotatePoint(yBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * zBasis;
		point2 = quaternion.RotatePoint(zBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * allBasis;
		point2 = quaternion.RotatePoint(allBasis);
		COMPARE_VEC3(point1, point2, 3.0f);
		// negative
		quaternion = quat::Rotation(rad.value(), -yBasis);
		matrix1 = mat3::Rotation(rad.value(), -yBasis);
		COMPARE_QUAT_AND_MATRIX(quaternion, matrix1, 1.0f);
		matrix2 = mat3::RotationBasic(-rad.value(), Axis::Y);
		COMPARE_QUAT_AND_MATRIX(quaternion, matrix2, 1.0f);
		COMPARE_MAT3(matrix1, matrix2, 1.0f);
		point1 = matrix1 * xBasis;
		point2 = quaternion.RotatePoint(xBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * yBasis;
		point2 = quaternion.RotatePoint(yBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * zBasis;
		point2 = quaternion.RotatePoint(zBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * allBasis;
		point2 = quaternion.RotatePoint(allBasis);
		COMPARE_VEC3(point1, point2, 3.0f);

		// z basis rotations
		quaternion = quat::Rotation(rad.value(), zBasis);
		matrix1 = mat3::Rotation(rad.value(), zBasis);
		COMPARE_QUAT_AND_MATRIX(quaternion, matrix1, 1.0f);
		matrix2 = mat3::RotationBasic(rad.value(), Axis::Z);
		COMPARE_QUAT_AND_MATRIX(quaternion, matrix2, 1.0f);
		COMPARE_MAT3(matrix1, matrix2, 1.0f);
		point1 = matrix1 * xBasis;
		point2 = quaternion.RotatePoint(xBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * yBasis;
		point2 = quaternion.RotatePoint(yBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * zBasis;
		point2 = quaternion.RotatePoint(zBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * allBasis;
		point2 = quaternion.RotatePoint(allBasis);
		COMPARE_VEC3(point1, point2, 3.0f);
		// negative
		quaternion = quat::Rotation(rad.value(), -zBasis);
		matrix1 = mat3::Rotation(rad.value(), -zBasis);
		COMPARE_QUAT_AND_MATRIX(quaternion, matrix1, 1.0f);
		matrix2 = mat3::RotationBasic(-rad.value(), Axis::Z);
		COMPARE_QUAT_AND_MATRIX(quaternion, matrix2, 1.0f);
		COMPARE_MAT3(matrix1, matrix2, 1.0f);
		point1 = matrix1 * xBasis;
		point2 = quaternion.RotatePoint(xBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * yBasis;
		point2 = quaternion.RotatePoint(yBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * zBasis;
		point2 = quaternion.RotatePoint(zBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * allBasis;
		point2 = quaternion.RotatePoint(allBasis);
		COMPARE_VEC3(point1, point2, 3.0f);

		// all basis rotations
		quaternion = quat::Rotation(rad.value(), allBasis);
		matrix1 = mat3::Rotation(rad.value(), allBasis);
		COMPARE_QUAT_AND_MATRIX(quaternion, matrix1, allBasisMag);
		point1 = matrix1 * xBasis;
		point2 = quaternion.RotatePoint(xBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * yBasis;
		point2 = quaternion.RotatePoint(yBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * zBasis;
		point2 = quaternion.RotatePoint(zBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * allBasis;
		point2 = quaternion.RotatePoint(allBasis);
		COMPARE_VEC3(point1, point2, 3.0f);
		// negative
		quaternion = quat::Rotation(rad.value(), -allBasis);
		matrix1 = mat3::Rotation(rad.value(), -allBasis);
		COMPARE_QUAT_AND_MATRIX(quaternion, matrix1, allBasisMag);
		point1 = matrix1 * xBasis;
		point2 = quaternion.RotatePoint(xBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * yBasis;
		point2 = quaternion.RotatePoint(yBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * zBasis;
		point2 = quaternion.RotatePoint(zBasis);
		COMPARE_VEC3(point1, point2, 1.0f);
		point1 = matrix1 * allBasis;
		point2 = quaternion.RotatePoint(allBasis);
		COMPARE_VEC3(point1, point2, 3.0f);

		// Reciprocal test
		quaternion = quat::Rotation(rad.value(), xBasis);
		point1 = quaternion.RotatePoint(yBasis);
		point2 = quaternion.Reciprocal().RotatePoint(point1);
		COMPARE_VEC3(point2, yBasis, 1.0f);
	}

	{ // slerp
		quat a = quat(0.0f, xBasis);
		quat b = quat(0.0f, yBasis);
		quat actual;
		quat expected;
		actual = slerp(a, b, 0.0f);
		expected = quat(0.0f, xBasis);
		COMPARE_QUAT(actual, expected, 1.0f);

		actual = slerp(a, b, 1.0f);
		expected = quat(0.0f, yBasis);
		COMPARE_QUAT(actual, expected, 1.0f);

		actual = slerp(a, b, 2.0f);
		expected = quat(0.0f, -xBasis);
		COMPARE_QUAT(actual, expected, 1.0f);

		actual = slerp(a, b, -1.0f);
		expected = quat(0.0f, -yBasis);
		COMPARE_QUAT(actual, expected, 1.0f);

		actual = slerp(a, b, 0.25f);
		expected = quat(0.0f, cos(pi / 8.0f), sin(pi / 8.0f), 0.0f);
		COMPARE_QUAT(actual, expected, 1.0f);

		actual = slerp(a, b, 0.5f);
		expected = quat(0.0f, cos(pi / 4.0f), sin(pi / 4.0f), 0.0f);
		COMPARE_QUAT(actual, expected, 1.0f);

		actual = slerp(a, b, 0.75f);
		expected = quat(0.0f, cos(3.0f * pi / 8.0f), sin(3.0f * pi / 8.0f), 0.0f);
		COMPARE_QUAT(actual, expected, 1.0f);

		actual = slerp(a, b, 1.0f / 3.0f);
		expected = quat(0.0f, cos(pi / 6.0f), sin(pi / 6.0f), 0.0f);
		COMPARE_QUAT(actual, expected, 1.0f);

		actual = slerp(a, b, 2.0f / 3.0f);
		expected = quat(0.0f, cos(pi / 3.0f), sin(pi / 3.0f), 0.0f);
		COMPARE_QUAT(actual, expected, 1.0f);
	}

	fpError.Report(__LINE__);
}

} // namespace QuaternionTestNamespace