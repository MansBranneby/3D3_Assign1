#include "OpenGL/OpenGLRenderer.h"
#include "RendererDX.h"
#include "Renderer.h"


Renderer* Renderer::makeRenderer(BACKEND option)
{
	if (option == BACKEND::GL45)
		return new OpenGLRenderer();
	if (option == BACKEND::DX12)
		return new RendererDX();
}

