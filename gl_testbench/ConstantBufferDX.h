#pragma once

#include "ConstantBuffer.h"

#include <windows.h>
#include <d3d12.h>

class ConstantBufferDX : public ConstantBuffer
{


public:
	ConstantBufferDX(std::string NAME, unsigned int location);
	~ConstantBufferDX();

	// set data will update the buffer associated, including whatever is necessary to
	// update the GPU memory.
	void setData(const void* data, size_t size, Material* m, unsigned int location);
	void bind(Material*);

private:
	std::string _name;
	int _location;
};

