#include <Novice.h>
#define _USE_MATH_DEFINES
#include "MyMathUtility.h"
#include <assert.h>
#include <algorithm>
#include <imgui.h>
#include <math.h>

using namespace KamataEngine;

const int kWindowWidth = 1280;
const int kWindowHeight = 720;

struct Sphere {
	Vector3 center;
	float radius;
};

struct Segment {
	Vector3 origin;
	Vector3 diff;
};

// --- 正射影ベクトルを求める関数 ---
Vector3 Project(const Vector3& v1, const Vector3& v2) {
	float lengthSq = MyMathUtility::LengthSquared(v2);
	if (lengthSq == 0.0f) {
		return {0.0f, 0.0f, 0.0f};
	}
	float t = MyMathUtility::Dot(v1, v2) / lengthSq;
	return {v2.x * t, v2.y * t, v2.z * t};
}

// --- 点から線分への最近接点を求める関数 ---
Vector3 ClosestPoint(const Vector3& point, const Segment& segment) {
	Vector3 v1 = MyMathUtility::Subtract(point, segment.origin);
	float lengthSq = MyMathUtility::LengthSquared(segment.diff);
	float t = 0.0f;
	if (lengthSq != 0.0f) {
		t = MyMathUtility::Dot(v1, segment.diff) / lengthSq;
	}
	// 線分のため 0.0f ～ 1.0f の範囲に制限（クランプ）する
	t = std::clamp(t, 0.0f, 1.0f);

	Vector3 result = {segment.origin.x + segment.diff.x * t, segment.origin.y + segment.diff.y * t, segment.origin.z + segment.diff.z * t};
	return result;
}

// --- 大量のライン消費によるアサートエラーを防ぐための軽量な点描画関数 ---
void DrawCustomPoint(const Vector3& point, float size, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	// 点を中心にひし形（8面体）のワイヤーフレームを定義
	Vector3 vertices[6] = {
	    {point.x + size, point.y,        point.z       },
        {point.x - size, point.y,        point.z       },
        {point.x,        point.y + size, point.z       },
	    {point.x,        point.y - size, point.z       },
        {point.x,        point.y,        point.z + size},
        {point.x,        point.y,        point.z - size}
    };

	Vector3 screenPoints[6];
	for (int i = 0; i < 6; ++i) {
		screenPoints[i] = MyMathUtility::Transform(MyMathUtility::Transform(vertices[i], viewProjectionMatrix), viewportMatrix);
	}

	// 12本のラインで点の立体的な目印を綺麗に描画
	int indices[12][2] = {
	    {0, 2},
        {2, 1},
        {1, 3},
        {3, 0},
        {0, 4},
        {1, 4},
        {2, 4},
        {3, 4},
        {0, 5},
        {1, 5},
        {2, 5},
        {3, 5}
    };

	for (int i = 0; i < 12; ++i) {
		Novice::DrawLine(int(screenPoints[indices[i][0]].x), int(screenPoints[indices[i][0]].y), int(screenPoints[indices[i][1]].x), int(screenPoints[indices[i][1]].y), color);
	}
}

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {

	const float kGridHalfWidth = 2.0f;
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

		// グリッド線の色（実装イメージに合わせて薄いグレー）
		uint32_t color = 0xAAAAAAFF;

		Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
	}

	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		float z = -kGridHalfWidth + zIndex * kGridEvery;
		Vector3 start = {-kGridHalfWidth, 0.0f, z};
		Vector3 end = {kGridHalfWidth, 0.0f, z};

		// 3D空間の座標をスクリーン空間（2D）に変換
		Vector3 ndcStart = MyMathUtility::Transform(start, viewProjectionMatrix);
		Vector3 screenStart = MyMathUtility::Transform(ndcStart, viewportMatrix);

		Vector3 ndcEnd = MyMathUtility::Transform(end, viewProjectionMatrix);
		Vector3 screenEnd = MyMathUtility::Transform(ndcEnd, viewportMatrix);

		uint32_t color = 0xAAAAAAFF;

		// ラインの描画
		Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
	}
}

void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectMatrix, const Matrix4x4& viewPortMatrix, uint32_t color) {
	const uint32_t kSubdivision = 32;
	const float kLonEvery = 2.0f * float(M_PI) / float(kSubdivision);
	const float kLatEvery = float(M_PI) / float(kSubdivision);

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -float(M_PI) / 2.0f + kLatEvery * latIndex;
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {

			float lon = lonIndex * kLonEvery;
			// 現在の基準点Aの3D座標計算
			Vector3 a = {sphere.radius * cosf(lat) * cosf(lon) + sphere.center.x, sphere.radius * sinf(lat) + sphere.center.y, sphere.radius * cosf(lat) * sinf(lon) + sphere.center.z};

			// 次の緯度線上の点Bの3D座標計算
			float nextLat = lat + kLatEvery;
			Vector3 b = {sphere.radius * cosf(nextLat) * cosf(lon) + sphere.center.x, sphere.radius * sinf(nextLat) + sphere.center.y, sphere.radius * cosf(nextLat) * sinf(lon) + sphere.center.z};

			// 次の経度線上の点Cの3D座標計算
			float nextLon = lon + kLonEvery;
			Vector3 c = {sphere.radius * cosf(lat) * cosf(nextLon) + sphere.center.x, sphere.radius * sinf(lat) + sphere.center.y, sphere.radius * cosf(lat) * sinf(nextLon) + sphere.center.z};

			// 3D座標をスクリーン空間の2D座標へ変換
			Vector3 screenA = MyMathUtility::Transform(MyMathUtility::Transform(a, viewProjectMatrix), viewPortMatrix);
			Vector3 screenB = MyMathUtility::Transform(MyMathUtility::Transform(b, viewProjectMatrix), viewPortMatrix);
			Vector3 screenC = MyMathUtility::Transform(MyMathUtility::Transform(c, viewProjectMatrix), viewPortMatrix);

			// 網目を構成する2方向の線を描画
			// 縦の線（緯度方向）
			Novice::DrawLine(int(screenA.x), int(screenA.y), int(screenB.x), int(screenB.y), color);
			// 横の線（経度方向）
			Novice::DrawLine(int(screenA.x), int(screenA.y), int(screenC.x), int(screenC.y), color);
		}
	}
}


const char kWindowTitle[] = "LC1C_22_ツノダ_タケマサ_タイトル";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 cameraTranslate{0.0f, 2.5f, -6.0f};
	Vector3 cameraRotate{0.35f, 0.0f, 0.0f};

	Segment segment{
	    {-2.0f, -1.0f, 0.0f},
        {3.0f,  2.0f,  2.0f}
    };
	Vector3 point{-1.5f, 0.6f, 0.6f};
	
	//Sphere sphere;
	//sphere.center = {0.0f, 0.0f, 0.0f};
	//sphere.radius = 0.73f;
	
	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///
		
		// 毎フレーム計算を行う
		Vector3 project = Project(MyMathUtility::Subtract(point, segment.origin), segment.diff);
		Vector3 closestPoint = ClosestPoint(point, segment);

		// ImGuiウィンドウの表示とパラメータ更新処理
		ImGui::Begin("Window");
		ImGui::DragFloat3("Point", &point.x, 0.01f);
		ImGui::DragFloat3("Segment origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("Segment diff", &segment.diff.x, 0.01f);
		ImGui::InputFloat3("Project", &project.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
		ImGui::End();
		
		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		Matrix4x4 cameraMatrix = MyMathUtility::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = MyMathUtility::Inverse(cameraMatrix);

		Matrix4x4 projectionMatrix = MyMathUtility::MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = MyMathUtility::Multiply(viewMatrix, projectionMatrix);

		Matrix4x4 viewportMatrix = MyMathUtility::MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		// 1. 床グリッド描画
		DrawGrid(viewProjectionMatrix, viewportMatrix);

		// 2. 線分の描画（白色）
		Vector3 start = MyMathUtility::Transform(MyMathUtility::Transform(segment.origin, viewProjectionMatrix), viewportMatrix);
		Vector3 end = MyMathUtility::Transform(MyMathUtility::Transform(MyMathUtility::Add(segment.origin, segment.diff), viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), WHITE);

		// 3. 元の点を「赤(RED)」、最近接点を「黒(BLACK)」で安全に目印として描画
		DrawCustomPoint(point, 0.04f, viewProjectionMatrix, viewportMatrix, RED);
		DrawCustomPoint(closestPoint, 0.04f, viewProjectionMatrix, viewportMatrix, BLACK);

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}