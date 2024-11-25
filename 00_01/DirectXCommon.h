#pragma once
#include <wrl.h>
#include <string>
#include <format>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "Logger.h"
#include "WindowsAPI.h"
#include <dxcapi.h>

class DirectXCommon
{

public:

	void Log(const std::string& message);

	std::string ConvertString(const std::wstring& str);

	void CommandInitialize();

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;

	void MakeSwapChain();

	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;

	void MakeDepthBuffer();
	
	void KindDescriptorHeap();

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	void RenderTargetViewInitialize();

	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

	void DepthStencilView();

	void Fence();

	void Viewport();

	void Scissor();

	void DXCCompiler();

	void ImGui();

	// 初期化
	void Initialize();

	uint64_t fenceValue = 0;

	void PreDraw();

	void PostDraw();


	UINT64 fenceVal = 0;

private:
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;

	WindowsAPI* windowsAPI = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	//RTVを２つ作るディスクリプタを２つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	//ビューポート
	D3D12_VIEWPORT viewport{};


	//シザー矩形
	D3D12_RECT scissorRect{};

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap=nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap=nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap=nullptr;

	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptoprHeap, uint32_t descriptorSize, uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;
};

