#include "Collision.h"

CCollision::CCollision(CScene* pScene, CWallShader* pWall, CPlayer* pPlayer)
{
	m_pScene = pScene;
	m_pWall = pWall;
	m_pPlayer = pPlayer;
}
CCollision::~CCollision()
{
}

void CCollision::check_PlayerByWall()
{
	CGameObject* tmp_Wall = m_pWall->GetGameObject(0);

	BoundingBox tmp(tmp_Wall->GetPosition(), XMFLOAT3(0.1f, 100.0f, 1000.0f));

	tmp.Center.x = -100.0f;

	switch (tmp.Contains(m_pPlayer->m_xmOBB))
	{
	case INTERSECTS:
		m_pPlayer->SetVelocity(XMFLOAT3(100.0f, 0.0f, 0.0f));
		break;
	}

	tmp.Center.x = 100.0f;
	switch (tmp.Contains(m_pPlayer->m_xmOBB))
	{
	case INTERSECTS:
		m_pPlayer->SetVelocity(XMFLOAT3(-100.0f, 0.0f, 0.0f));
		break;
	}

	auto wall_pos = tmp_Wall->GetPosition().z;

	tmp_Wall->SetPosition(tmp_Wall->GetPosition().x, tmp_Wall->GetPosition().y, m_pPlayer->GetPosition().z);
	tmp_Wall->UpdateBounding();
}

void CCollision::check_ObjOverPlayer()
{
	CObjectsShader* tmp_shader = m_pScene->GetShaders(0);

	for (int i = 0; i < tmp_shader->GetObjNum(); ++i)
	{
		CGameObject* obj = tmp_shader->GetGameObject(i);
		if (obj->GetPosition().z < m_pPlayer->GetPosition().z-50.0f)
			obj->SetPosition(obj->GetPosition().x, obj->GetPosition().y, obj->GetPosition().z + 500);
	}
}

bool CCollision::check_PlayerbyObj()
{
	CObjectsShader* tmp_shader = m_pScene->GetShaders(0);

	for (int i = 0; i < tmp_shader->GetObjNum(); ++i)
	{
		CGameObject* obj = tmp_shader->GetGameObject(i);
		if (m_pPlayer->m_xmOBB.Intersects(obj->m_xmOBB))
		{
			obj->SetPosition(obj->GetPosition().x, obj->GetPosition().y, obj->GetPosition().z + 500);
			return true;
		}
	}

	return false;
}