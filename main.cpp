#include <Novice.h>
#define _USE_MATH_DEFINES
#include "MyMathUtility.h"
#include <assert.h>
#include <math.h>

// MyMathUtilityで使われているものと連携
using namespace KamataEngine;

static const int kColumnWidth = 60;
static const int kRowHeight = 20;

// 行列を描画する関数
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

		Vector3 rotate{0.4f, 1.43f, -0.8f};

		// 各軸の回転行列を生成
		Matrix4x4 rotateXMatrix = MyMathUtility::MakeRotateXMatrix(rotate.x);
		Matrix4x4 rotateYMatrix = MyMathUtility::MakeRotateYMatrix(rotate.y);
		Matrix4x4 rotateZMatrix = MyMathUtility::MakeRotateZMatrix(rotate.z);

		// 3つの回転行列を合成 (X * Y * Z)
		Matrix4x4 rotateXYZMatrix = MyMathUtility::Multiply(rotateXMatrix, MyMathUtility::Multiply(rotateYMatrix, rotateZMatrix));

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		MatrixScreenPrintf(0, 0, rotateXMatrix, "rotateXMatrix");
		MatrixScreenPrintf(0, kRowHeight * 5, rotateYMatrix, "rotateYMatrix");
		MatrixScreenPrintf(0, kRowHeight * 5 * 2, rotateZMatrix, "rotateZMatrix");
		MatrixScreenPrintf(0, kRowHeight * 5 * 3, rotateXYZMatrix, "rotateXYZMatrix");

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