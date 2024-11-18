#pragma once
#include <wrl.h>
#include <stdint.h>

#include "../externals/imgui/imgui.h"

class WindowsAPI
{
public:// メンバ関数

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	// 初期化
	void Initialize();

	//クライアントの領域サイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

	// getter
	HWND GetHwnd() const { return hwnd; }
	HINSTANCE GetHInstance() const { return wc.hInstance; }

	// 更新
	void Update();

	// メッセージの処理
	bool ProcessMessage();

	// 終了
	void Finalize();

private:
	// ウィンドウハンドル
	HWND hwnd = nullptr;
	// ウィンドクラスの設定
	WNDCLASS wc{};
};

