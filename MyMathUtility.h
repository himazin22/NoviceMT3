#pragma once
#include "KamataEngine.h"

// 名前が被らないように MyMathUtility に変更
class MyMathUtility {
public:
	static KamataEngine::Matrix4x4 MakeScaleMatrix(const KamataEngine::Vector3& scale);
	static KamataEngine::Matrix4x4 MakeRotationMatrix(const KamataEngine::Vector3& rotation);
	static KamataEngine::Matrix4x4 MakeTranslateMatrix(const KamataEngine::Vector3& translate);
	static KamataEngine::Matrix4x4 Multiply(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);

	static KamataEngine::Matrix4x4 Add(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);
	static KamataEngine::Matrix4x4 Subtract(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);
	static KamataEngine::Matrix4x4 Inverse(const KamataEngine::Matrix4x4& m);
	static KamataEngine::Matrix4x4 Transpose(const KamataEngine::Matrix4x4& m);
	static KamataEngine::Matrix4x4 MakeIdentity4x4();
	static KamataEngine::Vector3 Transform(const KamataEngine::Vector3& vector, const KamataEngine::Matrix4x4& matrix);

	static KamataEngine::Matrix4x4 MakeRotateXMatrix(float radian);
	static KamataEngine::Matrix4x4 MakeRotateYMatrix(float radian);
	static KamataEngine::Matrix4x4 MakeRotateZMatrix(float radian);

	static KamataEngine::Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rotate, const KamataEngine::Vector3& translate);

	static KamataEngine::Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
	static KamataEngine::Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);
	static KamataEngine::Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
	// 内積
	static float Dot(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);
	// ベクトルの減算
	static KamataEngine::Vector3 Subtract(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);
	// ベクトルの加算
	static KamataEngine::Vector3 Add(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);
	// ベクトルの長さの二乗
	static float LengthSquared(const KamataEngine::Vector3& v);
};