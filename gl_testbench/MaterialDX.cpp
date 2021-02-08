#include "MaterialDX.h"

MaterialDX::MaterialDX(const std::string& name)
{
	shaderFileNames[ShaderType::VS] = "";
	shaderFileNames[ShaderType::GS] = "";
	shaderFileNames[ShaderType::PS] = "";
	shaderFileNames[ShaderType::CS] = "";
}

MaterialDX::~MaterialDX()
{
}

void MaterialDX::setShader(const std::string& shaderFileName, ShaderType type)
{
	if (shaderFileNames.find(type) != shaderFileNames.end())
	{
		removeShader(type);
	}
	shaderFileNames[type] = shaderFileName;
	
	std::pair<std::string, std::string> compileStrings;
	switch (type)
	{
	case ShaderType::VS:
		compileStrings = std::make_pair("VS_main", "vs_5_0");
		break;
	case ShaderType::PS:
		compileStrings = std::make_pair("PS_main", "ps_5_0");
		break;
	default:
		compileStrings = std::make_pair("", "");
		break;
	}
	_shadercompileStrings[type] = compileStrings; 
}

void MaterialDX::removeShader(ShaderType type)
{
	shaderFileNames[type] = "";
	SafeRelease(&_shaderBlob[(int)type]);
}

void MaterialDX::setDiffuse(Color c)
{
}

int MaterialDX::compileMaterial(std::string& errString)
{
	std::map<ShaderType, std::string>::iterator it = shaderFileNames.begin();
	for (it; it != shaderFileNames.end(); it++)
	{
		if (it->second != "")
		{
			D3D_SHADER_MACRO* macros;
			createShaderMacros(macros, it->first);

			const wchar_t* widecstr = std::wstring(it->second.begin(), it->second.end()).c_str();
			HRESULT hr = D3DCompileFromFile(
				widecstr, // filename
				nullptr,		// optional macros
				nullptr,		// optional include files
				_shadercompileStrings[it->first].first.c_str(),		// entry point
				_shadercompileStrings[it->first].second.c_str(),		// shader model (target)
				0,				// shader compile options			// here DEBUGGING OPTIONS
				0,				// effect compile options
				&_shaderBlob[(int)it->first],	// double pointer to ID3DBlob		
				nullptr			// pointer for Error Blob messages.
								// how to use the Error blob, see here
								// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
			);
			if (FAILED(hr))
				MessageBox(NULL, L"Error", L"Error: Shader compile", MB_OK | MB_ICONERROR);
		}
	}

	////// Input Layout //////
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXTCOORD"	, 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	_inputLayoutDesc.pInputElementDescs = inputElementDesc;
	_inputLayoutDesc.NumElements = ARRAYSIZE(inputElementDesc);

	return 0;
}

void MaterialDX::addConstantBuffer(std::string name, unsigned int location)
{
}

void MaterialDX::updateConstantBuffer(const void* data, size_t size, unsigned int location)
{
}

int MaterialDX::enable()
{
	return 0;
}

void MaterialDX::disable()
{
}

void MaterialDX::createShaderMacros(D3D_SHADER_MACRO* macros, ShaderType type)
{
}
