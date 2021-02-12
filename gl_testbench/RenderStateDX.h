#pragma once
#include "RenderState.h"
#include <d3d12.h>

class RenderStateDX :
    public RenderState
{
public:
	RenderStateDX();
	~RenderStateDX();
	void setWireFrame(bool wireframe);
	void set();
	ID3D12PipelineState* getPipeLineState();
	bool isWireframe();

private:
	ID3D12PipelineState* _pipeLineState = nullptr;

	bool _wireframe;
};

