#include <Novice.h>
#define _USE_MATH_DEFINES
#include "MyMathUtility.h"
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

// --- クロス積の実装 ---
//Vector3 Cross(const Vector3& v1, const Vector3& v2) {
//	Vector3 result;
//	result.x = v1.y * v2.z - v1.z * v2.y;
//	result.y = v1.z * v2.x - v1.x * v2.z;
//	result.z = v1.x * v2.y - v1.y * v2.x;
//	return result;
//}

// --- ベクトルを描画する関数 ---
//void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label) { Novice::ScreenPrintf(x, y, "%.02f %.02f %.02f %s", vector.x, vector.y, vector.z, label); }

const char kWindowTitle[] = "LC1C_22_ツノダ_タケマサ_タイトル";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 cameraTranslate{0.0f, 1.9f, -6.49f};
	Vector3 cameraRotate{0.26f, 0.0f, -0.0f};

	// 今回描画する球体の初期化
	Sphere sphere;
	sphere.center = {0.0f, 0.0f, 0.0f};
	sphere.radius = 0.73f;
	
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
		
		// ImGuiウィンドウの表示とパラメータ更新処理
		ImGui::Begin("Window");
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);
		ImGui::DragFloat3("SphereCenter", &sphere.center.x, 0.01f);
		ImGui::DragFloat("SphereRadius", &sphere.radius, 0.01f);
		ImGui::End();
		
		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		// --- 3D描画処理 ---
		// カメラ行列・ビュー行列の作成（ImGuiで調整した値を使用）
		Matrix4x4 cameraMatrix = MyMathUtility::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = MyMathUtility::Inverse(cameraMatrix);

		// 透視投影行列とビュープロジェクション行列の作成
		Matrix4x4 projectionMatrix = MyMathUtility::MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = MyMathUtility::Multiply(viewMatrix, projectionMatrix);

		// スクリーン空間へマッピングするためのビューポート行列
		Matrix4x4 viewportMatrix = MyMathUtility::MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		// 床のグリッドを描画
		DrawGrid(viewProjectionMatrix, viewportMatrix);

		// 球体を描画（実装イメージに合わせて黒色のワイヤーフレーム）
		DrawSphere(sphere, viewProjectionMatrix, viewportMatrix, BLACK);

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