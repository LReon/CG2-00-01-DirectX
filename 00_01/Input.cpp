#include "Input.h"



void Input::Initialize(WindowsAPI* windowsAPI)
{
	HRESULT result;

	this->windowsAPI_ = windowsAPI;
	// DirectInputのインスタンス生成
	//Microsoft::WRL::ComPtr<IDirectInput8> directInput = nullptr;
	result = DirectInput8Create(windowsAPI->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイス生成
	//Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard;
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);

	// 入力データ形成のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);

	// 排他制御レベルのセット
	result = keyboard->SetCooperativeLevel(windowsAPI->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

}

bool Input::PushKey(BYTE keyNumber)
{
	if (key[keyNumber]) {
		return true;
	}

	return false;
}

bool Input::TriggerKey(BYTE keyNumber)
{
	if (!preKey[keyNumber] && key[keyNumber]) {
		
		return true;
	}

	return false;
}

void Input::Update()
{
	memcpy(preKey, key, sizeof(key));
	keyboard->Acquire();
	//BYTE key[256] = {};
	keyboard->GetDeviceState(sizeof(key), key);
	

}
