#include "GameObject.h"
#include "Shader.h"

CGameObject::CGameObject(int nMeshes)
{
	m_xmf4x4World = Matrix4x4::Identity();
	m_nMeshes = nMeshes;
	m_ppMeshes = NULL;
	if (m_nMeshes > 0)
	{
		m_ppMeshes = new CMesh * [m_nMeshes];
		for (int i = 0; i < m_nMeshes; i++) m_ppMeshes[i] = NULL;
	}
}

CGameObject::~CGameObject()
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Release();
			m_ppMeshes[i] = NULL;
		}
		delete[] m_ppMeshes;
	}
	if (m_pShader)
	{
		m_pShader->ReleaseShaderVariables();
		m_pShader->Release();
	}
}

void CGameObject::SetShader(CShader* pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}
void CGameObject::SetMesh(int nIndex, CMesh* pMesh)
{
	if (m_ppMeshes)
	{
		if (m_ppMeshes[nIndex]) m_ppMeshes[nIndex]->Release();
		m_ppMeshes[nIndex] = pMesh;
		if (pMesh) pMesh->AddRef();
	}
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->ReleaseUploadBuffers();
		}
	}
}

void CGameObject::OnPrepareRender()
{
}

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList)
{
}
void CGameObject::ReleaseShaderVariables()
{
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	//��ü�� ���� ��ȯ ����� ��Ʈ ���(32-��Ʈ ��)�� ���Ͽ� ���̴� ����(��� ����)�� �����Ѵ�. 
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &xmf4x4World, 0);
}


void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();
	UpdateShaderVariables(pd3dCommandList);
	if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera);
	//���� ��ü�� �����ϴ� ��� �޽��� �������Ѵ�. 
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Render(pd3dCommandList);
		}
	}
}

void CGameObject::Animate(float fTimeElapsed)
{
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}
void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

//���� ��ü�� ���� z-�� ���͸� ��ȯ�Ѵ�. 
XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32,
		m_xmf4x4World._33)));
}

//���� ��ü�� ���� y-�� ���͸� ��ȯ�Ѵ�. 
XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22,
		m_xmf4x4World._23)));
}

//���� ��ü�� ���� x-�� ���͸� ��ȯ�Ѵ�. 
XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12,
		m_xmf4x4World._13)));
}

//���� ��ü�� ���� x-�� �������� �̵��Ѵ�. 
void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

//���� ��ü�� ���� y-�� �������� �̵��Ѵ�. 
void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

//���� ��ü�� ���� z-�� �������� �̵��Ѵ�. 
void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

//���� ��ü�� �־��� ������ ȸ���Ѵ�. 
void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch),
		XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}


CRotatingObject::CRotatingObject(int nMeshes) : CGameObject(nMeshes)
{
	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fRotationSpeed = 15.0f;
}

CRotatingObject::~CRotatingObject()
{
}

void CRotatingObject::Animate(float fTimeElapsed)
{
	CGameObject::Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);
}

CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int
	nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4
	xmf4Color) : CGameObject(0)
{
	//������ ����� ���� ���� ����, ������ ũ���̴�. 
	m_nWidth = nWidth;
	m_nLength = nLength;
	/*���� ��ü�� ���� �޽����� �迭�� ���� ���̴�. nBlockWidth, nBlockLength�� ���� �޽� �ϳ��� ����, 
	���� ũ���̴�. cxQuadsPerBlock, czQuadsPerBlock�� ���� �޽��� ���� ����� ���� ���� �簢���� �����̴�.*/
	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;
	//xmf3Scale�� ������ ������ �� �� Ȯ���� ���ΰ��� ��Ÿ����. 
	m_xmf3Scale = xmf3Scale;
	//������ ����� ���� ���� �����Ѵ�. 
	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);
	//�������� ���� ����, ���� �������� ���� �޽��� �� ���� �ִ� ���� ��Ÿ����. 
	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;
	//���� ��ü�� ǥ���ϱ� ���� ���� �޽��� �����̴�. 
	m_nMeshes = cxBlocks * czBlocks;
	//���� ��ü�� ǥ���ϱ� ���� ���� �޽��� ���� ������ �迭�� �����Ѵ�. 
	m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)m_ppMeshes[i] = NULL;
	CHeightMapGridMesh* pHeightMapGridMesh = NULL;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			//������ �Ϻκ��� ��Ÿ���� ���� �޽��� ���� ��ġ(��ǥ)�̴�. 
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			//������ �Ϻκ��� ��Ÿ���� ���� �޽��� �����Ͽ� ���� �޽��� �����Ѵ�. 
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, xStart,
			zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
			SetMesh(x + (z * cxBlocks), pHeightMapGridMesh);
		}
	}
	//������ �������ϱ� ���� ���̴��� �����Ѵ�. 
	CTerrainShader *pShader = new CTerrainShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pShader);
}
CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}