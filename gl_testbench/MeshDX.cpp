#include "MeshDX.h"

MeshDX::MeshDX(ID3D12Device5* device, ID3D12DescriptorHeap** descriptorHeap, int nrOfMeshes)
{
	createConstantBuffer(device, descriptorHeap, nrOfMeshes);
}

void MeshDX::mapCBData(int backBufferIndex)
{
	CBStruct cbData = static_cast<ConstantBufferDX*>(this->txBuffer)->getCBData();
	//Update GPU memory
	void* mappedMem = nullptr;
	D3D12_RANGE readRange = { 0, 0 }; //We do not intend to read this resource on the CPU.
	if (SUCCEEDED(_constantBuffers[backBufferIndex]->Map(0, &readRange, &mappedMem)))
	{
		memcpy(mappedMem, &cbData, sizeof(CBStruct));

		D3D12_RANGE writeRange = { 0, sizeof(CBStruct) };
		_constantBuffers[backBufferIndex]->Unmap(0, &writeRange);
	}
}

void MeshDX::createConstantBuffer(ID3D12Device5* device, ID3D12DescriptorHeap** descriptorHeap, int nrOfMeshes)
{
	UINT cbSizeAligned = (sizeof(CBStruct) + 255) & ~255;	// 256-byte aligned CB.

	// DescriptorTable size
	int descriptorTableSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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
		HRESULT hr = device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&_constantBuffers[i])
		);
		if (FAILED(hr))
			MessageBox(NULL, L"Error", L"Error: CommittedResourceCB", MB_OK | MB_ICONERROR);

		_constantBuffers[i]->SetName(L"cb heap");

		// Increment based on number of created meshes
		D3D12_CPU_DESCRIPTOR_HANDLE cdh = descriptorHeap[i]->GetCPUDescriptorHandleForHeapStart();
		cdh.ptr += UINT(descriptorTableSize * nrOfMeshes);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = _constantBuffers[i]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = cbSizeAligned;
		device->CreateConstantBufferView(&cbvDesc, cdh);

	}
}
