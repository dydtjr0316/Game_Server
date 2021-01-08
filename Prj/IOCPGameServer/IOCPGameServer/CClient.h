#pragma once


class CClient
{
public:
	void LevelUP();

	void Attack(int monsterID);
	void Kill_Monster(int monsterID);

	void Player_Die(int monsterID);
	void MinusHP(int damage);

	short GetAttackDamage() { return m_sAttack_Damage; };
	short GetHP() { return m_shp; }

public: // ���� �߰� 1216
	int		m_id;
	short m_shp = 100;
	short m_slevel = 1;
	int   m_iexp = 0;
	bool m_bisLevelUp = false;
	short x, y;
	int	  m_iMax_exp = 100;
	short m_sHp_Regen = (m_shp/10); // ü��
	short m_sAttack_Damage = 5;
	short m_monster_exp = 2;
	bool m_isFighter = false;
	short m_Monster_level =0 ;
	int m_iFirstX;
	int m_iFirstY;

public:
	// server
	SOCKET	m_s;
	mutex	m_cl;
	mutex vl;
	unordered_set<int> view_list;
	// ������ �� �ʿ䰡 ������ unordered�� ������� -> �ξ� ������!
	// int
	int   m_prev_size;
	int m_iclinet_time;
	// short
	
	// char
	char  m_packe_buf[MAX_PACKET_SIZE];
	char m_name[MAX_ID_LEN + 1];
	// struct
	EXOVER  m_recv_over;
	C_STATUS m_status;
	CURRENT_SECTOR m_iCSector;	// sector ����
	// lua
	lua_State* L;



};

