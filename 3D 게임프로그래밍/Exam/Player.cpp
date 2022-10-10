#include "Player.h"
#include "Shader.h"

const float XZ_VELOCITY{ 50.0f };
const float Y_VELOCITY{ 80.0f };

CPlayer::CPlayer(int nMeshes) : CGameObject(nMeshes)
{
	m_pCamera = NULL;
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();
	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList)
{
	CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList);
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
void CPlayer::ReleaseShaderVariables()
{
	CGameObject::ReleaseShaderVariables();
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}
void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::UpdateShaderVariables(pd3dCommandList);
}

/*플레이어의 위치를 변경하는 함수이다. 플레이어의 위치는 기본적으로 사용자가 플레이어를 이동하기 위한 키보드를
누를 때 변경된다. 플레이어의 이동 방향(dwDirection)에 따라 플레이어를 fDistance 만큼 이동한다.*/
void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		//화살표 키 ‘↑’를 누르면 로컬 z-축 방향으로 이동(전진)한다. ‘↓’를 누르면 반대 방향으로 이동한다. 
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);

		//화살표 키 ‘→’를 누르면 로컬 x-축 방향으로 이동한다. ‘←’를 누르면 반대 방향으로 이동한다. 
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);

		//‘Page Up’을 누르면 로컬 y-축 방향으로 이동한다. ‘Page Down’을 누르면 반대 방향으로 이동한다. 
		//if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		//if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

		//플레이어를 현재 위치 벡터에서 xmf3Shift 벡터만큼 이동한다. 
		Move(xmf3Shift, bUpdateVelocity);
	}
}
void CPlayer::Move(XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	//bUpdateVelocity가 참이면 플레이어를 이동하지 않고 속도 벡터를 변경한다. 
	if (bUpdateVelocity)
	{
		//플레이어의 속도 벡터를 xmf3Shift 벡터만큼 변경한다. 
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
	//플레이어를 현재 위치 벡터에서 xmf3Shift 벡터만큼 이동한다. 
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
	//플레이어의 위치가 변경되었으므로 카메라의 위치도 xmf3Shift 벡터만큼 이동한다. 
		if (m_pCamera) m_pCamera->Move(xmf3Shift);
	}
}

//플레이어를 로컬 x-축, y-축, z-축을 중심으로 회전한다. 
void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCameraMode = m_pCamera->GetMode();
	//1인칭 카메라 또는 3인칭 카메라의 경우 플레이어의 회전은 약간의 제약이 따른다. 
	if ((nCameraMode == FIRST_PERSON_CAMERA) || (nCameraMode == THIRD_PERSON_CAMERA))
	{
		/*로컬 x-축을 중심으로 회전하는 것은 고개를 앞뒤로 숙이는 동작에 해당한다. 
		그러므로 x-축을 중심으로 회전하는 각도는 -89.0~+89.0도 사이로 제한한다. 
		x는 현재의 m_fPitch에서 실제 회전하는 각도이므로 x만큼 회전한 다음 Pitch가 +89도 보다 크거나 -89도 보다 작으면 
		m_fPitch가 +89도 또는 -89도가 되도록 회전각도(x)를 수정한다.*/
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { 
				x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; 
			}
			if (m_fPitch < -89.0f) {
				x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; 
			}
		}
		if (y != 0.0f)
		{
			//로컬 y-축을 중심으로 회전하는 것은 몸통을 돌리는 것이므로 회전 각도의 제한이 없다. 
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			/*로컬 z-축을 중심으로 회전하는 것은 몸통을 좌우로 기울이는 것이므로 회전 각도는 -20.0~+20.0도 사이로 제한된다. 
			z는 현재의 m_fRoll에서 실제 회전하는 각도이므로 z만큼 회전한 다음 m_fRoll이 +20도 보다 크거나 -20도보다 작으면 
			m_fRoll이 +20도 또는 -20도가 되도록 회전각도(z)를 수정한다.*/
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		//카메라를 x, y, z 만큼 회전한다. 플레이어를 회전하면 카메라가 회전하게 된다. 
		m_pCamera->Rotate(x, y, z);

		/*플레이어를 회전한다. 1인칭 카메라 또는 3인칭 카메라에서 플레이어의 회전은 로컬 y-축에서만 일어난다. 
		플레이어의 로컬 y-축(Up 벡터)을 기준으로 로컬 z-축(Look 벡터)와 로컬 x-축(Right 벡터)을 회전시킨다. 
		기본적으로 Up 벡터를 기준으로 회전하는 것은 플레이어가 똑바로 서있는 것을 가정한다는 의미이다.*/
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up),
				XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

/*회전으로 인해 플레이어의 로컬 x-축, y-축, z-축이 서로 직교하지 않을 수 있으므로 z-축(Look 벡터)을 기준으로하여 서로 직교하고 단위벡터가 되도록 한다.*/
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

//이 함수는 매 프레임마다 호출된다. 플레이어의 속도 벡터에 중력과 마찰력 등을 적용한다. 
void CPlayer::Update(float fTimeElapsed)
{
	/*플레이어의 속도 벡터의 XZ-성분의 크기를 구한다. 이것이 XZ-평면의 최대 속력보다 크면 속도 벡터의 x와 z-방향 성분을 조정한다.*/
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;

	XMFLOAT3 fNormal = Vector3::Normalize(m_xmf3Velocity);
	fNormal.x *= fMaxVelocityXZ * sqrtf(2);
	fNormal.z *= fMaxVelocityXZ * sqrtf(2);
	m_xmf3Velocity.x = fNormal.x;
	m_xmf3Velocity.z = fNormal.z;

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;
	if (pTerrain)
	{
		XMFLOAT3 xmf3PlayerPosition = GetPosition();
		float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z) + 6.0f;

		if (xmf3PlayerPosition.y > fHeight)
		{
			if (m_xmf3Gravity.y == 0.0f)
				SetGravity(XMFLOAT3(0.0f, -Y_VELOCITY, 0.0f));

			/*플레이어의 속도 벡터를 중력 벡터와 더한다. 중력 벡터에 fTimeElapsed를 곱하는 것은 중력을 시간에 비례하도록 적용한다는 의미이다.*/
			m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));
			SetGravity(Vector3::ScalarProduct(m_xmf3Gravity, 1.0009f, false));
		}
		else
		{
			SetGravity(XMFLOAT3(0.0f, -Y_VELOCITY, 0.0f));
			m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));
		}
	}

	/*플레이어의 속도 벡터의 y-성분의 크기를 구한다. 이것이 y-축 방향의 최대 속력보다 크면 속도 벡터의 y-방향 성분을 조정한다.*/
	float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	//플레이어를 속도 벡터 만큼 실제로 이동한다(카메라도 이동될 것이다).
	//XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	XMFLOAT3 xmf3Velocity = m_xmf3Velocity;
	Move(xmf3Velocity, false);

	/*플레이어의 위치가 변경될 때 추가로 수행할 작업을 수행한다. 플레이어의 새로운 위치가 유효한 위치가 아닐 수도
	있고 또는 플레이어의 충돌 검사 등을 수행할 필요가 있다. 이러한 상황에서 플레이어의 위치를 유효한 위치로 다시
	변경할 수 있다.*/
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCameraMode = m_pCamera->GetMode();
	//플레이어의 위치가 변경되었으므로 3인칭 카메라를 갱신한다. 
	if (nCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position,fTimeElapsed);
	//카메라의 위치가 변경될 때 추가로 수행할 작업을 수행한다. 
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	//카메라가 3인칭 카메라이면 카메라가 변경된 플레이어 위치를 바라보도록 한다. 
	if (nCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	//카메라의 카메라 변환 행렬을 다시 생성한다. 
	m_pCamera->RegenerateViewMatrix();

	///*플레이어의 속도 벡터가 마찰력 때문에 감속이 되어야 한다면 감속 벡터를 생성한다. 속도 벡터의 반대 방향 벡터를
	//구하고 단위 벡터로 만든다. 마찰 계수를 시간에 비례하도록 하여 마찰력을 구한다. 단위 벡터에 마찰력을 곱하여 감
	//속 벡터를 구한다. 속도 벡터에 감속 벡터를 더하여 속도 벡터를 줄인다. 마찰력이 속력보다 크면 속력은 0이 될 것이
	//다.*/
	//fLength = Vector3::Length(m_xmf3Velocity);
	//float fDeceleration = (m_fFriction * fTimeElapsed);
	//if (fDeceleration > fLength) fDeceleration = fLength;
	//m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity,-fDeceleration, true));
	m_xmf3Velocity = XMFLOAT3(0, m_xmf3Velocity.y, 0);
}

/*카메라를 변경할 때 ChangeCamera() 함수에서 호출되는 함수이다. nCurrentCameraMode는 현재 카메라의 모드이고 nNewCameraMode는 새로 설정할 카메라 모드이다.*/
CCamera* CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	//새로운 카메라의 모드에 따라 카메라를 새로 생성한다. 
	CCamera *pNewCamera = NULL;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		pNewCamera = new CFirstPersonCamera(m_pCamera);
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = new CThirdPersonCamera(m_pCamera);
		break;
	}
	
	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		//현재 카메라를 사용하는 플레이어 객체를 설정한다. 
		pNewCamera->SetPlayer(this);
	}
	if (m_pCamera) delete m_pCamera;
	return(pNewCamera);
}

/*플레이어의 위치와 회전축으로부터 월드 변환 행렬을 생성하는 함수이다. 
플레이어의 Right 벡터가 월드 변환 행렬의 첫 번째 행 벡터, Up 벡터가 두 번째 행 벡터, 
Look 벡터가 세 번째 행 벡터, 플레이어의 위치 벡터가 네 번째 행 벡터가 된다.*/
void CPlayer::OnPrepareRender()
{
	m_xmf4x4World._11 = m_xmf3Right.x;
	m_xmf4x4World._12 = m_xmf3Right.y;
	m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x;
	m_xmf4x4World._22 = m_xmf3Up.y;
	m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x;
	m_xmf4x4World._32 = m_xmf3Look.y;
	m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x;
	m_xmf4x4World._42 = m_xmf3Position.y;
	m_xmf4x4World._43 = m_xmf3Position.z;
}
void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	//카메라 모드가 3인칭이면 플레이어 객체를 렌더링한다. 
	if (nCameraMode == THIRD_PERSON_CAMERA)
	{
		if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera);
		CGameObject::Render(pd3dCommandList, pCamera);
	}
}

CTerrainPlayer::CTerrainPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, int
	nMeshes) : CPlayer(nMeshes)
{
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	float fHeight = pTerrain->GetHeight(pTerrain->GetWidth() * 0.5f, pTerrain->GetLength() * 0.5f);
	SetPosition(XMFLOAT3(pTerrain->GetWidth() * 0.5f, fHeight, pTerrain->GetLength() * 0.5f));

	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	//플레이어의 위치가 변경될 때 지형의 정보에 따라 플레이어의 위치를 변경할 수 있도록 설정한다. 
	SetPlayerUpdatedContext(pTerrain);
	//카메라의 위치가 변경될 때 지형의 정보에 따라 카메라의 위치를 변경할 수 있도록 설정한다. 
	SetCameraUpdatedContext(pTerrain);

	CCubeMeshDiffused* pCubeMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 4.0f, 12.0f, 4.0f);
	SetMesh(0, pCubeMesh);
	//플레이어를 렌더링할 셰이더를 생성한다. 
	CPlayerShader *pShader = new CPlayerShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pShader);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CTerrainPlayer::~CTerrainPlayer()
{
}

CCamera* CTerrainPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(250.0f);
		//1인칭 카메라일 때 플레이어에 y-축 방향으로 중력이 작용한다. 
		//SetGravity(XMFLOAT3(0.0f, -XYZ_VELOCITY, 0.0f));
		SetMaxVelocityXZ(XZ_VELOCITY);
		SetMaxVelocityY(Y_VELOCITY);

		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 50000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
		break;
	
	case THIRD_PERSON_CAMERA:
		SetFriction(250.0f);
		//3인칭 카메라일 때 플레이어에 y-축 방향으로 중력이 작용한다. 
		//SetGravity(XMFLOAT3(0.0f, -XYZ_VELOCITY, 0.0f));
		SetMaxVelocityXZ(XZ_VELOCITY);
		SetMaxVelocityY(Y_VELOCITY);

		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.25f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 50000.0f, ASPECT_RATIO, 60.0f);

		XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Identity();
		xmf4x4Rotate._11 = m_xmf3Right.x; xmf4x4Rotate._21 = m_xmf3Up.x; xmf4x4Rotate._31 = m_xmf3Look.x;
		xmf4x4Rotate._12 = m_xmf3Right.y; xmf4x4Rotate._22 = m_xmf3Up.y; xmf4x4Rotate._32 = m_xmf3Look.y;
		xmf4x4Rotate._13 = m_xmf3Right.z; xmf4x4Rotate._23 = m_xmf3Up.z; xmf4x4Rotate._33 = m_xmf3Look.z;
		XMFLOAT3 xmf3Offset = Vector3::TransformCoord(XMFLOAT3(0.0f, 20.0f, -40.0f), xmf4x4Rotate);
		XMFLOAT3 xmf3Position = Vector3::Add(m_xmf3Position, xmf3Offset);
		m_pCamera->SetPosition(xmf3Position);
		break;
	default:
		break;
	}
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	Update(fTimeElapsed);
	return(m_pCamera);
}

void CTerrainPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;
	/*지형에서 플레이어의 현재 위치 (x, z)의 지형 높이(y)를 구한다. 그리고 플레이어 메쉬의 높이가 12이고 플레이어의
	중심이 직육면체의 가운데이므로 y 값에 메쉬의 높이의 절반을 더하면 플레이어의 위치가 된다.*/
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z) + 6.0f;
	/*플레이어의 위치 벡터의 y-값이 음수이면(예를 들어, 중력이 적용되는 경우) 플레이어의 위치 벡터의 y-값이 점점
	작아지게 된다. 이때 플레이어의 현재 위치 벡터의 y 값이 지형의 높이(실제로 지형의 높이 + 6)보다 작으면 플레이어
	의 일부가 지형 아래에 있게 된다. 이러한 경우를 방지하려면 플레이어의 속도 벡터의 y 값을 0으로 만들고 플레이어
	의 위치 벡터의 y-값을 지형의 높이(실제로 지형의 높이 + 6)로 설정한다. 그러면 플레이어는 항상 지형 위에 있게 된
	다.*/
	if (xmf3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}
}


void CTerrainPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	/*높이 맵에서 카메라의 현재 위치 (x, z)에 대한 지형의 높이(y 값)를 구한다. 이 값이 카메라의 위치 벡터의 y-값 보
	다 크면 카메라가 지형의 아래에 있게 된다. 이렇게 되면 다음 그림의 왼쪽과 같이 지형이 그려지지 않는 경우가 발생
	한다(카메라가 지형 안에 있으므로 삼각형의 와인딩 순서가 바뀐다). 이러한 경우가 발생하지 않도록 카메라의 위치 벡
	터의 y-값의 최소값은 (지형의 높이 + 5)로 설정한다. 카메라의 위치 벡터의 y-값의 최소값은 지형의 모든 위치에서
	카메라가 지형 아래에 위치하지 않도록 설정해야 한다.*/
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pCameraUpdatedContext;
	float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z) + 5.0f;
	if (xmf3CameraPosition.y <= fHeight)
	{
		xmf3CameraPosition.y = fHeight;
		m_pCamera->SetPosition(xmf3CameraPosition);
		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			//3인칭 카메라의 경우 카메라 위치(y-좌표)가 변경되었으므로 카메라가 플레이어를 바라보도록 한다. 
			CThirdPersonCamera *p3rdPersonCamera = (CThirdPersonCamera *)m_pCamera;
			p3rdPersonCamera->SetLookAt(GetPosition());
		}
	}
}