#pragma once
#include "Scene.h"
#include "Player.h"

class CCollision
{
private:
	CScene* m_pScene = NULL;
	CWallShader* m_pWall = NULL;
	CPlayer* m_pPlayer = NULL;

public:
	CCollision(CScene* pScene, CWallShader* pWall, CPlayer* pPlayer);
	~CCollision();
	void check_PlayerByWall();
	void check_ObjOverPlayer();
	bool check_PlayerbyObj();
};

