#include "Input.h"



void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
	HRESULT result;

	// DirectInputのインスタンス生成
	Microsoft::WRL::ComPtr<IDirectInput8> directInput = nullptr;
	result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイス生成
	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard;
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);

	// 入力データ形成のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);

	// 排他制御レベルのセット
	result = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

}

void Input::Update()
{
}
