
#include "DirectXCommon.h"
#include <cassert>
#include <chrono>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#include "Logger.h"
#include "StringUtility.h"
#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"



using namespace Microsoft::WRL;


void DirectXCommon::Initialize(WindowsAPI* windowsAPI)
{
	// NULL検出
	assert(windowsAPI);
	// メンバ変数に記録
	this->windowsAPI = windowsAPI;

	
	// デバイスの初期化
	DeviceInitialize();
	// コマンド関連の初期化
	CommandsInitialize();
	// スワップチェーンの生成
	CreateSwapChain();
	// 深度バッファの生成
	CreateDepthBuffer();
	// 各種デスクリプタヒープの生成
	CreateDescriptorHeaps();
	// レンダーターゲットビューの初期化
	RenderTargetViewInitialize();
	// 深度ステンシルビューの初期化
	DepthStencilViewInitialize();
	// フェンスの初期化
	FenceInitialize();
	// ビューポート矩形の初期化
	ViewportRectangleInitialize();
	// シザリングの初期化
	ScissoringRectangle();
	// DXCコンパイラの生成
	CreateDXCCompiler();
	// ImGuiの初期化
	ImGuiInitialize();

	InitializeFixFPS();
}

void DirectXCommon::DeviceInitialize()
{
	
#ifdef _DEBUG

	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		//デバッグプレイヤーを有効化する
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif // _DEBUG

	//DXGIファクトリーの生成
	//HRESULはWindows系のエラーコードあり
	//関数が成功したかどうかをSUCCEEDEマクロで判定できる
	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	//曽木課の根本な部分でエラーが出た場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));


	//いい順にアダプターを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&UseAdapter))
		!= DXGI_ERROR_NOT_FOUND; ++i)
	{
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = UseAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));
		//ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//採用したアダプタの情報をログに出力
			Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		UseAdapter = nullptr;//ソフトウェアアダプタの場合は見なかったことにする
	}
	//適切なアダプターが見つからなかったので起動できない
	assert(UseAdapter != nullptr);

	// デバイスの生成
	//昨日レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelString[] = { "12.2", "12.1" , "12.0" };
	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i)
	{
		//採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(UseAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		//指定した機能レベルでデバイス生成
		if (SUCCEEDED(hr)) {
			//生成できたのでログ出力を行ってループを抜ける
			Logger::Log(std::format("FeatureLevel : {} \n", featureLevelString[i]));
			break;
		}
	}
	//デバイスの生成がうまくいかなかったので起動できない
	assert(device != nullptr);
	Logger::Log("Complete create DeD12Device!!!\n");//初期化完了のログを出す

#ifdef _DEBUG

	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&InfoQueue))))
	{
		//ヤバイエラー時に止まる
		InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		//抑制するメッセージID
		D3D12_MESSAGE_ID denyIds[] = {
			//
			//
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑圧するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList; denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		InfoQueue->PushStorageFilter(&filter);

		//解放
		//InfoQueue->Release();
	}
#endif

}

void DirectXCommon::CommandsInitialize()
{
	

	//コマンドアロケータを生成する
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドリストを生成する
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	//コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドキューを生成する
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(commandQueue.GetAddressOf()));
	//コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

}

void DirectXCommon::CreateSwapChain()
{
	
	//スワップチェーンを生成する

	swapChainDesc.Width = WindowsAPI::kClientWidth;//画面の幅と高さをクライアントと同じにする
	swapChainDesc.Height = WindowsAPI::kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
	swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲット
	swapChainDesc.BufferCount = 2;//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタに移したら中身を破棄
	//コマンドキュー,ウィンドウハンドル,設定を渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), windowsAPI->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateDepthBuffer()
{
	// 生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = WindowsAPI::kClientWidth;; // Textureの幅
	resourceDesc.Height = WindowsAPI::kClientHeight; // Textureの高さ
	resourceDesc.MipLevels = 1; // mipmapの数
	resourceDesc.DepthOrArraySize = 1; // 奥行き or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント、1固定。
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f; // 1.0f (最大値) でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット。Resourceと合わせる

	// Resourceの生成
	hr = device->CreateCommittedResource(
		&heapProperties, // Heapの設定
		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定。特になし。
		&resourceDesc, // Resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値を書き込む状態にしておく
		&depthClearValue, // Clear最適値
		IID_PPV_ARGS(&depthBufferResource) // 作成するResourceポインタへのポインタ
	);

	assert(SUCCEEDED(hr));

}

void DirectXCommon::CreateDescriptorHeaps()
{
	descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
}

void DirectXCommon::RenderTargetViewInitialize()
{
	

	//SwapChainから Resourceを引っ張ってくる
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//うまく取得できなければ起動しない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

	//RTVの設定

	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//２ｄテクスチャとして書き込む
	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	
	//まず1つ目を作る。１つ目の最初のところに作る。作る場所を指定してあげる必要がある
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
	//2つ目のディスクリプタハンドルを得る
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//２つ目を作る
	device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);

}

void DirectXCommon::DepthStencilViewInitialize()
{
	//DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//DSVHeapの先頭に
	device->CreateDepthStencilView(depthBufferResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());


}

void DirectXCommon::FenceInitialize()
{
	
	//初期値0でFenceを作る
	
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	//FenceのSignalを待つためのイベントを作成する
	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);
}

void DirectXCommon::ViewportRectangleInitialize()
{

	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = WindowsAPI::kClientWidth;
	viewport.Height = WindowsAPI::kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

void DirectXCommon::ScissoringRectangle()
{
	//基本的にビューポートと同じ矩形が個性されるようにする
	scissorRect.left = 0;
	scissorRect.right = WindowsAPI::kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WindowsAPI::kClientHeight;
}

void DirectXCommon::CreateDXCCompiler()
{


	//dxcCompilerを初期化

	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::ImGuiInitialize()
{
	//ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(windowsAPI->GetHwnd());
	ImGui_ImplDX12_Init(device.Get(),
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap.Get(),
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index)
{
	return GetCPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index)
{
	return GetGPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, index);
}

void DirectXCommon::PreDraw()
{
	//これから書き込むバッファのインデックスを取得する
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	//TransitionBarrierの設定
	
	//今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
	//偏移前(現在)のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;

	//偏移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);

	// 描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);

	//指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f, 0.125f, 0.5f, 1.0f }; //青っぽい色	RGBAの順
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList->RSSetViewports(1, &viewport);//viewportを設定
	commandList->RSSetScissorRects(1, &scissorRect);//scirssorを設定

	//描画用のDescriptorHeapの設定
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptoHeaps[] = { srvDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, descriptoHeaps->GetAddressOf());
}

void DirectXCommon::PostDraw()
{
	//これから書き込むバッファのインデックスを取得する
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	//TransitionBarrierの設定
	

	//今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
	//偏移前(現在)のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);

	//コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	HRESULT hr = commandList->Close();
	assert(SUCCEEDED(hr));

	//GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(1, commandLists);
	//GPUとOSに画面の交換を行うよう通知する
	swapChain->Present(1, 0);

	//Fenceの値を更新
	fenceValue++;

	//GPUがここまでたどり着いたとき、Fenceの値を代入するようにSignalを送る
	commandQueue->Signal(fence.Get(), fenceValue);

	//GetCompleteValueの初期値はFence作成時に渡した初期値
	if (fence->GetCompletedValue() < fenceValue)
	{
		//指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを指定する
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		//イベントを待つ
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	//次のフレーム用のコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));

	UpdateFixFPS();

}

Microsoft::WRL::ComPtr<IDxcBlob> DirectXCommon::CompileShader(const std::wstring& filePath, const wchar_t* profile)
{


	//ここからシェーダーをコンパイルする旨をログに出す
	Logger::Log(StringUtility::ConvertString(std::format(L"begin Compiler, path:{}, profile:{}\n", filePath, profile)));
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	
	hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr));
	//読み込んだファイル内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;// utf-8の文字コードであることを確認

	LPCWSTR arguments[] = {
		filePath.c_str(),	//コンパイル対象ファイル
		L"-E", L"main",	//エントリーポイントの指定。基本的にmain以外にはしない
		L"-T", profile,		//ShaderProfileに指定
		L"-Zi", L"-Qembed_debug",	//デバッグ用の情報を埋め込む
		L"-Od", //	最適化を外しておく
		L"-Zpr", //メモリレイアウトは行優先
	};
	//実際にShaderをコンパイルする
	
	
	//コンパイルエラーではなくdxcが起動できないなどの致命的な状況
	assert(SUCCEEDED(hr));

	//警告・エラーが出たらログを出して止める

	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));
	
	hr = dxcCompiler->Compile(&shaderSourceBuffer,arguments,_countof(arguments),includeHandler.Get(), IID_PPV_ARGS(&shaderResult));


	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Logger::Log(shaderError->GetStringPointer());
		//警告,エラー[
		assert(false);
	}

	//コンパイル結果から実行用バイナリ部分を取得
	
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlod), nullptr);
	//成功したログを出す
	Logger::Log(StringUtility::ConvertString(std::format(L"Compile Succeded, path:{}, profile:{}\n", filePath, profile)));
	//もう使わないリソースを開放
	//shaderSource->Release();
	//shaderResult->Release();
	//実行用のバイナリを返却
	return shaderBlod;


	
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(size_t sizeInBytes)
{
	

	//頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//	uploadHeapを使う
	//頂点リソースの決定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	//バッファリソース。手くすりゃの場合は別の設定
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;//リソースのサイズ。今回はVector4を三頂点分
	//バッファの場合はこれらを1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれをする決まり	
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	
	hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&VertexResource));
	assert(SUCCEEDED(hr));

	return VertexResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata)
{
	//metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);//Textureの幅
	resourceDesc.Height = UINT(metadata.height);//Textureの高さ
	resourceDesc.MipLevels = UINT(metadata.mipLevels);//mipmapの幅
	resourceDesc.DepthOrArraySize = UINT(metadata.arraySize);//　奥行き　or　配列Textureの配列数
	resourceDesc.Format = metadata.format;//TextureのFormat
	resourceDesc.SampleDesc.Count = 1;//サンプリングカウント
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);//Textureの次元数。普段は2次元

	//利用するHeapの設定。非常に特殊
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;//細かい設定を行う
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//WriteBackポリシーでCPUアクセス可能
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//プロセッサの近くに配置

	
	hr = device->CreateCommittedResource(
		&heapProperties,//Heapの設定
		D3D12_HEAP_FLAG_NONE,//Heapの特殊な設定
		&resourceDesc,//Resourceの設定
		D3D12_RESOURCE_STATE_GENERIC_READ,//作成するResourceのポインタへのポインタ
		nullptr,//Clearの最高値。使わないのでnullptr
		IID_PPV_ARGS(&createTextureResource));
	assert(SUCCEEDED(hr));
	return createTextureResource;

	
}

void DirectXCommon::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages)
{
	//Meta情報を取得
	const DirectX::TexMetadata& metaData = mipImages.GetMetadata();
	//全Mipmapについて
	for (size_t mipLevel = 0; mipLevel < metaData.mipLevels; ++mipLevel)
	{
		//MipMapLevelを指定して各Imageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		//Textureに転送
		HRESULT hr = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr,//全領域へのコピー
			img->pixels,//元データアクセス
			UINT(img->rowPitch),//1ラインサイズ
			UINT(img->slicePitch)//1枚サイズ
		);
		assert(SUCCEEDED(hr));
	}

}

DirectX::ScratchImage DirectXCommon::LoadTexture(const std::string& filePath)
{//テクスチャファイルを選んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);

	//ミップマップの生成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);

	//ミップマップ月のデータを返す
	return mipImages;
}


Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

void DirectXCommon::InitializeFixFPS()
{
	reference_ = std::chrono::steady_clock::now();

}

void DirectXCommon::UpdateFixFPS()
{
	// 1/60秒ぴったりの時間
	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
	// 1/60秒よりわずかに短い時間
	const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

	// 現在時間を取得する
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	// 前回記録からの経過時間を取得する
	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	// 1/60秒（よりわずかに短い時間）経っていない場合
	if (elapsed < kMinTime) {

		// 1/60秒経過するまで微小なスリープを繰り返す
		while (std::chrono::steady_clock::now() - reference_ < kMinTime)
		{
			// 1マイクロ秒スリープ
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}

	}
	reference_ = std::chrono::steady_clock::now();
}

// 指定番号のCPUデスクリプタハンドルを取得する
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;

}

// 指定番号のGPUデスクリプタハンドルを取得する
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptoprHeap, uint32_t descripotrSize, uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptoprHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descripotrSize * index);
	return handleGPU;
}



void DirectXCommon::Finalize()
{
	//ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CloseHandle(fenceEvent);
}

