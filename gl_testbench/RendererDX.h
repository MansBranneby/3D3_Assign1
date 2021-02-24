#pragma once

#include "Renderer.h"
#include "MaterialDX.h"
#include "MeshDX.h"
#include "RenderStateDX.h"
#include "VertexBufferDX.h"

#include <SDL.h>
#include "SDL_syswm.h"

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "d3dcompiler.lib")

#pragma comment(lib,"SDL2.lib")
#pragma comment(lib,"SDL2main.lib")

struct vbStruct
{
	float x, y, z;
	float u, v;
};

class RendererDX : public Renderer
{
public:
	RendererDX();
	~RendererDX();

	Material* makeMaterial(const std::string& name);
	Mesh* makeMesh();
	//VertexBuffer* makeVertexBuffer();
	VertexBuffer* makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage);
	ConstantBuffer* makeConstantBuffer(std::string NAME, unsigned int location);
	//	ResourceBinding* makeResourceBinding();
	RenderState* makeRenderState();
	Technique* makeTechnique(Material* m, RenderState* r);
	Texture2D* makeTexture2D();
	Sampler2D* makeSampler2D();
	std::string getShaderPath();
	std::string getShaderExtension();

	int initialize(unsigned int width = 640, unsigned int height = 480);
	void setWinTitle(const char* title);
	int shutdown();

	void setClearColor(float, float, float, float);
	void clearBuffer(unsigned int);
	//	void setRenderTarget(RenderTarget* rt); // complete parameters
	void setRenderState(RenderState* ps);
	void submit(Mesh* mesh);
	void frame();
	void present();

private:
	SDL_Window* _window;
	HWND _wndHandle;

	ID3D12Device5* _device = nullptr;
	ID3D12CommandQueue* _commandQueue = nullptr;
	ID3D12CommandAllocator* _commandAllocator = nullptr;
	ID3D12GraphicsCommandList* _commandList = nullptr;

	ID3D12RootSignature* _rootSignature = nullptr;
	D3D12_INPUT_LAYOUT_DESC _inputLayoutDesc;

	ID3D12DescriptorHeap* _descriptorHeapBackBuffer = nullptr;
	ID3D12DescriptorHeap* _descriptorHeap[2] = {};
	UINT _rtvDescriptorSize = 0;

	ID3D12Fence1* _fence = nullptr;
	HANDLE _eventHandle = nullptr;
	UINT64 _fenceValue = 0;

	IDXGISwapChain4* _swapChain = nullptr;;
	D3D12_VIEWPORT _viewPort = {};
	D3D12_RECT _scissorRect = {};

	ID3D12Resource1* _renderTargets[2] = {}; // Backbuffer
//	ID3D12Resource1* _constantBuffers[2] = {};
	ID3D12Resource1* _SRVResource = nullptr;
	ID3D12Resource1* _VBResource = nullptr;
	
	D3D12_VERTEX_BUFFER_VIEW _VBView;

	float _clearColour[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	
	std::vector<Mesh*> _drawList;
	int _nrOfMeshes = 0;

	void createDevice();
	void createCommandQueue();
	void createFence();
	void createRenderTargetDescriptor();
	void createViewPort(unsigned int width, unsigned int height);
	void createRootSignature();
	void createDescriptorHeap();
	void createConstantBuffers();
	void createSRV();
	void createVertexBuffer();
};