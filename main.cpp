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

	// --- 追加：球と平面の初期化 ---
	Sphere sphere = {
	    {0.0f, 0.5f, 0.0f},
        0.5f
    };
	Plane plane = {
	    {0.0f, 1.0f, 0.0f},
        0.0f
    }; // 上向きの平面

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
		// // 1 = 右クリック
		if (Novice::IsPressMouse(1)) { 
			if (isFirstClick) {
				// クリックした瞬間は移動量を0にする
				isFirstClick = false;
			} else {
				float deltaX = float(currentMouseX - prevMouseX);
				float deltaY = float(currentMouseY - prevMouseY);

				cameraRotate.y += deltaX * mouseSensitivity;
				cameraRotate.x += deltaY * mouseSensitivity;

				// カメラが逆さまにならないように制限
				cameraRotate.x = std::clamp(cameraRotate.x, -float(M_PI) / 2.1f, float(M_PI) / 2.1f);
			}
		} else {
			isFirstClick = true;
		}

		// カメラの向き（回転）を基準にした移動ベクトルの計算
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

		// 向いている方向に合わせてカメラ位置を移動
		if (moveDir.x != 0.0f || moveDir.z != 0.0f) {
			Vector3 transformedDir = MyMathUtility::Transform(moveDir, rotationMatrix);
			cameraTranslate.x += transformedDir.x * cameraSpeed;
			cameraTranslate.y += transformedDir.y * cameraSpeed;
			cameraTranslate.z += transformedDir.z * cameraSpeed;
		}

		// 上下上昇キー（これはワールド座標基準のほうが操作しやすいので独立）
		if (keys[DIK_SPACE])
			cameraTranslate.y += cameraSpeed;
		if (keys[DIK_LSHIFT])
			cameraTranslate.y -= cameraSpeed;

		// リセットキー
		if (keys[DIK_R]) {
			cameraTranslate = {0.0f, 4.0f, -10.0f};
			cameraRotate = {0.45f, 0.0f, 0.0f};
		}

		// ===================================
		// ImGui の処理
		// ===================================
		ImGui::Begin("Sphere vs Plane Collision");

		ImGui::Text("--- Sphere ---");
		ImGui::DragFloat3("Sphere Center", &sphere.center.x, 0.02f);
		ImGui::DragFloat("Sphere Radius", &sphere.radius, 0.01f, 0.01f, 5.0f);

		ImGui::Separator();

		ImGui::Text("--- Plane ---");
		// 法線（Normal）の数値をいじったら
		if (ImGui::DragFloat3("Plane Normal", &plane.normal.x, 0.01f, -1.0f, 1.0f)) {
			// ★必ずここで長さを1にリセットする！
			plane.normal = MyMathUtility::Normalize(plane.normal);
		}
		ImGui::DragFloat("Plane Distance", &plane.distance, 0.02f);

		ImGui::Separator();

		// 当たり判定
		bool colliding = IsCollision(sphere, plane);
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
		uint32_t sphereColor = colliding ? RED : WHITE;

		// 球と平面の描画
		DrawMiniSphere(sphere, viewProjectionMatrix, viewportMatrix, sphereColor);
		DrawPlane(plane, viewProjectionMatrix, viewportMatrix, WHITE);

		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}