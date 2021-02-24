#pragma once
#include <d3d12.h>

#include "Mesh.h"
#include "TransformDX.h"
#include "ConstantBufferDX.h"

class MeshDX :
    public Mesh
{
public:
    MeshDX(ID3D12Device5* device, ID3D12DescriptorHeap** descriptorHeap, int nrOfMeshes);

    void mapCBData(int backBufferIndex);

private:
    ID3D12Resource1* _constantBuffers[2] = {};

    void createConstantBuffer(ID3D12Device5* device, ID3D12DescriptorHeap** descriptorHeap, int nrOfMeshes);
};

