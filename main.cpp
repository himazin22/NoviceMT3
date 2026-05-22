#include <Novice.h>
#define _USE_MATH_DEFINES
#include "MyMathUtility.h"
#include <assert.h>
#include <math.h>

using namespace KamataEngine;

const int kWindowWidth = 1280;
const int kWindowHeight = 720;


// --- クロス積の実装 ---
Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;
	return result;
}

// --- ベクトルを描画する関数 ---
void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label) { Novice::ScreenPrintf(x, y, "%.02f %.02f %.02f %s", vector.x, vector.y, vector.z, label); }

const char kWindowTitle[] = "LC1C_22_ツノダ_タケマサ_タイトル";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	char keys[256] = {0};
	char preKeys[256] = {0};
	// --- 1. クロス積の確認用 ---
	Vector3 v1{1.2f, -3.9f, 2.5f};
	Vector3 v2{2.8f, 0.4f, -1.3f};
	// ループ内で計算して表示
	Vector3 crossResult;

	// --- 2. 3D描画用 ---
	Vector3 rotate = {0.0f, 0.0f, 0.0f};
	Vector3 translate = {0.0f, 0.0f, 0.0f};
	// カメラの位置
	Vector3 cameraPosition = {0.0f, 0.0f, -10.0f};
	// 三角形のローカル頂点
	Vector3 kLocalVertices[3] = {
	    {0.0f,  1.0f,  0.0f},
        {-1.0f, -1.0f, 0.0f},
        {1.0f,  -1.0f, 0.0f}
    };
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
		// 移動
		if (keys[DIK_A]) {
			translate.x -= 0.1f;
		}
		if (keys[DIK_D]) {
			translate.x += 0.1f;
		}
		if (keys[DIK_W]) {
			translate.z -= 0.1f;
		}
		if (keys[DIK_S]) {
			translate.z += 0.1f;
		}

		// b. 自動でY軸回転 (1フレームあたりの回転角)
		rotate.y += 0.03f;
		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		// 背景を青で塗りつぶす (完成イメージに合わせる)
		Novice::DrawBox(0, 0, 1280, 720, 0.0f, 0x6699CCFF, kFillModeSolid);

		// クロス積を計算して表示
		crossResult = Cross(v1, v2);
		VectorScreenPrintf(0, 0, crossResult, "Cross");

		// --- 3D描画処理 ---
		// 各種行列の計算
		Matrix4x4 worldMatrix = MyMathUtility::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, rotate, translate);
		Matrix4x4 cameraMatrix = MyMathUtility::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, cameraPosition);
		Matrix4x4 viewMatrix = MyMathUtility::Inverse(cameraMatrix);
		// ProjectionMatrixのFOVを完成イメージに合わせる
		Matrix4x4 projectionMatrix = MyMathUtility::MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = MyMathUtility::Multiply(worldMatrix, MyMathUtility::Multiply(viewMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = MyMathUtility::MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		// Screen空間へと頂点を変換する
		Vector3 screenVertices[3];
		for (uint32_t i = 0; i < 3; ++i) {
			// NDCまで変換。Transformを使うと同次座標系->デカルト座標系の処理が行われ、結果的にZDivideが行われることになる
			Vector3 ndcVertex = MyMathUtility::Transform(kLocalVertices[i], worldViewProjectionMatrix);
			// Viewport変換を行ってScreen空間へ
			screenVertices[i] = MyMathUtility::Transform(ndcVertex, viewportMatrix);
		}

		// 描画 (Novice::DrawTriangleで赤い三角形)
		Novice::DrawTriangle(
		    int(screenVertices[0].x), int(screenVertices[0].y),
		    int(screenVertices[1].x), int(screenVertices[1].y),
		    int(screenVertices[2].x), int(screenVertices[2].y),
		    RED, kFillModeSolid
		);

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