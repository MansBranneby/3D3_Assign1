#include "ConstantBufferDX.h"
#include "IA.h"

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
	if (location == TRANSLATION)
	{
		memcpy(&_cbData.translate, &data, size);
		memcpy(&_cbData.color, &m->color, sizeof(Color));
	}
	else if (location == DIFFUSE_TINT)
		memcpy(&_cbData.color, &data, size);
}

void ConstantBufferDX::bind(Material*)
{
}
