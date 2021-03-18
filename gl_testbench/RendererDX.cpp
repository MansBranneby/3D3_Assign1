#include "RendererDX.h"

RendererDX::RendererDX()
{
}

RendererDX::~RendererDX()
{
}

Material* RendererDX::makeMaterial(const std::string& name)
{
	return new MaterialDX(name);
}

Mesh* RendererDX::makeMesh()
{
	return new MeshDX(_device, _descriptorHeap, _nrOfMeshes++);
}

VertexBuffer* RendererDX::makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage)
{
	return new VertexBufferDX;
}

ConstantBuffer* RendererDX::makeConstantBuffer(std::string NAME, unsigned int location)
{
	return new ConstantBufferDX(NAME, location);
}

RenderState* RendererDX::makeRenderState()
{
	return new RenderStateDX();
}

Technique* RendererDX::makeTechnique(Material* m, RenderState* r)
{
	MaterialDX* mDX = static_cast<MaterialDX*>(m);
	RenderStateDX* rDX = static_cast<RenderStateDX*>(r);
	rDX->createPipelineState(_device, _rootSignature, mDX);

	return new Technique(m, r);
}

Texture2D* RendererDX::makeTexture2D()
{
	return new Texture2DDX();
}

Sampler2D* RendererDX::makeSampler2D()
{
	return new Sampler2DDX();
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
	createRootSignature();
	createDescriptorHeap();
	createViewPort(width, height);
	createConstantBuffers();
	createSRV();
	createVertexBuffer();

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
	if(_drawList.size() < 100)
		_drawList.push_back(mesh);

	int backBufferIndex = _swapChain->GetCurrentBackBufferIndex();
	static_cast<MeshDX*>(mesh)->mapCBData(backBufferIndex);
}

void RendererDX::frame()
{
	int backBufferIndex = _swapChain->GetCurrentBackBufferIndex();
	D3D12_RESOURCE_BARRIER barrierDesc = {};
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = _descriptorHeapBackBuffer->GetCPUDescriptorHandleForHeapStart();
	// DescriptorTable size
	UINT descriptorTableSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 2;
	D3D12_GPU_DESCRIPTOR_HANDLE heapHandle = _descriptorHeap[backBufferIndex]->GetGPUDescriptorHandleForHeapStart();

	//Command list allocators can only be reset when the associated command lists have
	//finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)
	_commandAllocator->Reset();
	_commandList->Reset(_commandAllocator, static_cast<RenderStateDX*>(_drawList[1]->technique->getRenderState())->getPipeLineState());

	//Set constant buffer descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { _descriptorHeap[backBufferIndex] };
	_commandList->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

	//Set root signature
	_commandList->SetGraphicsRootSignature(_rootSignature);

	//Set necessary states.
	_commandList->RSSetViewports(1, &_viewPort);
	_commandList->RSSetScissorRects(1, &_scissorRect);

	//Indicate that the back buffer will be used as render target.
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = _renderTargets[backBufferIndex];
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	_commandList->ResourceBarrier(1, &barrierDesc);

	//Record commands.
	//Get the handle for the current render target used as back buffer.
	cdh.ptr += _rtvDescriptorSize * backBufferIndex;

	_commandList->OMSetRenderTargets(1, &cdh, true, nullptr);

	_commandList->ClearRenderTargetView(cdh, _clearColour, 0, nullptr);

	_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_commandList->IASetVertexBuffers(0, 1, &_VBView);

	for (UINT i = 0; i < 100; i++)
	{
		//Set root descriptor table to index 0 in previously set root signature
		_commandList->SetPipelineState(static_cast<RenderStateDX*>(_drawList[i]->technique->getRenderState())->getPipeLineState());
		_commandList->SetGraphicsRootDescriptorTable(0, heapHandle);
		_commandList->DrawInstanced(3, 1, 0, 0);
		heapHandle.ptr += UINT64(descriptorTableSize);
	}



	//Indicate that the back buffer will now be used to present.
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	_commandList->ResourceBarrier(1, &barrierDesc);
	//Close the list to prepare it for execution.
	_commandList->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { _commandList };
	_commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	//Present the frame.
	DXGI_PRESENT_PARAMETERS pp = {};
	_swapChain->Present1(0, 0, &pp);

	//WAITING FOR EACH FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	//This is code implemented as such for simplicity. The cpu could for example be used
	//for other tasks to prepare the next frame while the current one is being rendered.

	//Signal and increment the fence value.
	const UINT64 fence = _fenceValue;
	_commandQueue->Signal(_fence, fence);
	_fenceValue++;

	//Wait until command queue is done.
	if (_fence->GetCompletedValue() < fence)
	{
		_fence->SetEventOnCompletion(fence, _eventHandle);
		WaitForSingleObject(_eventHandle, INFINITE);
	}
}

void RendererDX::present()
{
}

void RendererDX::createDevice()
{
#ifdef _DEBUG
	//Enable the D3D12 debug layer.
	ID3D12Debug3* debugController = nullptr;

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
		debugController->SetEnableGPUBasedValidation(true);
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

	////Command lists are created in the recording state. Since there is nothing to
	////record right now and the main loop expects it to be closed, we close it.
	//_commandList->Close();

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
	dtRanges[0].NumDescriptors = 1;
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
	sDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
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
		heapDescriptorDesc.NumDescriptors = 200;
		heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		HRESULT hr = _device->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&_descriptorHeap[i]));
		if (FAILED(hr))
			MessageBox(NULL, L"Error", L"Error: _descriptorHeap", MB_OK | MB_ICONERROR);
	}
}

void RendererDX::createConstantBuffers()
{
	// TODO: Remove function
}

void RendererDX::createSRV()
{
	std::string filename = "../assets/textures/fatboy.png";
	int w, h, bpp;
	unsigned char* texture = stbi_load(filename.c_str(), &w, &h, &bpp, STBI_rgb_alpha);
	// DescriptorTable size
	int descriptorTableSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 2;

	Microsoft::WRL::ComPtr<ID3D12Resource> textureUploadHeap;

	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Width = w;
	textureDesc.Height = h;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	
	HRESULT hr = _device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&_SRVResource));
	if (FAILED(hr))
		MessageBox(NULL, L"Error", L"Error: _SRVResource", MB_OK | MB_ICONERROR);

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(_SRVResource, 0, 1);

	// Create the GPU upload buffer.
	_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureUploadHeap));

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = &texture[0];
	textureData.RowPitch = w*(int)4;
	textureData.SlicePitch = textureData.RowPitch * h;

	UpdateSubresources(_commandList, _SRVResource, textureUploadHeap.Get(), 0, 0, 1, &textureData);
	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_SRVResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_CPU_DESCRIPTOR_HANDLE cdh;
	for (int i = 0; i < 2; i++)
	{
		cdh = _descriptorHeap[i]->GetCPUDescriptorHandleForHeapStart();
		cdh.ptr += _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		for (int j = 0; j < 100; j++)
		{
			_device->CreateShaderResourceView(_SRVResource, &srvDesc, cdh);
			cdh.ptr += descriptorTableSize;
		}
	}

	//Command lists are created in the recording state. Since there is nothing to
	//record right now and the main loop expects it to be closed, we close it.
	_commandList->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { _commandList };
	_commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	//Signal and increment the fence value.
	const UINT64 fence = _fenceValue;
	_commandQueue->Signal(_fence, fence);
	_fenceValue++;

	//Wait until command queue is done.
	if (_fence->GetCompletedValue() < fence)
	{
		_fence->SetEventOnCompletion(fence, _eventHandle);
		WaitForSingleObject(_eventHandle, INFINITE);
	}
}

void RendererDX::createVertexBuffer()
{
	vbStruct triangleVertices[3] =
	{
		0.0f, 0.05, 0.0f,   // pos1
		0.5f, -0.99f,	    // uv1

		0.05, -0.05, 0.0f,  // pos2
		1.49f, 1.1f,		// uv2

		- 0.05, -0.05, 0.0f,// pos3
		-0.51, 1.1f			// uv3
	};

	D3D12_HEAP_PROPERTIES hp = {};
	hp.Type = D3D12_HEAP_TYPE_UPLOAD;
	hp.CreationNodeMask = 1;
	hp.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC rd = {};
	rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rd.Width = sizeof(triangleVertices);
	rd.Height = 1;
	rd.DepthOrArraySize = 1;
	rd.MipLevels = 1;
	rd.SampleDesc.Count = 1;
	rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//Creates both a resource and an implicit heap, such that the heap is big enough
	//to contain the entire resource and the resource is mapped to the heap. 
	HRESULT hr = _device->CreateCommittedResource(
		&hp,
		D3D12_HEAP_FLAG_NONE,
		&rd,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_VBResource));
	if (FAILED(hr))
		MessageBox(NULL, L"Error", L"Error: _VBResource", MB_OK | MB_ICONERROR);

	_VBResource->SetName(L"vb heap");

	//Copy the triangle data to the vertex buffer.
	void* dataBegin = nullptr;
	D3D12_RANGE range = { 0, 0 }; //We do not intend to read this resource on the CPU.
	_VBResource->Map(0, &range, &dataBegin);
	memcpy(dataBegin, triangleVertices, sizeof(triangleVertices));
	_VBResource->Unmap(0, nullptr);

	//Initialize vertex buffer view, used in the render call.
	_VBView.BufferLocation = _VBResource->GetGPUVirtualAddress();
	_VBView.StrideInBytes = sizeof(vbStruct);
	_VBView.SizeInBytes = sizeof(triangleVertices);

}
