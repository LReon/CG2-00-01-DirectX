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
	// 更新
	void Update();

};

