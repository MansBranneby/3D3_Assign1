#pragma once
#include "RenderState.h"
#include "MaterialDX.h"
#include <d3d12.h>

class RenderStateDX :
    public RenderState
{
public:
	RenderStateDX();
	~RenderStateDX();
	void setWireFrame(bool wireframe);
	void set();
	void createPipelineState(ID3D12Device5* device, ID3D12RootSignature* rootSignature, MaterialDX* m);
	ID3D12PipelineState* getPipeLineState();
	bool isWireframe();

private:
	ID3D12PipelineState* _pipeLineState = nullptr;

	bool _wireframe;
};

