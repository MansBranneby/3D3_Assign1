#include "RendererDX.h"

RendererDX::RendererDX()
{
}

RendererDX::~RendererDX()
{
}

Material* RendererDX::makeMaterial(const std::string& name)
{
	MaterialDX* m = new MaterialDX(name);

	m->createConstantBuffer(_device);

	return m;
}

Mesh* RendererDX::makeMesh()
{
	return nullptr;
}

VertexBuffer* RendererDX::makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage)
{
	return nullptr;
}

ConstantBuffer* RendererDX::makeConstantBuffer(std::string NAME, unsigned int location)
{
	return nullptr;
}

RenderState* RendererDX::makeRenderState()
{
	return nullptr;
}

Technique* RendererDX::makeTechnique(Material* m, RenderState* r)
{
	return nullptr;
}

Texture2D* RendererDX::makeTexture2D()
{
	return nullptr;
}

Sampler2D* RendererDX::makeSampler2D()
{
	return nullptr;
}

std::string RendererDX::getShaderPath()
{
	return std::string("..\\assets\\DX12\\");
}

std::string RendererDX::getShaderExtension()
{
	return std::string(".hlsl");
}

int RendererDX::initialize(unsigned int width, unsigned int height)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "%s", SDL_GetError());
		exit(-1);
	}

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);

	_window = SDL_CreateWindow("DirectX12", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL); // Behövs ändras?
	
	SDL_GetWindowWMInfo(_window, &wmInfo);
	_wndHandle = wmInfo.info.win.window;

	createDevice();
	createCommandQueue();
	createFence();
	createRenderTargetDescriptor();
	createDescriptorHeap();
	createViewPort(width, height);

	return 0;
}

void RendererDX::setWinTitle(const char* title)
{
	SDL_SetWindowTitle(this->_window, title);
}

int RendererDX::shutdown()
{
	return 0;
}

void RendererDX::setClearColor(float r, float g, float b, float a)
{
	_clearColour[0] = r;
	_clearColour[1] = g;
	_clearColour[2] = b;
	_clearColour[3] = a;
}

void RendererDX::clearBuffer(unsigned int)
{
}

void RendererDX::setRenderState(RenderState* ps)
{
}

void RendererDX::submit(Mesh* mesh)
{
}

void RendererDX::frame()
{
}

void RendererDX::present()
{
}

void RendererDX::createDevice()
{
#ifdef _DEBUG
	//Enable the D3D12 debug layer.
	ID3D12Debug* debugController = nullptr;

#ifdef STATIC_LINK_DEBUGSTUFF
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
	SafeRelease(debugController);
#else
	HMODULE mD3D12 = GetModuleHandle(L"D3D12.dll");
	PFN_D3D12_GET_DEBUG_INTERFACE f = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(mD3D12, "D3D12GetDebugInterface");
	if (SUCCEEDED(f(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
	SafeRelease(&debugController);
#endif
#endif

	//dxgi1_6 is only needed for the initialization process using the adapter.
	IDXGIFactory6* factory = nullptr;
	IDXGIAdapter1* adapter = nullptr;
	//First a factory is created to iterate through the adapters available.
	CreateDXGIFactory(IID_PPV_ARGS(&factory));
	for (UINT adapterIndex = 0;; ++adapterIndex)
	{
		adapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == factory->EnumAdapters1(adapterIndex, &adapter))
		{
			break; //No more adapters to enumerate.
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device5), nullptr)))
		{
			break;
		}

		SafeRelease(&adapter);
	}
	if (adapter)
	{
		HRESULT hr = S_OK;
		//Create the actual device.
		if (SUCCEEDED(hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&_device))))
		{

		}

		SafeRelease(&adapter);
	}
	else
	{
		//Create warp device if no adapter was found.
		factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_device));
	}

	SafeRelease(&factory);
}

void RendererDX::createCommandQueue()
{
	//Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC cqd = {};
	HRESULT hr = _device->CreateCommandQueue(&cqd, IID_PPV_ARGS(&_commandQueue));
	if (FAILED(hr))
		MessageBox(NULL, L"Error", L"Error: _commandQueue", MB_OK | MB_ICONERROR);

	//Create command allocator. The command allocator object corresponds
	//to the underlying allocations in which GPU commands are stored.
	hr = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator));
	if (FAILED(hr))
		MessageBox(NULL, L"Error", L"Error: _commandAllocator", MB_OK | MB_ICONERROR);

	//Create command list.
	hr = _device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		_commandAllocator,
		nullptr,
		IID_PPV_ARGS(&_commandList));
	if (FAILED(hr))
		MessageBox(NULL, L"Error", L"Error: _commandList", MB_OK | MB_ICONERROR);

	//Command lists are created in the recording state. Since there is nothing to
	//record right now and the main loop expects it to be closed, we close it.
	_commandList->Close();

	IDXGIFactory5* factory = nullptr;
	hr = CreateDXGIFactory(IID_PPV_ARGS(&factory));
	if (FAILED(hr))
		MessageBox(NULL, L"Error", L"Error: factory", MB_OK | MB_ICONERROR);

	//Create swap chain.
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = 0;
	scDesc.Height = 0;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = 2;
	scDesc.Scaling = DXGI_SCALING_NONE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = 0;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	IDXGISwapChain1* swapChain1 = nullptr;
	if (SUCCEEDED(factory->CreateSwapChainForHwnd(
		_commandQueue,
		_wndHandle,
		&scDesc,
		nullptr,
		nullptr,
		&swapChain1)))
	{
		if (SUCCEEDED(swapChain1->QueryInterface(IID_PPV_ARGS(&_swapChain))))
		{
			_swapChain->Release();
		}
	}

	SafeRelease(&factory);
}

void RendererDX::createFence()
{
	HRESULT hr =_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	if (FAILED(hr))
		MessageBox(NULL, L"Error", L"Error: _fence", MB_OK | MB_ICONERROR);
	_fenceValue = 1;
	//Create an event handle to use for GPU synchronization.
	_eventHandle = CreateEvent(0, false, false, 0);
}

void RendererDX::createRenderTargetDescriptor()
{
	//Create descriptor heap for render target views.
	D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
	dhd.NumDescriptors = 2;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	HRESULT hr = _device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&_descriptorHeapBackBuffer));
	if (FAILED(hr))
		MessageBox(NULL, L"Error", L"Error: _descriptorHeapBackBuffer", MB_OK | MB_ICONERROR);

	//Create resources for the render targets.
	_rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = _descriptorHeapBackBuffer->GetCPUDescriptorHandleForHeapStart();

	//One RTV for each frame.
	for (UINT n = 0; n < 2; n++)
	{
		hr = _swapChain->GetBuffer(n, IID_PPV_ARGS(&_renderTargets[n]));
		_device->CreateRenderTargetView(_renderTargets[n], nullptr, cdh);
		cdh.ptr += _rtvDescriptorSize;
	}
}

void RendererDX::createViewPort(unsigned int width, unsigned int height)
{
	_viewPort.TopLeftX = 0.0f;
	_viewPort.TopLeftY = 0.0f;
	_viewPort.Width = (float)width;
	_viewPort.Height = (float)height;
	_viewPort.MinDepth = 0.0f;
	_viewPort.MaxDepth = 1.0f;

	_scissorRect.left = (long)0;
	_scissorRect.right = (long)width;
	_scissorRect.top = (long)0;
	_scissorRect.bottom = (long)height;
}

void RendererDX::createRootSignature()
{
	//define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE  dtRanges[2];
	dtRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	dtRanges[0].NumDescriptors = 2;
	dtRanges[0].BaseShaderRegister = 0; //register b0
	dtRanges[0].RegisterSpace = 0; //register(b0,space0);
	dtRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	dtRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	dtRanges[1].NumDescriptors = 1;
	dtRanges[1].BaseShaderRegister = 0; //register t0
	dtRanges[1].RegisterSpace = 0; //register(t0,space0);
	dtRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE dt;
	dt.NumDescriptorRanges = ARRAYSIZE(dtRanges);
	dt.pDescriptorRanges = dtRanges;

	//create root parameter
	D3D12_ROOT_PARAMETER  rootParam[1];
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable = dt;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC sDesc;
	sDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sDesc.MipLODBias = 0;
	sDesc.MaxAnisotropy = 0;
	sDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sDesc.MinLOD = 0.0f;
	sDesc.MaxLOD = D3D12_FLOAT32_MAX;
	sDesc.ShaderRegister = 0;
	sDesc.RegisterSpace = 0;
	sDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC rsDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = ARRAYSIZE(rootParam);
	rsDesc.pParameters = rootParam;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.pStaticSamplers = &sDesc; // TODO: fatta detta

	ID3DBlob* sBlob;
	D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&sBlob,
		nullptr);

	_device->CreateRootSignature(
		0,
		sBlob->GetBufferPointer(),
		sBlob->GetBufferSize(),
		IID_PPV_ARGS(&_rootSignature));
}

void RendererDX::createDescriptorHeap()
{
	for (int i = 0; i < 2; i++)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc = {};
		heapDescriptorDesc.NumDescriptors = 1;
		heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		HRESULT hr = _device->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&_descriptorHeap[i]));
		if (FAILED(hr))
			MessageBox(NULL, L"Error", L"Error: _descriptorHeap", MB_OK | MB_ICONERROR);
	}
}