#include "ConstantBufferDX.h"

ConstantBufferDX::ConstantBufferDX(std::string NAME, unsigned int location)
{
	_name = NAME;
	_location = location;
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
