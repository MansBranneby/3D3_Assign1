#include "RenderStateDX.h"

RenderStateDX::RenderStateDX()
{
	_wireframe = false;
}

RenderStateDX::~RenderStateDX()
{
}

void RenderStateDX::setWireFrame(bool wireframe)
{
	_wireframe = wireframe;
}

void RenderStateDX::set()
{
}

void RenderStateDX::createPipelineState(ID3D12Device5* device, ID3D12RootSignature* rootSignature, MaterialDX* m)
{
	////// Input Layout //////
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXTCOORD"	, 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = ARRAYSIZE(inputElementDesc);


	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd = {};

	//Specify pipeline stages:
	gpsd.pRootSignature = rootSignature;
	gpsd.InputLayout = inputLayoutDesc;
	gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(m->getShaderBlob(Material::ShaderType::VS)->GetBufferPointer());
	gpsd.VS.BytecodeLength = m->getShaderBlob(Material::ShaderType::VS)->GetBufferSize();
	gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(m->getShaderBlob(Material::ShaderType::PS)->GetBufferPointer());
	gpsd.PS.BytecodeLength = m->getShaderBlob(Material::ShaderType::PS)->GetBufferSize();

	//Specify render target and depthstencil usage.
	gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsd.NumRenderTargets = 1;

	gpsd.SampleDesc.Count = 1;
	gpsd.SampleMask = UINT_MAX;

	//Specify rasterizer behaviour.
	gpsd.RasterizerState.FillMode = this->isWireframe() ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
	gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

	//Specify blend descriptions.
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL };
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		gpsd.BlendState.RenderTarget[i] = defaultRTdesc;

	HRESULT hr = device->CreateGraphicsPipelineState(&gpsd, IID_PPV_ARGS(&_pipeLineState));
	if (FAILED(hr))
		MessageBox(NULL, L"Error", L"Error: PipeLineState", MB_OK | MB_ICONERROR);
}

ID3D12PipelineState* RenderStateDX::getPipeLineState()
{
	return _pipeLineState;
}

bool RenderStateDX::isWireframe()
{
	return _wireframe;
}
