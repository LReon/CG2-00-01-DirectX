#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <wrl.h>
#include <cassert>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

class Input
{

public:

	void Initialize(HINSTANCE hInstance,HWND hwnd);
	
	void Update();

};
