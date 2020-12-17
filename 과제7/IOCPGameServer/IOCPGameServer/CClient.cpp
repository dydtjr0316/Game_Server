#include "stdafx.h"
#include "CClient.h"
#include "CMonster.h"
CMonster g_Monsters[MAX_MONSTER];
CClient g_clients[MAX_USER + NUM_NPC + MAX_MONSTER];

void CClient::LevelUP()
{
   	m_slevel++;
	
	m_iexp = 0;
	m_iMax_exp *= 2;
	m_shp += 10;

	m_sAttack_Damage += 2;

	m_bisLevelUp = true;
}

bool CClient::is_near(int monsterID)
{
	int dist = (x - g_Monsters[monsterID].GetX()) * (x - g_Monsters[monsterID].GetX());
	dist += (y - g_Monsters[monsterID].GetY()) * (y - g_Monsters[monsterID].GetY());

	return dist <= (2 * 2); // ¹üÀ§ Á¶Á¤
}

void CClient::Attack(int monsterID)
{
	if (g_clients[monsterID].m_shp > 0) {
		//cout << "Á¶ ÆÐ »Ô¶ó" << endl;
		//cout << g_clients[monsterID].m_shp << endl;
		g_clients[monsterID].MinusHP(m_sAttack_Damage);

		if (g_clients[monsterID].GetHP() <= 0)
		{
			m_bisLevelUp = true;
			Kill_Monster(monsterID);
		}
	}
}

void CClient::Kill_Monster(int monsterID)
{
	if (monsterID < MAX_USER)
	{
		Player_Die(monsterID);
	}

	else {
		//cout << "¾Ó Á×¾ú¶ì" << endl;
		m_iexp += g_clients[monsterID].m_monster_exp;

		if (m_iexp >= m_iMax_exp)
		{
			LevelUP();
		}

		g_clients[monsterID].m_status = ST_ALLOC;
	}
}

void CClient::Player_Die(int monsterID)
{
	g_clients[monsterID].m_shp = 100 + m_slevel * 10;
	g_clients[monsterID].m_iexp /= 2;
	g_clients[monsterID].x = g_clients[monsterID].m_iFirstX;
	g_clients[monsterID].y = g_clients[monsterID].m_iFirstY;
}

void CClient::MinusHP(int damage)
{
	m_shp -= damage;
}



