#pragma once
class CMonster
{
public:
	short GetDamage() { return m_sAttack_Damage; }
	short GetX() { return x; }
	short GetY() { return y; }
	short GetHP() { return m_shp; }

	void MinusHP(short damage) { m_shp -= damage; }


public:
	void initialize_Monster();
	
private:
	short m_shp = 100;
	short m_slevel = 1;
	int   m_iexp = 0;

	int	  m_iMax_exp = 100;
	short m_sHp_Regen = (m_shp / 10); // ü��
	short m_sAttack_Damage = 5;

private:
	// server
	SOCKET	m_s;
	mutex	m_cl;
	mutex vl;
	unordered_set<int> view_list;
	// ������ �� �ʿ䰡 ������ unordered�� ������� -> �ξ� ������!
	// int
	int		m_id;
	int   m_prev_size;
	int m_iclinet_time;
	// short
	short x, y;
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

