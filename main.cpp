#include <Novice.h>
#define _USE_MATH_DEFINES
#include "MyMathUtility.h"
#include <assert.h>
#include <math.h>

// MyMathUtilityで使われているものと連携
using namespace KamataEngine;

static const int kColumnWidth = 60;
static const int kRowHeight = 20;



// ベクトルを描画する関数（復活させました）
void VectorScreenPrintf(int x, int y, const  Vector3& vector, const char* label) {
	Novice::ScreenPrintf(x, y, "%.02f", vector.x);
	Novice::ScreenPrintf(x + kColumnWidth, y, "%.02f", vector.y);
	Novice::ScreenPrintf(x + kColumnWidth * 2, y, "%.02f", vector.z);
	Novice::ScreenPrintf(x + kColumnWidth * 3, y, "%s", label);
}

// 行列を描画する関数を追加（4x4の要素を5行分使って描画します）
void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label) {
	Novice::ScreenPrintf(x, y, "%s", label);
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			Novice::ScreenPrintf(x + col * kColumnWidth, y + (row + 1) * kRowHeight, "%.02f", matrix.m[row][col]);
		}
	}
}

const char kWindowTitle[] = "LC1C_22_ツノダ_タケマサ_タイトル";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};


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

		Vector3 translate{4.1f, 2.6f, 0.8f};
		Vector3 scale{1.5f, 5.2f, 7.3f};
		Matrix4x4 translateMatrix = MyMathUtility::MakeTranslateMatrix(translate);
		Matrix4x4 scaleMatrix = MyMathUtility::MakeScaleMatrix(scale);

		Vector3 point{2.3f, 3.8f, 1.4f};
		Matrix4x4 transformMatrix = {1.0f, 2.0f, 3.0f, 4.0f, 3.0f, 1.0f, 1.0f, 2.0f, 1.0f, 4.0f, 2.0f, 3.0f, 2.0f, 2.0f, 1.0f, 3.0f};

		Vector3 transformed = MyMathUtility::Transform(point, transformMatrix);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

	
		VectorScreenPrintf(0, 0, transformed, "transformed");
		MatrixScreenPrintf(0, kRowHeight, translateMatrix, "translateMatrix");
		MatrixScreenPrintf(0, kRowHeight * 6, scaleMatrix, "scaleMatrix");

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