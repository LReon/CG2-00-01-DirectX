#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WindowsAPI.h"
#include <array>


class DirectXCommon
{
public:// メンバ関数
	// 初期化
	void Initialize(WindowsAPI* windowsAPI);
	// デバイスの初期化
	void DeviceInitialize();
	// コマンド関連の初期化
	void CommandsInitialize();
	// スワップチェーンの生成
	void CreateSwapChain();
	// 深度バッファの生成
	void CreateDepthBuffer();
	// 各種デスクリプタヒープの生成
	void CreateDescriptorHeaps();
	// レンダーターゲットビューの初期化
	void RenderTargetViewInitialize();
	// 深度ステンシルビューの初期化
	void DepthStencilViewInitialize();
	// フェンスの初期化
	void FenceInitialize();
	// ビューポート矩形の初期化
	void ViewportRectangleInitialize();
	// シザリング矩形
	void ScissoringRectangle();
	// DXCコンパイラの生成
	void CreateDXCCompiler();
	// IMGUI
	void ImGuiInitialize();


	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

private:
	// DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	// Debugコントロール
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	//使用するアダプタ用の変数。最初にnullptrを入れておく
	Microsoft::WRL::ComPtr<IDXGIAdapter4> UseAdapter = nullptr;
	// エラー
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> InfoQueue = nullptr;
	//コマンドアロケータを生成する
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	//コマンドリストを生成する
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	//コマンドキューを生成する
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	//スワップチェーンを生成する
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	// WindowsAPI
	WindowsAPI* windowsAPI = nullptr;
	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthBufferResource = nullptr;
	uint32_t descriptorSizeSRV;
	uint32_t descriptorSizeRTV;
	uint32_t descriptorSizeDSV;

	//ディスクリプターヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	//SwapChainから Resourceを引っ張ってくる
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };

	// 指定番号のCPUデスクリプタハンドルを取得する
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptoprHeap, uint32_t descripotrSize, uint32_t index);
	// 指定番号のGPUデスクリプタハンドルを取得する
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptoprHeap, uint32_t descripotrSize, uint32_t index);

	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;
	//初期値0でFenceを作る
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	//ビューポート
	D3D12_VIEWPORT viewport{};

	//シザー矩形
	D3D12_RECT scissorRect{};

	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;

};