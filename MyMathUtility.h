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
};