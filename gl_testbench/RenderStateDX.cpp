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

ID3D12PipelineState* RenderStateDX::getPipeLineState()
{
	return _pipeLineState;
}

bool RenderStateDX::isWireframe()
{
	return _wireframe;
}
