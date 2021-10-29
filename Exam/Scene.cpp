#include "Scene.h"

CScene::CScene()
{
}
CScene::~CScene()
{
}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	//������ Ȯ���� ������ �����̴�. x-��� z-���� 8��, y-���� 2�� Ȯ���Ѵ�. 
	XMFLOAT3 xmf3Scale(8.0f, 1.0f, 8.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.2f, 0.0f, 0.0f);
	//������ ���� �� �̹��� ����(HeightMap.raw)�� ����Ͽ� �����Ѵ�. ���� ���� ũ��� ����x����(257x257)�̴�. 
#ifdef _WITH_TERRAIN_PARTITION
	/*�ϳ��� ���� �޽��� ũ��� ����x����(17x17)�̴�. ���� ��ü�� ���� �������� 16��, ���� �������� 16�� ���� ��
	���� ������. ������ �����ϴ� ���� �޽��� ������ �� 256(16x16)���� �ȴ�.*/
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList,
		m_pd3dGraphicsRootSignature, _T("../Assets/HeightMap.raw"), 257, 257, 17,
		17, xmf3Scale, xmf4Color);
#else
//������ �ϳ��� ���� �޽�(257x257)�� �����Ѵ�. 
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList,
		m_pd3dGraphicsRootSignature, _T("../Assets/HeightMap.raw"), 257, 257, 257,
		257, xmf3Scale, xmf4Color);
#endif
	m_nShaders = 1;
	m_pShaders = new CObjectsShader[m_nShaders];
	m_pShaders[0].CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	m_pShaders[0].BuildObjects(pd3dDevice, pd3dCommandList, m_pTerrain);

	m_pCrosshair = new CCrosshairShader;
	m_pCrosshair->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);

	m_pBullet.CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	m_pBullet.BuildObjects(pd3dDevice, pd3dCommandList, m_pTerrain);
}

void CScene::ReleaseObjects()
{
	if (m_pTerrain) delete m_pTerrain;
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].ReleaseShaderVariables();
		m_pShaders[i].ReleaseObjects();
	}
	if (m_pShaders) delete[] m_pShaders;

	if (m_pCrosshair) delete m_pCrosshair;

	m_pBullet.ReleaseObjects();
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
	for (int i = 0; i < m_nShaders; i++) m_pShaders[i].ReleaseUploadBuffers();

	m_pBullet.ReleaseUploadBuffers();
}
void CScene::AnimateObjects(float fTimeElapsed, CPlayer* player)
{
	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].AnimateObjects(fTimeElapsed, player, m_pTerrain);
	}

	m_pBullet.AnimateObjects(fTimeElapsed);
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	pCamera->UpdateShaderVariables(pd3dCommandList);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].Render(pd3dCommandList, pCamera);
	}

	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode != THIRD_PERSON_CAMERA)
		m_pCrosshair->Render(pd3dCommandList);

	m_pBullet.Render(pd3dCommandList, pCamera);
}

ID3D12RootSignature* CScene::GetGraphicsRootSignature()
{
	return(m_pd3dGraphicsRootSignature);
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[2];
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[0].Constants.Num32BitValues = 16;
	pd3dRootParameters[0].Constants.ShaderRegister = 0;
	pd3dRootParameters[0].Constants.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 32;
	pd3dRootParameters[1].Constants.ShaderRegister = 1;
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void
			**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}


bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

CGameObject* CScene::PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera)
{
	XMFLOAT3 xmf3PickPosition;
	//xmf3PickPosition.x = (((2.0f * xClient) / pCamera->m_d3dViewport.Width) - 1) / pCamera->m_xmf4x4Projection._11;
	//xmf3PickPosition.y = -(((2.0f * yClient) / pCamera->m_d3dViewport.Height) - 1) / pCamera->m_xmf4x4Projection._22;

	// ȭ�� �߾ӿ� ��ǥ
	if (pCamera->GetMode() == FIRST_PERSON_CAMERA)
	{
		xmf3PickPosition.x = 0 / pCamera->m_xmf4x4Projection._11;
		xmf3PickPosition.y = 0 / pCamera->m_xmf4x4Projection._22;
		xmf3PickPosition.z = 1.0f;
	}
	else
	{
		xmf3PickPosition.x = 0 / pCamera->m_xmf4x4Projection._11;
		xmf3PickPosition.y = -((420 / pCamera->m_d3dViewport.Height) - 1) / pCamera->m_xmf4x4Projection._22;
		xmf3PickPosition.z = 1.0f;
	}

	

	XMVECTOR xmvPickPosition = XMLoadFloat3(&xmf3PickPosition);
	XMMATRIX xmmtxView = XMLoadFloat4x4(&pCamera->m_xmf4x4View);

	int nIntersected = 0;
	float fNearestHitDistance = FLT_MAX;
	CRotatingObject* pNearestObject = NULL;
	for (int i = 0; i < m_pShaders[0].m_nObjects; i++)
	{
		if (m_pShaders[0].m_ppObjects[i]->state != DESTROY)
		{
			float fHitDistance = FLT_MAX;
			nIntersected = m_pShaders[0].m_ppObjects[i]->PickObjectByRayIntersection(xmvPickPosition, xmmtxView, &fHitDistance);
			if ((nIntersected > 0) && (fHitDistance < fNearestHitDistance) && (fHitDistance <= 250.0f))
			{
				fNearestHitDistance = fHitDistance;
				pNearestObject = m_pShaders[0].m_ppObjects[i];
			}
		}
	}

	// �ѱ� �߻���ġ�� ���� ����
	XMMATRIX inverse_xmmtxView = XMMatrixInverse(NULL, xmmtxView);
	XMFLOAT4X4 m;
	XMStoreFloat4x4(&m, inverse_xmmtxView);
	XMFLOAT3 rayDirection, rayOrigin;
	rayDirection.x = xmf3PickPosition.x * m._11 + xmf3PickPosition.y * m._21 + xmf3PickPosition.z * m._31;
	rayDirection.y = xmf3PickPosition.x * m._12 + xmf3PickPosition.y * m._22 + xmf3PickPosition.z * m._32;
	rayDirection.z = xmf3PickPosition.x * m._13 + xmf3PickPosition.y * m._23 + xmf3PickPosition.z * m._33;
	rayOrigin.x = m._41;
	rayOrigin.y = m._42;
	rayOrigin.z = m._43;
	

	// ź������ ����Ʈ
	if (pNearestObject)
	{
		XMVECTOR position;
		XMFLOAT3 f_position;
		position = XMVectorAdd(XMLoadFloat3(&rayOrigin), XMVectorScale(XMLoadFloat3(&rayDirection), fNearestHitDistance));
		XMStoreFloat3(&f_position, position);
		m_pBullet.SetObject(f_position);

		pNearestObject->HP -= 1;
	}
	else
	{
		XMVECTOR position;

		for (int i = 1; i < 250; ++i)
		{
			position = XMVectorAdd(XMLoadFloat3(&rayOrigin), XMVectorScale(XMLoadFloat3(&rayDirection), i));
			if (m_pTerrain->GetHeight(XMVectorGetX(position), XMVectorGetZ(position)) > XMVectorGetY(position))
			{
				position = XMVectorAdd(XMLoadFloat3(&rayOrigin), XMVectorScale(XMLoadFloat3(&rayDirection), i-1));
				XMFLOAT3 f_position;
				XMStoreFloat3(&f_position, position);
				m_pBullet.SetObject(f_position);
				break;
			}
		}

	}

	return(pNearestObject);
}