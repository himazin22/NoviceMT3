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

// --- 点を表現するために分割数を大幅に抑えた軽量な球体描画関数 ---
void DrawMiniSphere(const Sphere& sphere, const Matrix4x4& viewProjectMatrix, const Matrix4x4& viewPortMatrix, uint32_t color) {
	const uint32_t kSubdivision = 8;
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

	Vector3 cameraTranslate{0.0f, 2.5f, -6.0f};
	Vector3 cameraRotate{0.35f, 0.0f, 0.0f};

	// MyMathUtility.h の Segment 構造体を使用
	Segment segment{
	    {-2.0f, -1.0f, 0.0f},
        {3.0f,  2.0f,  2.0f}
    };
	Vector3 point{-1.5f, 0.6f, 0.6f};

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		// クラス内の静的関数から計算を呼び出し
		Vector3 project = MyMathUtility::Project(MyMathUtility::Subtract(point, segment.origin), segment.diff);
		Vector3 closestPoint = MyMathUtility::ClosestPoint(point, segment);

		ImGui::Begin("Window");
		ImGui::DragFloat3("Point", &point.x, 0.01f);
		ImGui::DragFloat3("Segment origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("Segment diff", &segment.diff.x, 0.01f);
		ImGui::InputFloat3("Project", &project.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
		ImGui::End();

		Matrix4x4 cameraMatrix = MyMathUtility::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = MyMathUtility::Inverse(cameraMatrix);

		Matrix4x4 projectionMatrix = MyMathUtility::MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = MyMathUtility::Multiply(viewMatrix, projectionMatrix);

		Matrix4x4 viewportMatrix = MyMathUtility::MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		// 床グリッド描画
		DrawGrid(viewProjectionMatrix, viewportMatrix);

		// 線分の描画（白色）
		Vector3 start = MyMathUtility::Transform(MyMathUtility::Transform(segment.origin, viewProjectionMatrix), viewportMatrix);
		Vector3 end = MyMathUtility::Transform(MyMathUtility::Transform(MyMathUtility::Add(segment.origin, segment.diff), viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), WHITE);

		// 元の点を 赤(RED) で描画
		Sphere pointSphere{point, 0.08f};
		DrawMiniSphere(pointSphere, viewProjectionMatrix, viewportMatrix, RED);

		// 最近接点を 黒(BLACK) で描画
		Sphere closestPointSphere{closestPoint, 0.08f};
		DrawMiniSphere(closestPointSphere, viewProjectionMatrix, viewportMatrix, BLACK);

		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}