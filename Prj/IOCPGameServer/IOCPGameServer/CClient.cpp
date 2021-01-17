#include "stdafx.h"
#include "CClient.h"

CClient g_clients[MAX_USER + NUM_NPC + MAX_MONSTER];

void CClient::LevelUP()
{
   	m_slevel++;
	
	m_iexp = 0;
	m_iMax_exp *= 2;
	m_shp += 30;

	m_sAttack_Damage += 4;

	m_bisLevelUp = true;
}


void CClient::Attack(int monsterID)
{
	if (g_clients[monsterID].m_shp > 0) {
		if (monsterID < MAX_USER)
		{
			/*cout << "공격당함" << endl;
			cout << "내 체력 "<<g_clients[monsterID].m_shp << endl<<endl;*/
		}
		else
		{
			{
				/*cout << "공격함" << endl;
				cout << "상대 체력 " << g_clients[monsterID].m_shp << endl<<endl;*/
			}
		}

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
		//cout << "앙 죽었띠" << endl;
		m_iexp += g_clients[monsterID].m_monster_exp;
		g_clients[monsterID].x = 1000;
		g_clients[monsterID].y = 1000;
		g_clients[monsterID].m_status = ST_SLEEP;
		 

		if (m_iexp >= m_iMax_exp)
		{
			LevelUP();
		}

		//g_clients[monsterID].m_status = ST_ALLOC;
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



