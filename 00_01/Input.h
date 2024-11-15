#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <wrl.h>
#include <cassert>


class Input
{
public:

	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	void Initialize(HINSTANCE hInstance,HWND hwnd);
	
	void Update();
private:

	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard;

};
