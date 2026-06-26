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

// 球と球の衝突判定関数
bool IsCollision(const Sphere& s1, const Sphere& s2) {
	float distance = MyMathUtility::Length(MyMathUtility::Subtract(s2.center, s1.center));
	if (distance <= (s1.radius + s2.radius)) {
		return true;
	}
	return false;
}

bool IsCollision(const Sphere& s1, const Plane& p1) {
	// 1. 球の中心点と、平面の法線ベクトルとの内積を計算
	float dot = MyMathUtility::Dot(s1.center, p1.normal);

	// 2. 内積から平面の距離を引いて絶対値をとり、平面からの最短距離を求める
	float distance = std::abs(dot - p1.distance);

	// 3. 最短距離が球の半径以下なら衝突している
	if (distance <= s1.radius) {
		return true;
	}
	return false;
}

// 線分と平面の衝突判定関数
bool IsCollision(const Segment& segment, const Plane& plane) {
	// 1. 法線と線の差分ベクトルの内積を求める（垂直＝平行判定のため）
	float dot = MyMathUtility::Dot(plane.normal, segment.diff);

	// 2. 平行である場合は衝突しない（分母が0になるのを防ぐ）
	if (dot == 0.0f) {
		return false;
	}

	// 3. スライドの数式から比率 t を求める
	float t = (plane.distance - MyMathUtility::Dot(segment.origin, plane.normal)) / dot;

	// 4. t の値が 0.0f ～ 1.0f の範囲にあれば、線分の範囲内で衝突している
	if (t >= 0.0f && t <= 1.0f) {
		return true;
	}

	return false;
}

// AABBとAABBの衝突判定
bool IsCollision(const AABB& aabb1, const AABB& aabb2) {
	if ((aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) && // X軸の重なり
	    (aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) && // Y軸の重なり
	    (aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z))   // Z軸の重なり
	{
		return true; // すべての軸で重なっていれば衝突
	}
	return false;
}

// 三角形と線分の衝突判定関数
bool IsCollision(const Triangle& triangle, const Segment& segment) {
	// 1. 三角形の各頂点を取り出す
	Vector3 v0 = triangle.Vvertices[0];
	Vector3 v1 = triangle.Vvertices[1];
	Vector3 v2 = triangle.Vvertices[2];

	// 各辺のベクトル
	Vector3 v01 = MyMathUtility::Subtract(v1, v0);
	Vector3 v12 = MyMathUtility::Subtract(v2, v1);
	Vector3 v20 = MyMathUtility::Subtract(v0, v2);

	// 2. 三角形がなす平面の法線と距離を計算
	Vector3 v02 = MyMathUtility::Subtract(v2, v0);
	Vector3 normal = MyMathUtility::Normalize(MyMathUtility::Cross(v01, v02));
	float distance = MyMathUtility::Dot(normal, v0);

	// 3. 線分と平面の交点(p)を求める
	float dot = MyMathUtility::Dot(normal, segment.diff);

	// 平行である場合は衝突しない（分母が0になるのを防ぐ）
	if (dot == 0.0f) {
		return false;
	}

	// 比率 t を求める
	float t = (distance - MyMathUtility::Dot(segment.origin, normal)) / dot;

	// t の値が 0.0f ～ 1.0f の範囲に外れていれば、線分の範囲内で衝突していない
	if (t < 0.0f || t > 1.0f) {
		return false;
	}

	// 衝突点 p を計算
	Vector3 p = MyMathUtility::Add(segment.origin, MyMathUtility::Multiply(t, segment.diff));

	// 4. 交点 p が三角形の内側にあるかを判定（資料の擬似コードより）
	Vector3 v0p = MyMathUtility::Subtract(p, v0);
	Vector3 v1p = MyMathUtility::Subtract(p, v1);
	Vector3 v2p = MyMathUtility::Subtract(p, v2);

	Vector3 cross01 = MyMathUtility::Cross(v01, v1p);
	Vector3 cross12 = MyMathUtility::Cross(v12, v2p);
	Vector3 cross20 = MyMathUtility::Cross(v20, v0p);

	// すべての小三角形のクロス積と法線が同じ方向を向いていたら衝突
	if (MyMathUtility::Dot(cross01, normal) >= 0.0f && MyMathUtility::Dot(cross12, normal) >= 0.0f && MyMathUtility::Dot(cross20, normal) >= 0.0f) {
		return true;
	}

	return false;
}

bool IsCollision(const AABB& aabb, const Sphere& sphere) {
	// AABB上の最も球の中心に近い点（最近接点）を求める
	Vector3 closestPoint;
	closestPoint.x = (std::clamp)(sphere.center.x, aabb.min.x, aabb.max.x);
	closestPoint.y = (std::clamp)(sphere.center.y, aabb.min.y, aabb.max.y);
	closestPoint.z = (std::clamp)(sphere.center.z, aabb.min.z, aabb.max.z);

	// 最近接点と球の中心との距離を求める
	float distance = MyMathUtility::Length(MyMathUtility::Subtract(closestPoint, sphere.center));

	// 距離が半径以下なら衝突している
	if (distance <= sphere.radius) {
		return true;
	}
	return false;
}

bool IsCollision(const AABB& aabb, const Segment& segment) {
	// 各軸における進入時間(tMin)と退出時間(tMax)の初期値を設定 (線分なので 0.0f から 1.0f の範囲)
	float tMin = 0.0f;
	float tMax = 1.0f;

	// --- X軸の判定 ---
	if (std::abs(segment.diff.x) < 0.0001f) {
		// 線分がX軸に対して平行な場合、始点がAABBの外にあれば衝突しない
		if (segment.origin.x < aabb.min.x || segment.origin.x > aabb.max.x) {
			return false;
		}
	} else {
		// 各面への到達時間を計算
		float t1 = (aabb.min.x - segment.origin.x) / segment.diff.x;
		float t2 = (aabb.max.x - segment.origin.x) / segment.diff.x;
		// 進入面と退出面を正しくソート
		float tNear = (std::min)(t1, t2);
		float tFar = (std::max)(t1, t2);
		// 全体の時間範囲を狭めていく
		tMin = (std::max)(tMin, tNear);
		tMax = (std::min)(tMax, tFar);
		if (tMin > tMax)
			return false; // 矛盾が生じたら衝突していない
	}

	// --- Y軸の判定 ---
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

	// --- Z軸の判定 ---
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

	// 3軸すべてで重なる時間領域（tMin <= tMax）が存在すれば衝突している
	return true;
}

bool IsCollision(const OBB& obb, const Sphere& sphere) {
	// 1. OBBの中心から球の中心へのベクトルを計算
	Vector3 d = MyMathUtility::Subtract(sphere.center, obb.center);

	// 最近接点を求めるためのベース（最初はOBBの中心）
	Vector3 closestPoint = obb.center;

	// 2. OBBの各軸（X, Y, Z）について、球の中心がOBBの範囲外にあればクランプして手繰り寄せる
	// orientations[0] = X軸, orientations[1] = Y軸, orientations[2] = Z軸
	float sizes[3] = {obb.size.x, obb.size.y, obb.size.z};

	for (int i = 0; i < 3; ++i) {
		// OBBの各軸に対する距離を内積で射影
		float dist = MyMathUtility::Dot(d, obb.orientations[i]);

		// OBBのサイズ（半幅）でクランプ
		dist = std::clamp(dist, -sizes[i], sizes[i]);

		// OBBの中心に、各軸方向のクランプされた距離を足していく
		closestPoint.x += obb.orientations[i].x * dist;
		closestPoint.y += obb.orientations[i].y * dist;
		closestPoint.z += obb.orientations[i].z * dist;
	}

	// 3. 最近接点と球の中心の距離の2乗を計算
	Vector3 diff = MyMathUtility::Subtract(closestPoint, sphere.center);
	float distanceSquared = MyMathUtility::LengthSquared(diff);

	// 4. 距離の2乗が半径の2乗以下なら衝突
	return distanceSquared <= (sphere.radius * sphere.radius);
}

// 線分とOBBの衝突判定関数
bool IsCollision(const Segment& segment, const OBB& obb) {
	// 1. OBBのローカル空間へ変換するための、線分の始点をOBB中心からの相対座標にする
	Vector3 localOrigin = MyMathUtility::Subtract(segment.origin, obb.center);

	// 2. 線分の始点と方向ベクトルを、OBBの各軸に射影（回転の逆変換）する
	// OBBの orientations は直交しているので、内積（Dot）をとるだけでローカル座標に変換できます。
	Segment localSegment;
	localSegment.origin.x = MyMathUtility::Dot(localOrigin, obb.orientations[0]);
	localSegment.origin.y = MyMathUtility::Dot(localOrigin, obb.orientations[1]);
	localSegment.origin.z = MyMathUtility::Dot(localOrigin, obb.orientations[2]);

	localSegment.diff.x = MyMathUtility::Dot(segment.diff, obb.orientations[0]);
	localSegment.diff.y = MyMathUtility::Dot(segment.diff, obb.orientations[1]);
	localSegment.diff.z = MyMathUtility::Dot(segment.diff, obb.orientations[2]);

	// 3. OBBのサイズから、ローカル空間上でのAABB（min, max）を作成する
	AABB localAABB;
	localAABB.min = {-obb.size.x, -obb.size.y, -obb.size.z};
	localAABB.max = {obb.size.x, obb.size.y, obb.size.z};

	// 4. すでに作成済みの「AABB と Segment の衝突判定」に丸投げする
	return IsCollision(localAABB, localSegment);
}

void DrawOBB(const OBB& obb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	// OBBの8頂点を定義（ローカルオフセットの組み合わせ）
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
		// ローカル座標からワールド座標に変換 (中心 + 各軸 × オフセット)
		worldVertices[i] = obb.center;
		worldVertices[i].x += obb.orientations[0].x * localVertices[i].x + obb.orientations[1].x * localVertices[i].y + obb.orientations[2].x * localVertices[i].z;
		worldVertices[i].y += obb.orientations[0].y * localVertices[i].x + obb.orientations[1].y * localVertices[i].y + obb.orientations[2].y * localVertices[i].z;
		worldVertices[i].z += obb.orientations[0].z * localVertices[i].x + obb.orientations[1].z * localVertices[i].y + obb.orientations[2].z * localVertices[i].z;

		// ビュー・プロジェクション変換
		Vector3 ndcVertex = MyMathUtility::Transform(worldVertices[i], viewProjectionMatrix);
		// スクリーン（ビューポート）変換
		screenVertices[i] = MyMathUtility::Transform(ndcVertex, viewportMatrix);
	}

	// 12本の辺を描画
	// 手前の面
	Novice::DrawLine((int)screenVertices[0].x, (int)screenVertices[0].y, (int)screenVertices[1].x, (int)screenVertices[1].y, color);
	Novice::DrawLine((int)screenVertices[1].x, (int)screenVertices[1].y, (int)screenVertices[3].x, (int)screenVertices[3].y, color);
	Novice::DrawLine((int)screenVertices[3].x, (int)screenVertices[3].y, (int)screenVertices[2].x, (int)screenVertices[2].y, color);
	Novice::DrawLine((int)screenVertices[2].x, (int)screenVertices[2].y, (int)screenVertices[0].x, (int)screenVertices[0].y, color);

	// 奥の面
	Novice::DrawLine((int)screenVertices[4].x, (int)screenVertices[4].y, (int)screenVertices[5].x, (int)screenVertices[5].y, color);
	Novice::DrawLine((int)screenVertices[5].x, (int)screenVertices[5].y, (int)screenVertices[7].x, (int)screenVertices[7].y, color);
	Novice::DrawLine((int)screenVertices[7].x, (int)screenVertices[7].y, (int)screenVertices[6].x, (int)screenVertices[6].y, color);
	Novice::DrawLine((int)screenVertices[6].x, (int)screenVertices[6].y, (int)screenVertices[4].x, (int)screenVertices[4].y, color);

	// 手前と奥を繋ぐ4本
	Novice::DrawLine((int)screenVertices[0].x, (int)screenVertices[0].y, (int)screenVertices[4].x, (int)screenVertices[4].y, color);
	Novice::DrawLine((int)screenVertices[1].x, (int)screenVertices[1].y, (int)screenVertices[5].x, (int)screenVertices[5].y, color);
	Novice::DrawLine((int)screenVertices[2].x, (int)screenVertices[2].y, (int)screenVertices[6].x, (int)screenVertices[6].y, color);
	Novice::DrawLine((int)screenVertices[3].x, (int)screenVertices[3].y, (int)screenVertices[7].x, (int)screenVertices[7].y, color);
}

void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	// 8つの頂点を定義
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

	// スクリーン座標に変換された頂点を格納する配列
	Vector3 screenVertices[8];
	for (int i = 0; i < 8; ++i) {
		Matrix4x4 wvpVpMatrix = MyMathUtility::Multiply(viewProjectionMatrix, viewportMatrix);
		screenVertices[i] = MyMathUtility::Transform(vertices[i], wvpVpMatrix);
	}

	// 12本の辺（インデックスペア）を描画
	int indices[12][2] = {
	    {0, 1},
        {1, 3},
        {3, 2},
        {2, 0}, // 手前の面
	    {4, 5},
        {5, 7},
        {7, 6},
        {6, 4}, // 奥の面
	    {0, 4},
        {1, 5},
        {2, 6},
        {3, 7}  // 手前と奥を繋ぐ辺
	};

	for (int i = 0; i < 12; ++i) {
		Novice::DrawLine(
		    static_cast<int>(screenVertices[indices[i][0]].x), static_cast<int>(screenVertices[indices[i][0]].y), static_cast<int>(screenVertices[indices[i][1]].x),
		    static_cast<int>(screenVertices[indices[i][1]].y), color);
	}
}

// 三角形の描画関数
void DrawTriangle(const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 screenVertices[3];
	for (int i = 0; i < 3; ++i) {
		// 3次元座標をスクリーン座標系まで変換
		Vector3 ndc = MyMathUtility::Transform(triangle.Vvertices[i], viewProjectionMatrix);
		screenVertices[i] = MyMathUtility::Transform(ndc, viewportMatrix);
	}

	// Novice::DrawTriangle を利用してワイヤーフレームで描画
	Novice::DrawTriangle(
	    int(screenVertices[0].x), int(screenVertices[0].y), int(screenVertices[1].x), int(screenVertices[1].y), int(screenVertices[2].x), int(screenVertices[2].y), color, kFillModeWireFrame);
}

// 線分の描画関数
void DrawSegment(const Segment& segment, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	// 始点
	Vector3 start = segment.origin;
	// 終点 = 始点 + 差分ベクトル
	Vector3 end = MyMathUtility::Add(segment.origin, segment.diff);

	// スクリーンの座標に変換
	Vector3 screenStart = MyMathUtility::Transform(MyMathUtility::Transform(start, viewProjectionMatrix), viewportMatrix);
	Vector3 screenEnd = MyMathUtility::Transform(MyMathUtility::Transform(end, viewProjectionMatrix), viewportMatrix);

	// 線を描画
	Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
}

Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return {-vector.y, vector.x, 0.0f};
	}
	return {0.0f, -vector.z, vector.y};
}

void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	// 1. 中心点を決める
	Vector3 center = MyMathUtility::Multiply(plane.distance, plane.normal);

	// 2〜5. 中心から伸びる4つの直交するベクトルを求める
	Vector3 perpendiculars[4];
	perpendiculars[0] = MyMathUtility::Normalize(Perpendicular(plane.normal));              // 2
	perpendiculars[1] = {-perpendiculars[0].x, -perpendiculars[0].y, -perpendiculars[0].z}; // 3
	perpendiculars[2] = MyMathUtility::Cross(plane.normal, perpendiculars[0]);              // 4
	perpendiculars[3] = {-perpendiculars[2].x, -perpendiculars[2].y, -perpendiculars[2].z}; // 5

	// 6. ベクトルを定数倍(今回は2.0f)して中心に足し、スクリーンの座標に変換する
	Vector3 points[4];
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = MyMathUtility::Multiply(2.0f, perpendiculars[index]); // 2.0fの大きさの平面になる
		Vector3 point = MyMathUtility::Add(center, extend);
		points[index] = MyMathUtility::Transform(MyMathUtility::Transform(point, viewProjectionMatrix), viewportMatrix);
	}

	// pointsをそれぞれ結んで DrawLine で矩形（ひし形）を描画する
	// 0(右) -> 2(上) -> 1(左) -> 3(下) -> 0(右) の順に線を引く
	Novice::DrawLine(int(points[0].x), int(points[0].y), int(points[2].x), int(points[2].y), color);
	Novice::DrawLine(int(points[2].x), int(points[2].y), int(points[1].x), int(points[1].y), color);
	Novice::DrawLine(int(points[1].x), int(points[1].y), int(points[3].x), int(points[3].y), color);
	Novice::DrawLine(int(points[3].x), int(points[3].y), int(points[0].x), int(points[0].y), color);
}

// 軽量な球体描画関数
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

// 床グリッド描画関数
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
	float mouseSensitivity = 0.005f; // マウスの感度調整

	// OBB回転用角度(ラジアン)をImGuiで操作するための変数
	Vector3 obbRotate = {0.0f, 0.0f, 0.0f};

	OBB obb{
	    .center{-1.0f,              0.0f,               0.0f              },

	    .orientations{{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},

	    .size{0.5f,               0.5f,               0.5f              }
    };

	Segment segment{
	    .origin{-2.0f, 1.0f, 0.0f}, // 始点
	    .diff{4.0f,  0.0f, 0.0f}  // 終点へ向かうベクトル（右に突き抜ける線）
	};

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		// マウス位置の更新
		prevMouseX = currentMouseX;
		prevMouseY = currentMouseY;
		Novice::GetMousePosition(&currentMouseX, &currentMouseY);

		// ==================================================
		// 🛠️ FPSスタイル・デバッグカメラ操作 (WASD + マウス)
		// ==================================================

		// 1. マウスの右クリックドラッグによる視点変更
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

		// ImGuiの回転角から回転行列をつくり、各軸ベクトルを計算
		Matrix4x4 obbRotM = MyMathUtility::Multiply(
		    MyMathUtility::Multiply(MyMathUtility::MakeRotateXMatrix(obbRotate.x), MyMathUtility::MakeRotateYMatrix(obbRotate.y)), MyMathUtility::MakeRotateZMatrix(obbRotate.z));

		// 回転行列の各列(または行)から、回転後のXYZ軸ベクトルを抽出
		obb.orientations[0] = {obbRotM.m[0][0], obbRotM.m[0][1], obbRotM.m[0][2]};
		obb.orientations[1] = {obbRotM.m[1][0], obbRotM.m[1][1], obbRotM.m[1][2]};
		obb.orientations[2] = {obbRotM.m[2][0], obbRotM.m[2][1], obbRotM.m[2][2]};

		// 2. 衝突判定の呼び出し
		bool isColliding = IsCollision(segment, obb);

		// ===================================
		// ImGui の処理
		// ===================================

		// 3. ImGuiの描画（確認用にSegmentのデバッグを追加すると便利です）
		ImGui::Begin("OBB to Segment Window");
		ImGui::Text("--- OBB ---");
		ImGui::DragFloat3("OBB Center", &obb.center.x, 0.02f);
		ImGui::SliderFloat3("OBB Rotate", &obbRotate.x, -float(M_PI), float(M_PI));

		ImGui::Text("--- Segment ---");
		ImGui::DragFloat3("Segment Origin", &segment.origin.x, 0.02f);
		ImGui::DragFloat3("Segment Diff", &segment.diff.x, 0.02f);

		if (isColliding) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "STATUS: HIT!!");
		} else {
			ImGui::Text("STATUS: Safe");
		}
		ImGui::End();

		// ビュー・プロジェクション計算
		Matrix4x4 cameraMatrix = MyMathUtility::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = MyMathUtility::Inverse(cameraMatrix);

		Matrix4x4 projectionMatrix = MyMathUtility::MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = MyMathUtility::Multiply(viewMatrix, projectionMatrix);

		Matrix4x4 viewportMatrix = MyMathUtility::MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		// 描画
		DrawGrid(viewProjectionMatrix, viewportMatrix);
		// 衝突状態に合わせて色を変更
		uint32_t color = isColliding ? RED : WHITE;

		//図形 
		DrawOBB(obb, viewProjectionMatrix, viewportMatrix, color);
		DrawSegment(segment, viewProjectionMatrix, viewportMatrix, color);

		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}