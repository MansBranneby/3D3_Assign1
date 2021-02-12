#include "ConstantBufferDX.h"

ConstantBufferDX::ConstantBufferDX(std::string NAME, unsigned int location, ID3D12Resource1** constantBuffer)
{
	_name = NAME;
	_location = location;
	_constantBuffer = constantBuffer;
}

ConstantBufferDX::~ConstantBufferDX()
{
}

void ConstantBufferDX::setData(const void* data, size_t size, Material* m, unsigned int location)
{

}

void ConstantBufferDX::bind(Material*)
{
}
