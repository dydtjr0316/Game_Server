#include "stdafx.h"
#include "CMonster.h"

void CMonster::initialize_Monster()
{
	cout << "Initializing Monsters\n";
	for (int i = MAX_USER; i < MAX_USER + NUM_NPC; ++i)
	{
		x = rand() % WORLD_WIDTH;
		y = rand() % WORLD_HEIGHT;

		char npc_name[50];

		m_status = ST_ACTIVE;

	}
	cout << "Monster initialize finished.\n";
}