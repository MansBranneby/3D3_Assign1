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
	const int MACRO_SIZE = 10;
	std::map<ShaderType, std::string>::iterator it = shaderFileNames.begin();
	for (it; it != shaderFileNames.end(); it++)
	{
		if (it->second != "")
		{
			std::vector<std::string> defines = createShaderMacros(it->first);
			D3D_SHADER_MACRO macros[MACRO_SIZE];
			for (int i = 0; i < MACRO_SIZE*2; i+=2)
			{
				if (i < defines.size())
					macros[i / 2] = { defines[i].c_str(), defines[i + 1].c_str()};
				else
					macros[i / 2] = { NULL, NULL };
			}

			auto debug1 = _shadercompileStrings[it->first].first.c_str();
			auto debug2 = _shadercompileStrings[it->first].second.c_str();
		

			std::wstring filename = std::wstring(it->second.begin(), it->second.end()).c_str();
			HRESULT hr = D3DCompileFromFile(
				filename.c_str(), // filename
				macros,		// optional macros
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
	_constantBuffers[location] = new ConstantBufferDX(name, location, _constantBuffer);
}

void MaterialDX::updateConstantBuffer(const void* data, size_t size, unsigned int location)
{
	_constantBuffers[location]->setData(data, size, this, location);
}

int MaterialDX::enable()
{
	return 0;
}

void MaterialDX::disable()
{
}

void MaterialDX::createConstantBuffer(ID3D12Device5* device)
{
	// Create descriptor heap
	for (int i = 0; i < 2; i++)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc = {};
		heapDescriptorDesc.NumDescriptors = 1;
		heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		HRESULT hr = device->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&_descriptorHeap[i]));
		if (FAILED(hr))
			MessageBox(NULL, L"Error", L"Error: _descriptorHeap", MB_OK | MB_ICONERROR);
	}

	// Create constant buffer
	UINT cbSizeAligned = (sizeof(float) * 4 + 255) & ~255;	// 256-byte aligned CB.
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.CreationNodeMask = 1; //used when multi-gpu
	heapProperties.VisibleNodeMask = 1; //used when multi-gpu
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = cbSizeAligned;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//Create a resource heap, descriptor heap, and pointer to cbv for each frame
	for (int i = 0; i < 2; i++)
	{
		device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&_constantBuffer[i])
		);

		_constantBuffer[i]->SetName(L"cb heap");

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = _constantBuffer[i]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = cbSizeAligned;
		device->CreateConstantBufferView(&cbvDesc, _descriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());
	}
}

std::vector<std::string> MaterialDX::createShaderMacros(ShaderType type)
{
	// Convert shaderDefines to parsed strings
	auto it = shaderDefines[type].begin();
	std::string defStr = *it;
	
	char* tempStr = new char[defStr.size()];
	strcpy(tempStr, defStr.c_str());

	std::string line, define,value;
	std::vector<std::string> defVec;
	std::stringstream defStream(tempStr);
	while (std::getline(defStream, line))
	{
		define = line.substr(0, line.rfind(' '));
		value = line.substr(line.rfind(' ')+1, line.size());

		defVec.push_back(define);
		defVec.push_back(value);
	}

	return defVec;
}
