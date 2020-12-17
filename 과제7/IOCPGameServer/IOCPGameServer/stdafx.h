#pragma once
// c++
#include <iostream>
#include <chrono>
#include <stdio.h>  

// server
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <windows.h>  

// stl
#include <vector>
#include <queue>
#include <unordered_set>

// lua
#include "include/lua.hpp"
#include "include/luaconf.h"


// DB
#include <sqlext.h>  

// namespcae
using namespace std;
using namespace chrono;

// lib
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "lua54.lib")
#pragma comment (lib, "odbc32.lib")




// user
#include "protocol.h"
#include "enum.h"
#include "struct.h"
//#include "Global.h"