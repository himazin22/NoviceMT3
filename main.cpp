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

	// 線分と三角形の初期化
	Segment segment = {
	    {0.44f, 0.42f, -1.99f}, // 始点 (Origin)
	    {0.0f,  0.5f,  2.0f  }  // 差分ベクトル (Diff)
	};

	Triangle triangle = {
	    {
         {-1.0f, 0.0f, 0.0f}, // v0
	        {0.0f, 1.0f, 0.0f},  // v1
	        {1.0f, 0.0f, 0.0f}   // v2
	    }
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

		// ===================================
		// ImGui の処理
		// ===================================
		ImGui::Begin("Triangle vs Segment Collision");

		ImGui::Text("--- Triangle ---");
		ImGui::DragFloat3("Triangle.v0", &triangle.Vvertices[0].x, 0.02f);
		ImGui::DragFloat3("Triangle.v1", &triangle.Vvertices[1].x, 0.02f);
		ImGui::DragFloat3("Triangle.v2", &triangle.Vvertices[2].x, 0.02f);

		ImGui::Separator();

		ImGui::Text("--- Segment ---");
		ImGui::DragFloat3("Segment Origin", &segment.origin.x, 0.02f);
		ImGui::DragFloat3("Segment Diff", &segment.diff.x, 0.02f);

		ImGui::Separator();

		// 衝突判定
		bool colliding = IsCollision(triangle, segment);
		if (colliding) {
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

		// 当たっていたら赤、当たっていなければ白
		uint32_t segmentColor = colliding ? RED : WHITE;

		// 線分と三角形の描画
		DrawSegment(segment, viewProjectionMatrix, viewportMatrix, segmentColor);
		DrawTriangle(triangle, viewProjectionMatrix, viewportMatrix, WHITE);

		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}