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
		struct TRANS
		{
			float x, y, z, w;
		};
		TRANS* trans = (TRANS*)data;
		//memcpy(&trans, &data, size);
		_cbData.translate[0] = trans->x;
		_cbData.translate[1] = trans->y;
		_cbData.translate[2] = trans->z;
		_cbData.translate[3] = 0.0f;
		//memcpy(&_cbData.translate, &data, size);
		memcpy(&_cbData.color, &m->color, sizeof(Color));
	}
	else if (location == DIFFUSE_TINT)
		memcpy(&_cbData.color, &data, size);
}

void ConstantBufferDX::bind(Material*)
{
}

CBStruct ConstantBufferDX::getCBData()
{
	return _cbData;
}
