#include "stdafx.h"
#include "CClient.h"
#include "CMonster.h"
CMonster g_Monsters[MAX_MONSTER];
void CClient::LevelUP()
{
	m_slevel++;
	
	m_iexp = 0;
	m_iMax_exp *= 2;
	m_shp += 10;

	m_sAttack_Damage += 2;
}

bool CClient::is_near(int monsterID)
{
	int dist = (x - g_Monsters[monsterID].GetX()) * (x - g_Monsters[monsterID].GetX());
	dist += (y - g_Monsters[monsterID].GetY()) * (y - g_Monsters[monsterID].GetY());

	return dist <= (2 * 2); // 범위 조정
}

void CClient::Attack(int monsterID)
{
	if (is_near(monsterID))
	{
		g_Monsters[monsterID].MinusHP(m_sAttack_Damage);
		
		if (g_Monsters[monsterID].GetHP() <= 0)
		{
			Kill_Monster(monsterID);
		}
	}
}

void CClient::Kill_Monster(int monsterID)
{
	m_iexp += 20;

	if (m_iexp >= m_iMax_exp)
	{
		LevelUP();
	}
}

void CClient::Player_Die()
{
	m_shp = 100 + m_slevel * 10;
	m_iexp /= 2;
	x = 67;
	y = 41;
}



