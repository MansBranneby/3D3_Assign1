#pragma once

#include "Material.h"
#include "ConstantBufferDX.h"

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h> //Only used for initialization of the device and swap chain.
#include <d3dcompiler.h>

#include <iterator>
#include <iostream>
#include <sstream>
#include <vector>

template<class Interface>
inline void SafeRelease(
	Interface** ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = NULL;
	}
}

class MaterialDX : public Material
{
public:
	MaterialDX(const std::string& name);
	~MaterialDX();

	// set shader name, DOES NOT COMPILE
	virtual void setShader(const std::string& shaderFileName, ShaderType type);

	// removes any resource linked to shader type
	virtual void removeShader(ShaderType type);

	virtual void setDiffuse(Color c);

	/*
	 * Compile and link all shaders
	 * Returns 0  if compilation/linking succeeded.
	 * Returns -1 if compilation/linking fails.
	 * Error is returned in errString
	 * A Vertex and a Fragment shader MUST be defined.
	 * If compileMaterial is called again, it should RE-COMPILE the shader
	 * In principle, it should only be necessary to re-compile if the defines set
	 * has changed.
	*/
	virtual int compileMaterial(std::string& errString);

	// this constant buffer will be bound every time we bind the material
	virtual void addConstantBuffer(std::string name, unsigned int location);

	// location identifies the constant buffer in a unique way
	virtual void updateConstantBuffer(const void* data, size_t size, unsigned int location);

	// activate the material for use.
	virtual int enable();

	// disable material
	virtual void disable();

	ID3DBlob* getShaderBlob(ShaderType type);
	ConstantBufferDX* getConstantBuffer();

private:
	std::map<ShaderType, std::pair<std::string, std::string>> _shadercompileStrings;
	ID3DBlob* _shaderBlob[4] = {nullptr, nullptr, nullptr, nullptr};

	ConstantBufferDX* _constantBuffer;

	std::vector<std::string> createShaderMacros(ShaderType type);
};


