#pragma once

// enum class 사용 고려
enum ENUMOP { OP_RECV, OP_SEND, OP_ACCEPT, OP_RANDOM, OP_NOTIFY, OP_NPC };

enum C_STATUS { ST_FREE, ST_ALLOC, ST_ACTIVE, ST_SLEEP }; 