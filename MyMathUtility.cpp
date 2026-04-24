#include "MyMathUtility.h" // 変更
#include <cmath>

using namespace KamataEngine;

// ※中身の計算式は変更していません。MathUtility:: を MyMathUtility:: にしただけです。
Matrix4x4 MyMathUtility::MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result{};
	result.m[0][0] = scale.x;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][1] = scale.y;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = scale.z;
	result.m[2][3] = 0.0f;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MyMathUtility::MakeRotationMatrix(const Vector3& rotation) {
	float sinX = std::sin(rotation.x), cosX = std::cos(rotation.x);
	float sinY = std::sin(rotation.y), cosY = std::cos(rotation.y);
	float sinZ = std::sin(rotation.z), cosZ = std::cos(rotation.z);

	Matrix4x4 result{};
	result.m[0][0] = cosY * cosZ;
	result.m[0][1] = cosY * sinZ;
	result.m[0][2] = -sinY;
	result.m[0][3] = 0.0f;

	result.m[1][0] = sinX * sinY * cosZ - cosX * sinZ;
	result.m[1][1] = sinX * sinY * sinZ + cosX * cosZ;
	result.m[1][2] = sinX * cosY;
	result.m[1][3] = 0.0f;

	result.m[2][0] = cosX * sinY * cosZ + sinX * sinZ;
	result.m[2][1] = cosX * sinY * sinZ - sinX * cosZ;
	result.m[2][2] = cosX * cosY;
	result.m[2][3] = 0.0f;

	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MyMathUtility::MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result{};
	result.m[0][0] = 1.0f;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][1] = 1.0f;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f;
	result.m[2][3] = 0.0f;
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MyMathUtility::Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m1.m[i][0] * m2.m[0][j] + m1.m[i][1] * m2.m[1][j] + m1.m[i][2] * m2.m[2][j] + m1.m[i][3] * m2.m[3][j];
		}
	}
	return result;
}

// ========== ここから追加分 ==========

// 加算
Matrix4x4 MyMathUtility::Add(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m1.m[i][j] + m2.m[i][j];
		}
	}
	return result;
}

// 減算
Matrix4x4 MyMathUtility::Subtract(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m1.m[i][j] - m2.m[i][j];
		}
	}
	return result;
}

// 逆行列
Matrix4x4 MyMathUtility::Inverse(const Matrix4x4& m) {
	Matrix4x4 result{};
	float det = m.m[0][0] * (m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[2][1] * m.m[3][2] * m.m[1][3] + m.m[3][1] * m.m[1][2] * m.m[2][3] - m.m[3][1] * m.m[2][2] * m.m[1][3] -
	                         m.m[2][1] * m.m[1][2] * m.m[3][3] - m.m[1][1] * m.m[3][2] * m.m[2][3]) -
	            m.m[0][1] * (m.m[1][0] * m.m[2][2] * m.m[3][3] + m.m[2][0] * m.m[3][2] * m.m[1][3] + m.m[3][0] * m.m[1][2] * m.m[2][3] - m.m[3][0] * m.m[2][2] * m.m[1][3] -
	                         m.m[2][0] * m.m[1][2] * m.m[3][3] - m.m[1][0] * m.m[3][2] * m.m[2][3]) +
	            m.m[0][2] * (m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[2][0] * m.m[3][1] * m.m[1][3] + m.m[3][0] * m.m[1][1] * m.m[2][3] - m.m[3][0] * m.m[2][1] * m.m[1][3] -
	                         m.m[2][0] * m.m[1][1] * m.m[3][3] - m.m[1][0] * m.m[3][1] * m.m[2][3]) -
	            m.m[0][3] * (m.m[1][0] * m.m[2][1] * m.m[3][2] + m.m[2][0] * m.m[3][1] * m.m[1][2] + m.m[3][0] * m.m[1][1] * m.m[2][2] - m.m[3][0] * m.m[2][1] * m.m[1][2] -
	                         m.m[2][0] * m.m[1][1] * m.m[3][2] - m.m[1][0] * m.m[3][1] * m.m[2][2]);

	if (det != 0.0f) {
		float invDet = 1.0f / det;
		result.m[0][0] = (m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[2][1] * m.m[3][2] * m.m[1][3] + m.m[3][1] * m.m[1][2] * m.m[2][3] - m.m[3][1] * m.m[2][2] * m.m[1][3] -
		                  m.m[2][1] * m.m[1][2] * m.m[3][3] - m.m[1][1] * m.m[3][2] * m.m[2][3]) *
		                 invDet;
		result.m[0][1] = -(m.m[0][1] * m.m[2][2] * m.m[3][3] + m.m[2][1] * m.m[3][2] * m.m[0][3] + m.m[3][1] * m.m[0][2] * m.m[2][3] - m.m[3][1] * m.m[2][2] * m.m[0][3] -
		                   m.m[2][1] * m.m[0][2] * m.m[3][3] - m.m[0][1] * m.m[3][2] * m.m[2][3]) *
		                 invDet;
		result.m[0][2] = (m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[1][1] * m.m[3][2] * m.m[0][3] + m.m[3][1] * m.m[0][2] * m.m[1][3] - m.m[3][1] * m.m[1][2] * m.m[0][3] -
		                  m.m[1][1] * m.m[0][2] * m.m[3][3] - m.m[0][1] * m.m[3][2] * m.m[1][3]) *
		                 invDet;
		result.m[0][3] = -(m.m[0][1] * m.m[1][2] * m.m[2][3] + m.m[1][1] * m.m[2][2] * m.m[0][3] + m.m[2][1] * m.m[0][2] * m.m[1][3] - m.m[2][1] * m.m[1][2] * m.m[0][3] -
		                   m.m[1][1] * m.m[0][2] * m.m[2][3] - m.m[0][1] * m.m[2][2] * m.m[1][3]) *
		                 invDet;

		result.m[1][0] = -(m.m[1][0] * m.m[2][2] * m.m[3][3] + m.m[2][0] * m.m[3][2] * m.m[1][3] + m.m[3][0] * m.m[1][2] * m.m[2][3] - m.m[3][0] * m.m[2][2] * m.m[1][3] -
		                   m.m[2][0] * m.m[1][2] * m.m[3][3] - m.m[1][0] * m.m[3][2] * m.m[2][3]) *
		                 invDet;
		result.m[1][1] = (m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[2][0] * m.m[3][2] * m.m[0][3] + m.m[3][0] * m.m[0][2] * m.m[2][3] - m.m[3][0] * m.m[2][2] * m.m[0][3] -
		                  m.m[2][0] * m.m[0][2] * m.m[3][3] - m.m[0][0] * m.m[3][2] * m.m[2][3]) *
		                 invDet;
		result.m[1][2] = -(m.m[0][0] * m.m[1][2] * m.m[3][3] + m.m[1][0] * m.m[3][2] * m.m[0][3] + m.m[3][0] * m.m[0][2] * m.m[1][3] - m.m[3][0] * m.m[1][2] * m.m[0][3] -
		                   m.m[1][0] * m.m[0][2] * m.m[3][3] - m.m[0][0] * m.m[3][2] * m.m[1][3]) *
		                 invDet;
		result.m[1][3] = (m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[1][0] * m.m[2][2] * m.m[0][3] + m.m[2][0] * m.m[0][2] * m.m[1][3] - m.m[2][0] * m.m[1][2] * m.m[0][3] -
		                  m.m[1][0] * m.m[0][2] * m.m[2][3] - m.m[0][0] * m.m[2][2] * m.m[1][3]) *
		                 invDet;

		result.m[2][0] = (m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[2][0] * m.m[3][1] * m.m[1][3] + m.m[3][0] * m.m[1][1] * m.m[2][3] - m.m[3][0] * m.m[2][1] * m.m[1][3] -
		                  m.m[2][0] * m.m[1][1] * m.m[3][3] - m.m[1][0] * m.m[3][1] * m.m[2][3]) *
		                 invDet;
		result.m[2][1] = -(m.m[0][0] * m.m[2][1] * m.m[3][3] + m.m[2][0] * m.m[3][1] * m.m[0][3] + m.m[3][0] * m.m[0][1] * m.m[2][3] - m.m[3][0] * m.m[2][1] * m.m[0][3] -
		                   m.m[2][0] * m.m[0][1] * m.m[3][3] - m.m[0][0] * m.m[3][1] * m.m[2][3]) *
		                 invDet;
		result.m[2][2] = (m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[1][0] * m.m[3][1] * m.m[0][3] + m.m[3][0] * m.m[0][1] * m.m[1][3] - m.m[3][0] * m.m[1][1] * m.m[0][3] -
		                  m.m[1][0] * m.m[0][1] * m.m[3][3] - m.m[0][0] * m.m[3][1] * m.m[1][3]) *
		                 invDet;
		result.m[2][3] = -(m.m[0][0] * m.m[1][1] * m.m[2][3] + m.m[1][0] * m.m[2][1] * m.m[0][3] + m.m[2][0] * m.m[0][1] * m.m[1][3] - m.m[2][0] * m.m[1][1] * m.m[0][3] -
		                   m.m[1][0] * m.m[0][1] * m.m[2][3] - m.m[0][0] * m.m[2][1] * m.m[1][3]) *
		                 invDet;

		result.m[3][0] = -(m.m[1][0] * m.m[2][1] * m.m[3][2] + m.m[2][0] * m.m[3][1] * m.m[1][2] + m.m[3][0] * m.m[1][1] * m.m[2][2] - m.m[3][0] * m.m[2][1] * m.m[1][2] -
		                   m.m[2][0] * m.m[1][1] * m.m[3][2] - m.m[1][0] * m.m[3][1] * m.m[2][2]) *
		                 invDet;
		result.m[3][1] = (m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[2][0] * m.m[3][1] * m.m[0][2] + m.m[3][0] * m.m[0][1] * m.m[2][2] - m.m[3][0] * m.m[2][1] * m.m[0][2] -
		                  m.m[2][0] * m.m[0][1] * m.m[3][2] - m.m[0][0] * m.m[3][1] * m.m[2][2]) *
		                 invDet;
		result.m[3][2] = -(m.m[0][0] * m.m[1][1] * m.m[3][2] + m.m[1][0] * m.m[3][1] * m.m[0][2] + m.m[3][0] * m.m[0][1] * m.m[1][2] - m.m[3][0] * m.m[1][1] * m.m[0][2] -
		                   m.m[1][0] * m.m[0][1] * m.m[3][2] - m.m[0][0] * m.m[3][1] * m.m[1][2]) *
		                 invDet;
		result.m[3][3] = (m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[1][0] * m.m[2][1] * m.m[0][2] + m.m[2][0] * m.m[0][1] * m.m[1][2] - m.m[2][0] * m.m[1][1] * m.m[0][2] -
		                  m.m[1][0] * m.m[0][1] * m.m[2][2] - m.m[0][0] * m.m[2][1] * m.m[1][2]) *
		                 invDet;
	}
	return result;
}

// 転置行列
Matrix4x4 MyMathUtility::Transpose(const Matrix4x4& m) {
	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m.m[j][i];
		}
	}
	return result;
}

// 単位行列の作成
Matrix4x4 MyMathUtility::MakeIdentity4x4() {
	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
	return result;
}

// 座標変換
Vector3 MyMathUtility::Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result{};
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];

	assert(w != 0.0f); // wが0になるとゼロ除算が発生するためストップさせる

	result.x /= w;
	result.y /= w;
	result.z /= w;

	return result;
}