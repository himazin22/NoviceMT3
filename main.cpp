#include <Novice.h>
#define _USE_MATH_DEFINES
#include <assert.h>
#include <math.h>

typedef struct Vector3 {
	float x;
	float y;
	float z;
} Vector3;

struct Matrix3x3 {
	float m[3][3];
};

static const int kColumnWidth = 60;

Vector3 Add(const Vector3& v1, const Vector3& v2) {}
Vector3 Subtract(const Vector3& v1, const Vector3& v2) {}
Vector3 Multiply(float scalar, const Vector3& v) {}

float Dot(const Vector3& v1, const Vector3& v2) {}

float Lengh(const Vector3& v) {}

Vector3 Normalize(const Vector3& v) {}

void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label) {}

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

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

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
