#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <wrl.h>
#include <cassert>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

class Input
{
public:

	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	
	void Initialize(HINSTANCE hInstance,HWND hwnd);
	void Update();
	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);
private:

	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard;
	Microsoft::WRL::ComPtr<IDirectInput8> directInput;
	BYTE key[256] = {};
	BYTE preKey[256] = {};
};
