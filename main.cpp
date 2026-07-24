#include <Novice.h>
#define _USE_MATH_DEFINES
#include "MyMathUtility.h"
#include <algorithm>
#include <assert.h>
#include <imgui.h>
#include <math.h>

using namespace KamataEngine;

const int kWindowWidth = 1280;
const int kWindowHeight = 720;

struct Sphere {
	Vector3 center;
	float radius;
};

struct Plane {
	Vector3 normal;
	float distance;
};

struct Triangle {
	Vector3 Vvertices[3];
};

struct AABB {
	Vector3 min;
	Vector3 max;
};

struct OBB {
	Vector3 center;
	Vector3 orientations[3];
	Vector3 size;
};

struct Spring {
	Vector3 anchor;
	float naturalLength;
	float stiffness;
	float dampingCoefficient;
};

struct Ball {
	Vector3 position;
	Vector3 velocity;
	Vector3 acceleration;
	float mass;
	float radius;
	unsigned int color;
};

struct Pendulum {
	Vector3 anchor;
	float length;
	float angle;
	float angularVelocity;
	float angularAcceleration;
};

struct ConicalPendulum {
	Vector3 anchor;
	float length;
	float halfApexAngle;
	float angle;
	float angularVelocity;
};

// ===================================
// 演算子オーバーロードの定義
// ===================================

// Vector3 の単項マイナス (-Vector3)
Vector3 operator-(const Vector3& v) { return {-v.x, -v.y, -v.z}; }

// Vector3 の加算 (Vector3 + Vector3)
Vector3 operator+(const Vector3& v1, const Vector3& v2) { return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z}; }

// Vector3 の減算 (Vector3 - Vector3)
Vector3 operator-(const Vector3& v1, const Vector3& v2) { return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}; }

// Vector3 のスカラー倍 (Vector3 * float)
Vector3 operator*(const Vector3& v, float s) { return {v.x * s, v.y * s, v.z * s}; }

// Vector3 のスカラー倍 (float * Vector3)
Vector3 operator*(float s, const Vector3& v) { return {v.x * s, v.y * s, v.z * s}; }

// Vector3 のスカラー除算 (Vector3 / float)
Vector3 operator/(const Vector3& v, float s) { return {v.x / s, v.y / s, v.z / s}; }

// Matrix4x4 の加算 (Matrix4x4 + Matrix4x4)
Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2) { return MyMathUtility::Add(m1, m2); }

// Matrix4x4 の減算 (Matrix4x4 - Matrix4x4)
Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2) { return MyMathUtility::Subtract(m1, m2); }

// Matrix4x4 の積 (Matrix4x4 * Matrix4x4)
Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2) { return MyMathUtility::Multiply(m1, m2); }

// 線形補間関数
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) {
	Vector3 result;
	result.x = v1.x + (v2.x - v1.x) * t;
	result.y = v1.y + (v2.y - v1.y) * t;
	result.z = v1.z + (v2.z - v1.z) * t;
	return result;
}

// 2次ベジェ曲線の描画関数
void DrawBezier(const Vector3& controlPoint0, const Vector3& controlPoint1, const Vector3& controlPoint2, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const int kSubdivision = 32; // 曲線の滑らかさを決める分割数
	Vector3 prevPoint = controlPoint0;

	for (int i = 1; i <= kSubdivision; ++i) {
		float t = static_cast<float>(i) / kSubdivision;

		// 制御点p0, p1を線形補間
		Vector3 p0p1 = Lerp(controlPoint0, controlPoint1, t);
		// 制御点p1, p2を線形補間
		Vector3 p1p2 = Lerp(controlPoint1, controlPoint2, t);
		// 補間点p0p1, p1p2をさらに線形補間して曲線上の点pを求める
		Vector3 p = Lerp(p0p1, p1p2, t);

		// スクリーン座標に変換
		Vector3 screenPrev = MyMathUtility::Transform(MyMathUtility::Transform(prevPoint, viewProjectionMatrix), viewportMatrix);
		Vector3 screenP = MyMathUtility::Transform(MyMathUtility::Transform(p, viewProjectionMatrix), viewportMatrix);

		// 直前の点から現在の点へ線を引く
		Novice::DrawLine(static_cast<int>(screenPrev.x), static_cast<int>(screenPrev.y), static_cast<int>(screenP.x), static_cast<int>(screenP.y), color);

		prevPoint = p;
	}
}

// 球と球の衝突判定関数
bool IsCollision(const Sphere& s1, const Sphere& s2) {
	float distance = MyMathUtility::Length(MyMathUtility::Subtract(s2.center, s1.center));
	if (distance <= (s1.radius + s2.radius)) {
		return true;
	}
	return false;
}

bool IsCollision(const Sphere& s1, const Plane& p1) {
	float dot = MyMathUtility::Dot(s1.center, p1.normal);
	float distance = std::abs(dot - p1.distance);
	if (distance <= s1.radius) {
		return true;
	}
	return false;
}

// 線分と平面の衝突判定関数
bool IsCollision(const Segment& segment, const Plane& plane) {
	float dot = MyMathUtility::Dot(plane.normal, segment.diff);
	if (dot == 0.0f) {
		return false;
	}
	float t = (plane.distance - MyMathUtility::Dot(segment.origin, plane.normal)) / dot;
	if (t >= 0.0f && t <= 1.0f) {
		return true;
	}
	return false;
}

// AABBとAABBの衝突判定
bool IsCollision(const AABB& aabb1, const AABB& aabb2) {
	if ((aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) && (aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) && (aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z)) {
		return true;
	}
	return false;
}

// 三角形と線分の衝突判定関数
bool IsCollision(const Triangle& triangle, const Segment& segment) {
	Vector3 v0 = triangle.Vvertices[0];
	Vector3 v1 = triangle.Vvertices[1];
	Vector3 v2 = triangle.Vvertices[2];

	Vector3 v01 = MyMathUtility::Subtract(v1, v0);
	Vector3 v12 = MyMathUtility::Subtract(v2, v1);
	Vector3 v20 = MyMathUtility::Subtract(v0, v2);

	Vector3 v02 = MyMathUtility::Subtract(v2, v0);
	Vector3 normal = MyMathUtility::Normalize(MyMathUtility::Cross(v01, v02));
	float distance = MyMathUtility::Dot(normal, v0);

	float dot = MyMathUtility::Dot(normal, segment.diff);

	if (dot == 0.0f) {
		return false;
	}

	float t = (distance - MyMathUtility::Dot(segment.origin, normal)) / dot;

	if (t < 0.0f || t > 1.0f) {
		return false;
	}

	Vector3 p = MyMathUtility::Add(segment.origin, MyMathUtility::Multiply(t, segment.diff));

	Vector3 v0p = MyMathUtility::Subtract(p, v0);
	Vector3 v1p = MyMathUtility::Subtract(p, v1);
	Vector3 v2p = MyMathUtility::Subtract(p, v2);

	Vector3 cross01 = MyMathUtility::Cross(v01, v1p);
	Vector3 cross12 = MyMathUtility::Cross(v12, v2p);
	Vector3 cross20 = MyMathUtility::Cross(v20, v0p);

	if (MyMathUtility::Dot(cross01, normal) >= 0.0f && MyMathUtility::Dot(cross12, normal) >= 0.0f && MyMathUtility::Dot(cross20, normal) >= 0.0f) {
		return true;
	}

	return false;
}

bool IsCollision(const AABB& aabb, const Sphere& sphere) {
	Vector3 closestPoint;
	closestPoint.x = (std::clamp)(sphere.center.x, aabb.min.x, aabb.max.x);
	closestPoint.y = (std::clamp)(sphere.center.y, aabb.min.y, aabb.max.y);
	closestPoint.z = (std::clamp)(sphere.center.z, aabb.min.z, aabb.max.z);

	float distance = MyMathUtility::Length(MyMathUtility::Subtract(closestPoint, sphere.center));

	if (distance <= sphere.radius) {
		return true;
	}
	return false;
}

bool IsCollision(const AABB& aabb, const Segment& segment) {
	float tMin = 0.0f;
	float tMax = 1.0f;

	if (std::abs(segment.diff.x) < 0.0001f) {
		if (segment.origin.x < aabb.min.x || segment.origin.x > aabb.max.x) {
			return false;
		}
	} else {
		float t1 = (aabb.min.x - segment.origin.x) / segment.diff.x;
		float t2 = (aabb.max.x - segment.origin.x) / segment.diff.x;
		float tNear = (std::min)(t1, t2);
		float tFar = (std::max)(t1, t2);
		tMin = (std::max)(tMin, tNear);
		tMax = (std::min)(tMax, tFar);
		if (tMin > tMax)
			return false;
	}

	if (std::abs(segment.diff.y) < 0.0001f) {
		if (segment.origin.y < aabb.min.y || segment.origin.y > aabb.max.y) {
			return false;
		}
	} else {
		float t1 = (aabb.min.y - segment.origin.y) / segment.diff.y;
		float t2 = (aabb.max.y - segment.origin.y) / segment.diff.y;
		float tNear = (std::min)(t1, t2);
		float tFar = (std::max)(t1, t2);
		tMin = (std::max)(tMin, tNear);
		tMax = (std::min)(tMax, tFar);
		if (tMin > tMax)
			return false;
	}

	if (std::abs(segment.diff.z) < 0.0001f) {
		if (segment.origin.z < aabb.min.z || segment.origin.z > aabb.max.z) {
			return false;
		}
	} else {
		float t1 = (aabb.min.z - segment.origin.z) / segment.diff.z;
		float t2 = (aabb.max.z - segment.origin.z) / segment.diff.z;
		float tNear = (std::min)(t1, t2);
		float tFar = (std::max)(t1, t2);
		tMin = (std::max)(tMin, tNear);
		tMax = (std::min)(tMax, tFar);
		if (tMin > tMax)
			return false;
	}

	return true;
}

bool IsCollision(const OBB& obb, const Sphere& sphere) {
	Vector3 d = MyMathUtility::Subtract(sphere.center, obb.center);
	Vector3 closestPoint = obb.center;
	float sizes[3] = {obb.size.x, obb.size.y, obb.size.z};

	for (int i = 0; i < 3; ++i) {
		float dist = MyMathUtility::Dot(d, obb.orientations[i]);
		dist = std::clamp(dist, -sizes[i], sizes[i]);
		closestPoint.x += obb.orientations[i].x * dist;
		closestPoint.y += obb.orientations[i].y * dist;
		closestPoint.z += obb.orientations[i].z * dist;
	}

	Vector3 diff = MyMathUtility::Subtract(closestPoint, sphere.center);
	float distanceSquared = MyMathUtility::LengthSquared(diff);
	return distanceSquared <= (sphere.radius * sphere.radius);
}

bool IsCollision(const Segment& segment, const OBB& obb) {
	Vector3 localOrigin = MyMathUtility::Subtract(segment.origin, obb.center);

	Segment localSegment;
	localSegment.origin.x = MyMathUtility::Dot(localOrigin, obb.orientations[0]);
	localSegment.origin.y = MyMathUtility::Dot(localOrigin, obb.orientations[1]);
	localSegment.origin.z = MyMathUtility::Dot(localOrigin, obb.orientations[2]);

	localSegment.diff.x = MyMathUtility::Dot(segment.diff, obb.orientations[0]);
	localSegment.diff.y = MyMathUtility::Dot(segment.diff, obb.orientations[1]);
	localSegment.diff.z = MyMathUtility::Dot(segment.diff, obb.orientations[2]);

	AABB localAABB;
	localAABB.min = {-obb.size.x, -obb.size.y, -obb.size.z};
	localAABB.max = {obb.size.x, obb.size.y, obb.size.z};

	return IsCollision(localAABB, localSegment);
}

bool TestAxis(const Vector3& obb1Center, const OBB& obb1, const OBB& obb2, const Vector3& axis) {
	float axisLenSq = MyMathUtility::LengthSquared(axis);
	if (axisLenSq < 0.0001f) {
		return false;
	}

	Vector3 n = MyMathUtility::Normalize(axis);

	float centerDist = std::abs(MyMathUtility::Dot(MyMathUtility::Subtract(obb2.center, obb1Center), n));

	float r1 = std::abs(MyMathUtility::Dot(MyMathUtility::Multiply(obb1.size.x, obb1.orientations[0]), n)) +
	           std::abs(MyMathUtility::Dot(MyMathUtility::Multiply(obb1.size.y, obb1.orientations[1]), n)) +
	           std::abs(MyMathUtility::Dot(MyMathUtility::Multiply(obb1.size.z, obb1.orientations[2]), n));

	float r2 = std::abs(MyMathUtility::Dot(MyMathUtility::Multiply(obb2.size.x, obb2.orientations[0]), n)) +
	           std::abs(MyMathUtility::Dot(MyMathUtility::Multiply(obb2.size.y, obb2.orientations[1]), n)) +
	           std::abs(MyMathUtility::Dot(MyMathUtility::Multiply(obb2.size.z, obb2.orientations[2]), n));

	return centerDist > (r1 + r2);
}

bool IsCollision(const OBB& obb1, const OBB& obb2) {
	const Vector3& A0 = obb1.orientations[0];
	const Vector3& A1 = obb1.orientations[1];
	const Vector3& A2 = obb1.orientations[2];

	const Vector3& B0 = obb2.orientations[0];
	const Vector3& B1 = obb2.orientations[1];
	const Vector3& B2 = obb2.orientations[2];

	if (TestAxis(obb1.center, obb1, obb2, A0))
		return false;
	if (TestAxis(obb1.center, obb1, obb2, A1))
		return false;
	if (TestAxis(obb1.center, obb1, obb2, A2))
		return false;

	if (TestAxis(obb1.center, obb1, obb2, B0))
		return false;
	if (TestAxis(obb1.center, obb1, obb2, B1))
		return false;
	if (TestAxis(obb1.center, obb1, obb2, B2))
		return false;

	if (TestAxis(obb1.center, obb1, obb2, MyMathUtility::Cross(A0, B0)))
		return false;
	if (TestAxis(obb1.center, obb1, obb2, MyMathUtility::Cross(A0, B1)))
		return false;
	if (TestAxis(obb1.center, obb1, obb2, MyMathUtility::Cross(A0, B2)))
		return false;

	if (TestAxis(obb1.center, obb1, obb2, MyMathUtility::Cross(A1, B0)))
		return false;
	if (TestAxis(obb1.center, obb1, obb2, MyMathUtility::Cross(A1, B1)))
		return false;
	if (TestAxis(obb1.center, obb1, obb2, MyMathUtility::Cross(A1, B2)))
		return false;

	if (TestAxis(obb1.center, obb1, obb2, MyMathUtility::Cross(A2, B0)))
		return false;
	if (TestAxis(obb1.center, obb1, obb2, MyMathUtility::Cross(A2, B1)))
		return false;
	if (TestAxis(obb1.center, obb1, obb2, MyMathUtility::Cross(A2, B2)))
		return false;

	return true;
}

void DrawOBB(const OBB& obb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 localVertices[8] = {
	    {-obb.size.x, -obb.size.y, -obb.size.z},
        {obb.size.x,  -obb.size.y, -obb.size.z},
        {-obb.size.x, obb.size.y,  -obb.size.z},
        {obb.size.x,  obb.size.y,  -obb.size.z},
	    {-obb.size.x, -obb.size.y, obb.size.z },
        {obb.size.x,  -obb.size.y, obb.size.z },
        {-obb.size.x, obb.size.y,  obb.size.z },
        {obb.size.x,  obb.size.y,  obb.size.z },
	};

	Vector3 worldVertices[8];
	Vector3 screenVertices[8];

	for (int i = 0; i < 8; ++i) {
		worldVertices[i] = obb.center;
		worldVertices[i].x += obb.orientations[0].x * localVertices[i].x + obb.orientations[1].x * localVertices[i].y + obb.orientations[2].x * localVertices[i].z;
		worldVertices[i].y += obb.orientations[0].y * localVertices[i].x + obb.orientations[1].y * localVertices[i].y + obb.orientations[2].y * localVertices[i].z;
		worldVertices[i].z += obb.orientations[0].z * localVertices[i].x + obb.orientations[1].z * localVertices[i].y + obb.orientations[2].z * localVertices[i].z;

		Vector3 ndcVertex = MyMathUtility::Transform(worldVertices[i], viewProjectionMatrix);
		screenVertices[i] = MyMathUtility::Transform(ndcVertex, viewportMatrix);
	}

	Novice::DrawLine((int)screenVertices[0].x, (int)screenVertices[0].y, (int)screenVertices[1].x, (int)screenVertices[1].y, color);
	Novice::DrawLine((int)screenVertices[1].x, (int)screenVertices[1].y, (int)screenVertices[3].x, (int)screenVertices[3].y, color);
	Novice::DrawLine((int)screenVertices[3].x, (int)screenVertices[3].y, (int)screenVertices[2].x, (int)screenVertices[2].y, color);
	Novice::DrawLine((int)screenVertices[2].x, (int)screenVertices[2].y, (int)screenVertices[0].x, (int)screenVertices[0].y, color);

	Novice::DrawLine((int)screenVertices[4].x, (int)screenVertices[4].y, (int)screenVertices[5].x, (int)screenVertices[5].y, color);
	Novice::DrawLine((int)screenVertices[5].x, (int)screenVertices[5].y, (int)screenVertices[7].x, (int)screenVertices[7].y, color);
	Novice::DrawLine((int)screenVertices[7].x, (int)screenVertices[7].y, (int)screenVertices[6].x, (int)screenVertices[6].y, color);
	Novice::DrawLine((int)screenVertices[6].x, (int)screenVertices[6].y, (int)screenVertices[4].x, (int)screenVertices[4].y, color);

	Novice::DrawLine((int)screenVertices[0].x, (int)screenVertices[0].y, (int)screenVertices[4].x, (int)screenVertices[4].y, color);
	Novice::DrawLine((int)screenVertices[1].x, (int)screenVertices[1].y, (int)screenVertices[5].x, (int)screenVertices[5].y, color);
	Novice::DrawLine((int)screenVertices[2].x, (int)screenVertices[2].y, (int)screenVertices[6].x, (int)screenVertices[6].y, color);
	Novice::DrawLine((int)screenVertices[3].x, (int)screenVertices[3].y, (int)screenVertices[7].x, (int)screenVertices[7].y, color);
}

void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 vertices[8] = {
	    {aabb.min.x, aabb.min.y, aabb.min.z}, // 0
	    {aabb.max.x, aabb.min.y, aabb.min.z}, // 1
	    {aabb.min.x, aabb.max.y, aabb.min.z}, // 2
	    {aabb.max.x, aabb.max.y, aabb.min.z}, // 3
	    {aabb.min.x, aabb.min.y, aabb.max.z}, // 4
	    {aabb.max.x, aabb.min.y, aabb.max.z}, // 5
	    {aabb.min.x, aabb.max.y, aabb.max.z}, // 6
	    {aabb.max.x, aabb.max.y, aabb.max.z}  // 7
	};

	Vector3 screenVertices[8];
	for (int i = 0; i < 8; ++i) {
		Matrix4x4 wvpVpMatrix = MyMathUtility::Multiply(viewProjectionMatrix, viewportMatrix);
		screenVertices[i] = MyMathUtility::Transform(vertices[i], wvpVpMatrix);
	}

	int indices[12][2] = {
	    {0, 1},
        {1, 3},
        {3, 2},
        {2, 0},
        {4, 5},
        {5, 7},
        {7, 6},
        {6, 4},
        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7}
    };

	for (int i = 0; i < 12; ++i) {
		Novice::DrawLine(
		    static_cast<int>(screenVertices[indices[i][0]].x), static_cast<int>(screenVertices[indices[i][0]].y), static_cast<int>(screenVertices[indices[i][1]].x),
		    static_cast<int>(screenVertices[indices[i][1]].y), color);
	}
}

void DrawTriangle(const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 screenVertices[3];
	for (int i = 0; i < 3; ++i) {
		Vector3 ndc = MyMathUtility::Transform(triangle.Vvertices[i], viewProjectionMatrix);
		screenVertices[i] = MyMathUtility::Transform(ndc, viewportMatrix);
	}

	Novice::DrawTriangle(
	    int(screenVertices[0].x), int(screenVertices[0].y), int(screenVertices[1].x), int(screenVertices[1].y), int(screenVertices[2].x), int(screenVertices[2].y), color, kFillModeWireFrame);
}

void DrawSegment(const Segment& segment, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 start = segment.origin;
	Vector3 end = MyMathUtility::Add(segment.origin, segment.diff);

	Vector3 screenStart = MyMathUtility::Transform(MyMathUtility::Transform(start, viewProjectionMatrix), viewportMatrix);
	Vector3 screenEnd = MyMathUtility::Transform(MyMathUtility::Transform(end, viewProjectionMatrix), viewportMatrix);

	Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
}

Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return {-vector.y, vector.x, 0.0f};
	}
	return {0.0f, -vector.z, vector.y};
}

void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 center = MyMathUtility::Multiply(plane.distance, plane.normal);
	Vector3 perpendiculars[4];
	perpendiculars[0] = MyMathUtility::Normalize(Perpendicular(plane.normal));
	perpendiculars[1] = {-perpendiculars[0].x, -perpendiculars[0].y, -perpendiculars[0].z};
	perpendiculars[2] = MyMathUtility::Cross(plane.normal, perpendiculars[0]);
	perpendiculars[3] = {-perpendiculars[2].x, -perpendiculars[2].y, -perpendiculars[2].z};

	Vector3 points[4];
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = MyMathUtility::Multiply(2.0f, perpendiculars[index]);
		Vector3 point = MyMathUtility::Add(center, extend);
		points[index] = MyMathUtility::Transform(MyMathUtility::Transform(point, viewProjectionMatrix), viewportMatrix);
	}

	Novice::DrawLine(int(points[0].x), int(points[0].y), int(points[2].x), int(points[2].y), color);
	Novice::DrawLine(int(points[2].x), int(points[2].y), int(points[1].x), int(points[1].y), color);
	Novice::DrawLine(int(points[1].x), int(points[1].y), int(points[3].x), int(points[3].y), color);
	Novice::DrawLine(int(points[3].x), int(points[3].y), int(points[0].x), int(points[0].y), color);
}

void DrawMiniSphere(const Sphere& sphere, const Matrix4x4& viewProjectMatrix, const Matrix4x4& viewPortMatrix, uint32_t color) {
	const uint32_t kSubdivision = 12;
	const float kLonEvery = 2.0f * float(M_PI) / float(kSubdivision);
	const float kLatEvery = float(M_PI) / float(kSubdivision);

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -float(M_PI) / 2.0f + kLatEvery * latIndex;
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;
			Vector3 a = {sphere.radius * cosf(lat) * cosf(lon) + sphere.center.x, sphere.radius * sinf(lat) + sphere.center.y, sphere.radius * cosf(lat) * sinf(lon) + sphere.center.z};

			float nextLat = lat + kLatEvery;
			Vector3 b = {sphere.radius * cosf(nextLat) * cosf(lon) + sphere.center.x, sphere.radius * sinf(nextLat) + sphere.center.y, sphere.radius * cosf(nextLat) * sinf(lon) + sphere.center.z};

			float nextLon = lon + kLonEvery;
			Vector3 c = {sphere.radius * cosf(lat) * cosf(nextLon) + sphere.center.x, sphere.radius * sinf(lat) + sphere.center.y, sphere.radius * cosf(lat) * sinf(nextLon) + sphere.center.z};

			Vector3 screenA = MyMathUtility::Transform(MyMathUtility::Transform(a, viewProjectMatrix), viewPortMatrix);
			Vector3 screenB = MyMathUtility::Transform(MyMathUtility::Transform(b, viewProjectMatrix), viewPortMatrix);
			Vector3 screenC = MyMathUtility::Transform(MyMathUtility::Transform(c, viewProjectMatrix), viewPortMatrix);

			Novice::DrawLine(int(screenA.x), int(screenA.y), int(screenB.x), int(screenB.y), color);
			Novice::DrawLine(int(screenA.x), int(screenA.y), int(screenC.x), int(screenC.y), color);
		}
	}
}

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 5.0f;
	const uint32_t kSubdivision = 10;
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);

	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		float x = -kGridHalfWidth + xIndex * kGridEvery;
		Vector3 start = {x, 0.0f, -kGridHalfWidth};
		Vector3 end = {x, 0.0f, kGridHalfWidth};

		Vector3 ndcStart = MyMathUtility::Transform(start, viewProjectionMatrix);
		Vector3 screenStart = MyMathUtility::Transform(ndcStart, viewportMatrix);

		Vector3 ndcEnd = MyMathUtility::Transform(end, viewProjectionMatrix);
		Vector3 screenEnd = MyMathUtility::Transform(ndcEnd, viewportMatrix);

		uint32_t color = 0xAAAAAAFF;
		Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
	}

	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		float z = -kGridHalfWidth + zIndex * kGridEvery;
		Vector3 start = {-kGridHalfWidth, 0.0f, z};
		Vector3 end = {kGridHalfWidth, 0.0f, z};

		Vector3 ndcStart = MyMathUtility::Transform(start, viewProjectionMatrix);
		Vector3 screenStart = MyMathUtility::Transform(ndcStart, viewportMatrix);

		Vector3 ndcEnd = MyMathUtility::Transform(end, viewProjectionMatrix);
		Vector3 screenEnd = MyMathUtility::Transform(ndcEnd, viewportMatrix);

		uint32_t color = 0xAAAAAAFF;
		Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
	}
}

Vector3 Reflect(const Vector3& input, const Vector3& normal) { return input - 2.0f * MyMathUtility::Dot(input, normal) * normal; }

const char kWindowTitle[] = "LC1C_22_ツノダ_タケマサ_タイトル";

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Novice::Initialize(kWindowTitle, 1280, 720);

	char keys[256] = {0};
	char preKeys[256] = {0};

	// マウス位置保持用
	int currentMouseX = 0;
	int currentMouseY = 0;
	int prevMouseX = 0;
	int prevMouseY = 0;
	bool isFirstClick = true;

	// デバッグカメラ用の初期位置
	Vector3 cameraTranslate{0.0f, 4.0f, -10.0f};
	Vector3 cameraRotate{0.45f, 0.0f, 0.0f};
	float cameraSpeed = 0.08f;
	float mouseSensitivity = 0.005f;

	// ==================================================
	// 平面とボールの初期化処理 (資料記載の初期値)
	// ==================================================
	Plane plane;
	plane.normal = MyMathUtility::Normalize({-0.2f, 0.9f, -0.3f});
	plane.distance = 0.0f;

	const Vector3 kInitialBallPosition = {0.8f, 1.2f, 0.3f};
	Ball ball{};
	ball.position = kInitialBallPosition;
	ball.velocity = {0.0f, 0.0f, 0.0f};
	ball.acceleration = {0.0f, -9.8f, 0.0f};
	ball.mass = 2.0f;
	ball.radius = 0.05f;
	ball.color = WHITE;

	// 反発係数
	float e = 0.8f;
	bool isRunning = false;
	float deltaTime = 1.0f / 60.0f;

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		// マウス位置の更新
		prevMouseX = currentMouseX;
		prevMouseY = currentMouseY;
		Novice::GetMousePosition(&currentMouseX, &currentMouseY);

		// ==================================================
		// FPSスタイル・デバッグカメラ操作
		// ==================================================
		if (Novice::IsPressMouse(1)) {
			if (isFirstClick) {
				isFirstClick = false;
			} else {
				float deltaX = float(currentMouseX - prevMouseX);
				float deltaY = float(currentMouseY - prevMouseY);

				cameraRotate.y += deltaX * mouseSensitivity;
				cameraRotate.x += deltaY * mouseSensitivity;

				cameraRotate.x = std::clamp(cameraRotate.x, -float(M_PI) / 2.1f, float(M_PI) / 2.1f);
			}
		} else {
			isFirstClick = true;
		}

		Matrix4x4 rotationMatrix = MyMathUtility::Multiply(MyMathUtility::MakeRotateXMatrix(cameraRotate.x), MyMathUtility::MakeRotateYMatrix(cameraRotate.y));

		Vector3 moveDir = {0.0f, 0.0f, 0.0f};
		if (keys[DIK_W])
			moveDir.z += 1.0f;
		if (keys[DIK_S])
			moveDir.z -= 1.0f;
		if (keys[DIK_D])
			moveDir.x += 1.0f;
		if (keys[DIK_A])
			moveDir.x -= 1.0f;

		if (moveDir.x != 0.0f || moveDir.z != 0.0f) {
			Vector3 transformedDir = MyMathUtility::Transform(moveDir, rotationMatrix);
			cameraTranslate.x += transformedDir.x * cameraSpeed;
			cameraTranslate.y += transformedDir.y * cameraSpeed;
			cameraTranslate.z += transformedDir.z * cameraSpeed;
		}

		if (keys[DIK_SPACE])
			cameraTranslate.y += cameraSpeed;
		if (keys[DIK_LSHIFT])
			cameraTranslate.y -= cameraSpeed;

		if (keys[DIK_R]) {
			cameraTranslate = {0.0f, 4.0f, -10.0f};
			cameraRotate = {0.45f, 0.0f, 0.0f};
		}

		// ===================================
		// ImGui の処理
		// ===================================
		ImGui::Begin("Ball & Plane Simulation");

		if (ImGui::Button("Start")) {
			isRunning = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset")) {
			isRunning = false;
			ball.position = kInitialBallPosition;
			ball.velocity = {0.0f, 0.0f, 0.0f};
		}

		ImGui::SliderFloat("Coefficient of Restitution (e)", &e, 0.0f, 1.0f);
		ImGui::DragFloat3("Ball Position", &ball.position.x, 0.01f);
		ImGui::DragFloat3("Plane Normal", &plane.normal.x, 0.01f);
		plane.normal = MyMathUtility::Normalize(plane.normal); // 常時正規化

		ImGui::End();

		// Eキーで開始
		if (preKeys[DIK_E] == 0 && keys[DIK_E] != 0) {
			isRunning = true;
		}

		// ===================================
		// 円運動の物理演算 (XY平面上)
		// ===================================
		if (isRunning) {
			// 1. 速度と位置の更新
			ball.velocity = ball.velocity + ball.acceleration * deltaTime;
			ball.position = ball.position + ball.velocity * deltaTime;

			// 2. 球と平面の衝突判定
			if (IsCollision(Sphere{ball.position, ball.radius}, plane)) {
				// 反射ベクトルを求める
				Vector3 reflected = Reflect(ball.velocity, plane.normal);
				// 法線方向の速度成分を取り出す
				Vector3 projectToNormal = MyMathUtility::Project(reflected, plane.normal);
				// 接線方向（移動方向）の速度成分を取り出す
				Vector3 movingDirection = reflected - projectToNormal;
				// 法線方向のみ反発係数 e で減衰させて新しい速度を設定
				ball.velocity = projectToNormal * e + movingDirection;
			}
		}
		// ===================================
		// 描画処理
		// ===================================

		// ビュー・プロジェクション計算
		Matrix4x4 cameraMatrix = MyMathUtility::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = MyMathUtility::Inverse(cameraMatrix);

		Matrix4x4 projectionMatrix = MyMathUtility::MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = MyMathUtility::Multiply(viewMatrix, projectionMatrix);

		Matrix4x4 viewportMatrix = MyMathUtility::MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		// 描画
		DrawGrid(viewProjectionMatrix, viewportMatrix);

		DrawPlane(plane, viewProjectionMatrix, viewportMatrix, WHITE);

		Sphere ballSphere{ball.position, ball.radius};
		DrawMiniSphere(ballSphere, viewProjectionMatrix, viewportMatrix, ball.color);

		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}