#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WindowsAPI.h"
#include <array>
#include "../externals/DirectXTex/DirectXTex.h"
#include <dxcapi.h>
#include <string>
#include <Windows.h>
#include <chrono>
#include <thread>




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

	// 描画前処理
	void PreDraw();
	// 描画後処理
	void PostDraw();

	// getter
	ID3D12Device* GetDevice()const { return device.Get(); }
	ID3D12GraphicsCommandList* GetCommandList()const { return commandList.Get(); }

	// シェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeBytes);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);

	void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

	static DirectX::ScratchImage LoadTexture(const std::string& filePath);

	// 指定番号のCPUデスクリプタハンドルを取得する
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptoprHeap, uint32_t descripotrSize, uint32_t index);
	// 指定番号のGPUデスクリプタハンドルを取得する
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptoprHeap, uint32_t descripotrSize, uint32_t index);

	//ディスクリプターヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;

	HANDLE fenceEvent;

	uint32_t descriptorSizeSRV;
	uint32_t descriptorSizeRTV;
	uint32_t descriptorSizeDSV;

private:

	HRESULT hr;
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
	

	

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	
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
	IDxcIncludeHandler* includeHandler = nullptr;
	D3D12_RESOURCE_BARRIER barrier{};

	UINT backBufferIndex;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	uint64_t fenceValue = 0;


	

	//hlslファイルを読む
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	IDxcBlobUtf8* shaderError = nullptr;
	IDxcBlob* shaderBlod = nullptr;
	//実際に頂点リソースを作る
	ID3D12Resource* VertexResource = nullptr;
	//Resourceの生成
	ID3D12Resource* createTextureResource = nullptr;


	void InitializeFixFPS();

	void UpdateFixFPS();

	std::chrono::steady_clock::time_point reference_;

};