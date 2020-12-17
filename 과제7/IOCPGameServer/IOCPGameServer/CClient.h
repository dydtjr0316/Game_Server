#pragma once


class CClient
{
public:
	void LevelUP();
	bool is_near(int monsterID);

	void Attack(int monsterID);
	void Kill_Monster(int monsterID);

	void Player_Die();
	void MinusHP(int damage);

	short GetAttackDamage() { return m_sAttack_Damage; };
	short GetHP() { return m_shp; }

public: // 새로 추가 1216
	short m_shp = 100;
	short m_slevel = 1;
	int   m_iexp = 0;

	int	  m_iMax_exp = 100;
	short m_sHp_Regen = (m_shp/10); // 체젠
	short m_sAttack_Damage = 5;
	short m_monster_exp = 2;

public:
	// server
	SOCKET	m_s;
	mutex	m_cl;
	mutex vl;
	unordered_set<int> view_list;
	// 소팅이 될 필요가 없으면 unordered로 사용하자 -> 훨씬 빠르다!
	// int
	int		m_id;
	int   m_prev_size;
	int m_iclinet_time;
	short m_Monster_level =0 ;
	// short
	short x, y;
	// char
	char  m_packe_buf[MAX_PACKET_SIZE];
	char m_name[MAX_ID_LEN + 1];
	// struct
	EXOVER  m_recv_over;
	C_STATUS m_status;
	CURRENT_SECTOR m_iCSector;	// sector 수정
	// lua
	lua_State* L;



};

