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
    //for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
    //{
    //    D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc = {};
    //    heapDescriptorDesc.NumDescriptors = 1;
    //    heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    //    heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    //    gDevice5->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&gDescriptorHeap[i]));
    //}

    //UINT cbSizeAligned = (sizeof(ConstantBuffer) + 255) & ~255;    // 256-byte aligned CB.

    //D3D12_HEAP_PROPERTIES heapProperties = {};
    //heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    //heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    //heapProperties.CreationNodeMask = 1; //used when multi-gpu
    //heapProperties.VisibleNodeMask = 1; //used when multi-gpu
    //heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    //D3D12_RESOURCE_DESC resourceDesc = {};
    //resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    //resourceDesc.Width = cbSizeAligned;
    //resourceDesc.Height = 1;
    //resourceDesc.DepthOrArraySize = 1;
    //resourceDesc.MipLevels = 1;
    //resourceDesc.SampleDesc.Count = 1;
    //resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ////Create a resource heap, descriptor heap, and pointer to cbv for each frame
    //for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
    //{
    //    gDevice5->CreateCommittedResource(
    //        &heapProperties,
    //        D3D12_HEAP_FLAG_NONE,
    //        &resourceDesc,
    //        D3D12_RESOURCE_STATE_GENERIC_READ,
    //        nullptr,
    //        IID_PPV_ARGS(&gConstantBufferResource[i])
    //        );

    //    gConstantBufferResource[i]->SetName(L"cb heap");

    //    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    //    cbvDesc.BufferLocation = gConstantBufferResource[i]->GetGPUVirtualAddress();
    //    cbvDesc.SizeInBytes = cbSizeAligned;
    //    gDevice5->CreateConstantBufferView(&cbvDesc, gDescriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());
    //}
}

void ConstantBufferDX::bind(Material*)
{
}
