#pragma once
#include "pch.h"
#include "DeviceResources.h"

using namespace DirectX;
using namespace Microsoft::WRL;

class Scene
{
private:
	ComPtr<ID3D12RootSignature> m_rootSignature;

	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

public:
	void LoadAssets(ID3D12Device *m_device);

	void Render(ID3D12CommandList *CommandList);
};