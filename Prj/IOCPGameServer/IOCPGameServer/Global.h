#pragma once

SOCKET l_socket;
HANDLE g_iocp;

extern CClient g_clients[MAX_USER + NUM_NPC + MAX_MONSTER];

// mutex
mutex timer_lock;

// sector ¼öÁ¤
unordered_set<int>				sector[SECTOR_ROW][SECTOR_COL];
priority_queue<event_type>		timer_queue;
