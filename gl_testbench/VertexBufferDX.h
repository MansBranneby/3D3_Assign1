#pragma once
#include "VertexBuffer.h"
class VertexBufferDX :
    public VertexBuffer
{
public:
	VertexBufferDX();

	void setData(const void* data, size_t size, size_t offset);
	void bind(size_t offset, size_t size, unsigned int location);
	void unbind();
	size_t getSize();
};

