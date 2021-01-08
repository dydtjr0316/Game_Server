#include "stdafx.h"
#include "CClient.h"
#include "Global.h"
#include "CDataBase.h"


__m128i add;
__m128i add2;
__m128i mul;
__m128i x;
__m128i y;

__m128i resultadd;
__m128i resultx;
__m128i resulty;


__declspec(align(16)) int pointX[2];
__declspec(align(16)) int pointY[2];
__declspec(align(16)) int result[2];
__declspec(align(16)) int finalresult;
CDataBase g_DataBase;

void show_error() {
    printf("error\n");
}
bool CAS(int* addr, int exp, int update)        // cas 
{
    return atomic_compare_exchange_strong(reinterpret_cast<atomic_int*>(addr), &exp, update);
}
void error_display(const char* msg, int err_no)
{
    WCHAR* lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, err_no,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    std::cout << msg;
    std::wcout << L"에러 " << lpMsgBuf << std::endl;
    while (true);
    LocalFree(lpMsgBuf);
}




void add_timer(int obj_id, int ev_type, system_clock::time_point t)
{
    event_type ev{ obj_id, t, ev_type };

    timer_lock.lock();
    timer_queue.push(ev);
    timer_lock.unlock();
}

void random_move_npc(int id);

void timer_worker()
{
    while (true) {
        while (true) {
            if (!timer_queue.empty())
            {
                event_type ev = timer_queue.top();
                if (ev.wakeup_time > system_clock::now())break;
                timer_lock.lock();
                timer_queue.pop();
                timer_lock.unlock();

                if (ev.event_id == OP_RANDOM)
                {
                    EXOVER* over = new EXOVER();
                    over->op = OP_RANDOM;
                    PostQueuedCompletionStatus(g_iocp, 1, ev.obj_id, &over->over);
                }
                if (ev.event_id == OP_RANDOM_MONSTER)
                {
                    EXOVER* over = new EXOVER();
                    over->op = OP_RANDOM_MONSTER;
                    PostQueuedCompletionStatus(g_iocp, 1, ev.obj_id, &over->over);
                }
            }
            else break;
        }
        this_thread::sleep_for(1ms);
    }
}

void wake_up_npc(int id)
{
    // 공부
    int exp = ST_SLEEP;
    //if(true == atomic_compare_exchange_strong((atomic_int*)(&g_clients[id].m_status), &exp, (int)ST_ACTIVE))
    if (CAS((int*)(&g_clients[id].m_status), exp, ST_ACTIVE))
    {
        add_timer(id, OP_RANDOM, system_clock::now() + 1s);
    }
}

void wake_up_monster(int id) {
    // 공부
    int exp = ST_SLEEP;
    if (CAS((int*)(&g_clients[id].m_status), exp, ST_ACTIVE))
    {
        if(id > MAX_USER + NUM_NPC + DIVIDE_MONNSTER * 2
            && id<= MAX_USER + NUM_NPC + MAX_MONSTER)
        add_timer(id, OP_RANDOM_MONSTER, system_clock::now() + 1s);
    }
}

void Insert_Sector(int user_id)
{
    g_clients[user_id].m_iCSector.x = g_clients[user_id].x / (WORLD_WIDTH / SECTOR_ROW);
    g_clients[user_id].m_iCSector.y = g_clients[user_id].y / (WORLD_WIDTH / SECTOR_ROW);

    sector[g_clients[user_id].m_iCSector.x][g_clients[user_id].m_iCSector.y].insert(user_id);
}

void Change_Sector(int user_id)
{
    CURRENT_SECTOR oldSector;
    oldSector.x = g_clients[user_id].m_iCSector.x;
    oldSector.y = g_clients[user_id].m_iCSector.y;

    g_clients[user_id].m_iCSector.x = g_clients[user_id].x / (WORLD_WIDTH / SECTOR_ROW);
    g_clients[user_id].m_iCSector.y = g_clients[user_id].y / (WORLD_WIDTH / SECTOR_ROW);


    if (oldSector.x != g_clients[user_id].m_iCSector.x || oldSector.y != g_clients[user_id].m_iCSector.y)
    {
        sector[g_clients[user_id].m_iCSector.x][g_clients[user_id].m_iCSector.y].insert(user_id);
        // 
        sector[oldSector.x][oldSector.y].erase(user_id);
    }

}

bool is_npc(int p1)
{
    return (p1 >= MAX_USER && p1 < MAX_USER + NUM_NPC);
}

bool is_Monster(int p1)
{
    return p1 >= MAX_USER + NUM_NPC;
}

bool is_near_Monster(int p1, int p2)
{
    int dist = (g_clients[p1].x - g_clients[p2].x) * (g_clients[p1].x - g_clients[p2].x);
    dist += (g_clients[p1].y - g_clients[p2].y) * (g_clients[p1].y - g_clients[p2].y);
    return dist <= (1);
}
bool is_near(int p1, int p2)
{
    // 이 함수는 굉장히 자주 불리는 함수이다. -> sqrtf(), pawf() 이런 것들 자제하자!
    int dist = (g_clients[p1].x - g_clients[p2].x) * (g_clients[p1].x - g_clients[p2].x);
    dist += (g_clients[p1].y - g_clients[p2].y) * (g_clients[p1].y - g_clients[p2].y);

    //cout << "거리 -> " << dist << endl;


    return dist <= (VIEW_LIMIT * VIEW_LIMIT);
}

//bool is_near(int p1, int p2)
//{
//    pointX[0] = { g_clients[p1].x };
//    pointX[1] = { g_clients[p1].y };
//    
//    pointY[0] = { -g_clients[p2].x };
//    pointY[1] = { -g_clients[p2].y };
//
//    // 값만 저장
//    x = _mm_loadu_si128((__m128i*)pointX);
//    y = _mm_loadu_si128((__m128i*)pointY);
//
//    add = _mm_add_epi16(x, y);
//
//    mul = _mm_mullo_epi16(add, add);
//
//    //add2 = _mm_add_epi16(mul, mul);
//
//  /*  _mm_storeu_si128((__m128i*) result, mul);
//
//    resultx = _mm_loadu_si128((__m128i*)result[0]);
//    resulty = _mm_loadu_si128((__m128i*)result[1]);
//
//    resultadd = _mm_add_epi16(resultx, resulty);*/
//
//    _mm_storeu_si128((__m128i*) finalresult, resultadd);
//
//
//    return finalresult <= (VIEW_LIMIT * VIEW_LIMIT);
//}

void send_packet(int user_id, void* p)
{
    unsigned char* buf = reinterpret_cast<unsigned char*>(p);

    CClient& u = g_clients[user_id];

    EXOVER* exover = new EXOVER;
    ZeroMemory(&exover->io_buf, sizeof(exover->io_buf));
    memcpy(exover->io_buf, buf, buf[0]);

    exover->op = OP_SEND;
    exover->wsabuf.buf = reinterpret_cast<char*>(exover->io_buf);
    exover->wsabuf.len = buf[0];
    ZeroMemory(&exover->over, sizeof(exover->over));
    u.m_cl.lock();
    WSASend(u.m_s, &exover->wsabuf, 1, NULL, 0,
        &exover->over, NULL);
    u.m_cl.unlock();
}

void send_chat_packet(int to_client, int id, char* mess, size_t size)
{
    sc_packet_chat p;
    p.id = id;
    p.size = sizeof(sc_packet_chat) * 2 + 2;
    p.type = SC_PACKET_CHAT;
    strcpy_s(p.message, mess);

    send_packet(to_client, &p);
}

void send_level_up_packet(int user_id)
{
    sc_packet_level_up p;
    p.id = user_id;
    p.size = sizeof(sc_packet_level_up);
    p.type = SC_PACKET_LEVEL_UP;

    p.hp = g_clients[user_id].m_shp;
    p.attack_damage = g_clients[user_id].m_sAttack_Damage;
    p.exp = g_clients[user_id].m_iexp;
    p.level = g_clients[user_id].m_slevel;
    p.max_exp = g_clients[user_id].m_iMax_exp;

    send_packet(user_id, &p);
}

void send_attack_packet(int user_id)
{
    sc_packet_attack p;
    p.id = user_id;
    p.size = sizeof(sc_packet_attack);
    p.type = SC_PACKET_ATTACK;

    p.hp = g_clients[user_id].m_shp;

    send_packet(user_id, &p);
}

void send_login_ok_packet(int user_id)
{
    sc_packet_login_ok p;
    p.id = user_id;
    p.size = sizeof(sc_packet_login_ok);
    p.type = SC_PACKET_LOGIN_OK;

    p.exp = g_clients[user_id].m_iexp;
    p.hp = g_clients[user_id].m_shp;
    p.level = g_clients[user_id].m_slevel;
    p.x = g_clients[user_id].x;
    p.y = g_clients[user_id].y;

    p.iMax_exp = g_clients[user_id].m_iMax_exp;
    p.sHp_Regen = g_clients[user_id].m_sHp_Regen;
    p.Attack_Damage = g_clients[user_id].m_sAttack_Damage;

    send_packet(user_id, &p);
}

void send_enter_packet(int user_id, int o_id)
{
    sc_packet_enter p;
    p.id = o_id;
    p.size = sizeof(p);
    p.type = SC_PACKET_ENTER;
    p.x = g_clients[o_id].x;
    p.y = g_clients[o_id].y;

    strcpy_s(p.name, g_clients[o_id].m_name);   // data race???
    p.o_type = O_PLAYER;

    send_packet(user_id, &p);
}

void send_leave_packet(int user_id, int o_id)
{
    sc_packet_leave p;
    p.id = o_id;
    p.size = sizeof(p);
    p.type = SC_PACKET_LEAVE;

    send_packet(user_id, &p);
}

void send_move_packet(int user_id, int mover)
{
    sc_packet_move p;
    p.id = mover;
    p.size = sizeof(p);
    p.type = SC_PACKET_MOVE;
    p.x = g_clients[mover].x;
    p.y = g_clients[mover].y;

    p.move_time = g_clients[mover].m_iclinet_time;

    send_packet(user_id, &p);
}

void random_move_npc(int id)
{
    unordered_set <int> old_viewList;
    for (int i = 0; i < MAX_USER; ++i) {
        if (ST_ACTIVE != g_clients[i].m_status) continue;
        if (true == is_near(id, i)) old_viewList.insert(i);
    }

    int x = g_clients[id].x;
    int y = g_clients[id].y;

    switch (rand() % 4)
    {
    case 0: if (x > 0)x--; break;
    case 1: if (x < WORLD_WIDTH - 1)x++; break;
    case 2: if (y > 0)y--; break;
    case 3: if (y < WORLD_HEIGHT - 1)y++; break;
    }

    g_clients[id].x = x;
    g_clients[id].y = y;

    unordered_set<int> new_viewList;
    for (int i = 0; i < MAX_USER; ++i)
    {
        if (id == i) continue;
        if (g_clients[i].m_status != ST_ACTIVE) continue;
        /*if (true == is_near(id, i))
        {
        }*/

        if (true == is_near(i, id))
        {
            new_viewList.insert(i);
            g_clients[i].m_cl.lock();
            if (0 != g_clients[i].view_list.count(id))
            {
                g_clients[i].m_cl.unlock();
                send_move_packet(i, id);
            }
            else {
                g_clients[i].m_cl.unlock();
                send_enter_packet(i, id);
            }
        }
        else {
            g_clients[i].m_cl.lock();
            if (0 != g_clients[i].view_list.count(id))
            {
                g_clients[i].m_cl.unlock();
                send_leave_packet(i, id);
            }
            else g_clients[i].m_cl.unlock();
        }

    }


    //for (auto pc : new_viewList) {
    //    EXOVER* over_ex = new EXOVER;
    //    over_ex->id = pc;
    //    over_ex->op = OP_NOTIFY;
    //    PostQueuedCompletionStatus(g_iocp, 1, id, &over_ex->over);
    //}
}

void random_move_monster(int id)
{
    unordered_set <int> old_viewList;
    for (int i = 0; i < MAX_USER; ++i) {
        if (ST_ACTIVE != g_clients[i].m_status) continue;
        if (true == is_near(id, i)) old_viewList.insert(i);
    }

    int x = g_clients[id].x;
    int y = g_clients[id].y;

    switch (rand() % 4)
    {
    case 0: if (x > 0)x--; break;
    case 1: if (x < WORLD_WIDTH - 1)x++; break;
    case 2: if (y > 0)y--; break;
    case 3: if (y < WORLD_HEIGHT - 1)y++; break;
    }

    g_clients[id].x = x;
    g_clients[id].y = y;

    unordered_set<int> new_viewList;
    for (int i = 0; i < MAX_USER; ++i)
    {
        if (id == i) continue;
        if (g_clients[i].m_status != ST_ACTIVE) continue;
       /* if (true == is_near(id, i))
        {
        }*/

        if (true == is_near(i, id))
        {
            new_viewList.insert(i);
            g_clients[i].m_cl.lock();
            if (0 != g_clients[i].view_list.count(id))
            {
                g_clients[i].m_cl.unlock();
                send_move_packet(i, id);
            }
            else {
                g_clients[i].m_cl.unlock();
                send_enter_packet(i, id);
            }
        }
        else {
            g_clients[i].m_cl.lock();
            if (0 != g_clients[i].view_list.count(id))
            {
                g_clients[i].m_cl.unlock();
                send_leave_packet(i, id);
            }
            else g_clients[i].m_cl.unlock();
        }

        if (g_clients[id].m_isFighter && is_near_Monster(i, id))
        {
            g_clients[id].Attack(i);
            send_attack_packet(i);
        }
    }



    //for (auto pc : new_viewList) {
    //    EXOVER* over_ex = new EXOVER;
    //    over_ex->id = pc;
    //    over_ex->op = OP_NOTIFY_MONSTER;
    //    PostQueuedCompletionStatus(g_iocp, 1, id, &over_ex->over);
    //}
}

void do_attack(int user_id)
{
    // 근처 한칸에 있는 몬스터인가?
    for (int i = NUM_NPC + MAX_USER; i < NUM_NPC + MAX_USER + MAX_MONSTER;++i)
    {
        if (is_near_Monster(user_id, i))
        {
            g_clients[user_id].Attack(i);
            if (g_clients[user_id].m_bisLevelUp)
            {
                // 레벨업 패킷
                if (g_clients[i].m_isFighter == false)g_clients[i].m_isFighter = true;
                send_level_up_packet(user_id);
                g_clients[user_id].m_bisLevelUp = false;
            }
        }
    }
}

void do_move(int user_id, int direction)
{
    CClient& u = g_clients[user_id];

    int x = u.x;
    int y = u.y;

    switch (direction) {
    case MV_UP: if (y > 0) y--; break;
    case MV_DOWN: if (y < (WORLD_HEIGHT - 1)) y++; break;
    case MV_LEFT: if (x > 0) x--; break;
    case MV_RIGHT: if (x < (WORLD_WIDTH - 1)) x++; break;
    default:
        cout << "Unknown Direction from Client move packet!\n";
        DebugBreak();
        exit(-1);
    }

    // 시야 처리 구현중
    // 클라이언트가 원래 갖고 있던 이동하기 전 View List = old_ViewList
    //g_clients[user_id].m_cl.lock();
    unordered_set<int> old_viewList = g_clients[user_id].view_list;
    //g_clients[user_id].m_cl.unlock();

    u.x = x;
    u.y = y;

    Change_Sector(user_id);

    send_move_packet(user_id, user_id);

    //시야 처리 구현중
    unordered_set<int> new_viewList;

    vector<unordered_set<int>> vSectors;

    int x1;
    int x2;
    int y1;
    int y2;

    if (u.x > VIEW_LIMIT)
        x1 = (u.x - VIEW_LIMIT) / (WORLD_WIDTH / SECTOR_ROW);
    else
        x1 = u.x / (WORLD_WIDTH / SECTOR_ROW);

    if (u.x < WORLD_WIDTH - VIEW_LIMIT)
        x2 = (u.x + VIEW_LIMIT) / (WORLD_WIDTH / SECTOR_ROW);
    else
        x2 = u.x / (WORLD_WIDTH / SECTOR_ROW);

    if (u.y > VIEW_LIMIT)
        y1 = (u.y - VIEW_LIMIT) / (WORLD_HEIGHT / SECTOR_COL);
    else
        y1 = u.y / (WORLD_HEIGHT / SECTOR_COL);

    if (u.y < WORLD_HEIGHT - VIEW_LIMIT)
        y2 = (u.y + VIEW_LIMIT) / (WORLD_HEIGHT / SECTOR_COL);
    else
        y2 = u.y / (WORLD_HEIGHT / SECTOR_COL);


    if (x1 == x2 && y1 == y2)
    {
        vSectors.push_back(sector[x1][y1]);
    }
    else if (x1 == x2)
    {
        vSectors.push_back(sector[x1][y1]);
        vSectors.push_back(sector[x1][y2]);
    }
    else if (y1 == y2)
    {
        vSectors.push_back(sector[x1][y1]);
        vSectors.push_back(sector[x2][y1]);
    }
    else
    {
        vSectors.push_back(sector[x1][y2]);
        vSectors.push_back(sector[x2][y2]);
        vSectors.push_back(sector[x1][y1]);
        vSectors.push_back(sector[x2][y1]);
    }

    for (auto& vSec : vSectors)
    {
        if (vSec.size() != 0)
        {
            for (auto& i : vSec)
            {
                if (user_id == i) continue;
                if (ST_SLEEP == g_clients[i].m_status)
                {
                    wake_up_npc(g_clients[i].m_id);
                    wake_up_monster(g_clients[i].m_id);
                }
                if (g_clients[i].m_status != ST_ACTIVE) continue;

                if (true == is_near(user_id, i))
                {
                    new_viewList.insert(i);
                }
            }
        }
    }

    for (int i = MAX_USER; i < MAX_USER + NUM_NPC; ++i)
    {
        if (true == is_near(user_id, i))
        {
            new_viewList.insert(i);
            wake_up_npc(i);
        }
    }
    for (int i = MAX_USER + NUM_NPC; i < MAX_USER + NUM_NPC + DIVIDE_MONNSTER * 4; ++i)
    {
        if (true == is_near(user_id, i))
        {
            new_viewList.insert(i);
            wake_up_monster(i);
        }
    }

    //시야에 들어온 객체 처리
    for (int ob : new_viewList)
    {
        //시야에 새로 들어온 객체 구분
        if (0 == old_viewList.count(ob))
        {
            g_clients[user_id].view_list.insert(ob);
            send_enter_packet(user_id, ob);

            if (false == is_npc(ob)) // npc라면 검사안해도되는 부분
            {
                //상대방 viewlist에 내가 없으면
                if (0 == g_clients[ob].view_list.count(user_id))
                {
                    //상대방에 view_list에도 내가 있어야 함.
                    g_clients[ob].view_list.insert(user_id);
                    send_enter_packet(ob, user_id);
                }
                else
                {
                    // 있다면 이동 사실만 보내자
                    send_move_packet(ob, user_id);
                }
            }
        }
        //이전에도 시야에 있었고 이동 후에도 시야에 있는 객체
        else
        {
            //// 나한테 보낼 필요는 없음 (내가 이동한 것이기 때문임)
            // 내가 이동을 한 것이기 때문에 다른 사람에게는 알려야 한다.
            // 단, 멀티쓰레드로 돌리는 것이기 때문에 상대방이 그 사이에 시야에서 사라져버릴 수도 있음.


            if (false == is_npc(ob)) // npc라면 검사안해도되는 부분
            {
                if (0 != g_clients[ob].view_list.count(user_id))
                {
                    send_move_packet(ob, user_id);
                }
                else
                {
                    g_clients[ob].view_list.insert(user_id);
                    send_enter_packet(ob, user_id);
                }
            }
        }
    }
    //전에는 있었는데 지금은 없는 객체 처리
    for (int ob : old_viewList)
    {
        
            if (0 == new_viewList.count(ob))
            {
                g_clients[user_id].view_list.erase(ob);
                send_leave_packet(user_id, ob);

                if (false == is_npc(ob)) // npc라면 검사안해도되는 부분
                {
                    if (0 != g_clients[ob].view_list.count(user_id))
                    {
                        g_clients[ob].view_list.erase(user_id);
                        send_leave_packet(ob, user_id);
                    }
                }
            }
        
    }

    //if (false == is_npc(user_id))
    //{
    //    for (auto& npc : new_viewList)
    //    {
    //        if (false == is_npc(npc)) continue;
    //      
    //            EXOVER* ex_over = new EXOVER;
    //            ex_over->id = user_id;
    //            ex_over->op = OP_NOTIFY;
    //            PostQueuedCompletionStatus(g_iocp, 1, npc, &ex_over->over);
    //        
    //    }
    //}
}

void enter_game(int user_id, char name[])
{
    g_clients[user_id].m_cl.lock();
    strcpy_s(g_clients[user_id].m_name, name);
    g_clients[user_id].m_cl.unlock();

    g_clients[user_id].m_name[MAX_ID_LEN] = NULL;
    //g_DataBase.Import_DB(g_clients[user_id]);
    send_login_ok_packet(user_id);

    //sector 수정
    Insert_Sector(user_id);
    //
    unordered_set<int> new_viewList;

    vector<unordered_set<int>> vSectors;
    int x1;
    int x2;
    int y1;
    int y2;

    if (g_clients[user_id].x > VIEW_LIMIT)
        x1 = (g_clients[user_id].x - VIEW_LIMIT) / (WORLD_WIDTH / SECTOR_ROW);
    else
        x1 = g_clients[user_id].x / (WORLD_WIDTH / SECTOR_ROW);

    if (g_clients[user_id].x < WORLD_WIDTH - VIEW_LIMIT)
        x2 = (g_clients[user_id].x + VIEW_LIMIT) / (WORLD_WIDTH / SECTOR_ROW);
    else
        x2 = g_clients[user_id].x / (WORLD_WIDTH / SECTOR_ROW);

    if (g_clients[user_id].y > VIEW_LIMIT)
        y1 = (g_clients[user_id].y - VIEW_LIMIT) / (WORLD_HEIGHT / SECTOR_COL);
    else
        y1 = g_clients[user_id].y / (WORLD_HEIGHT / SECTOR_COL);

    if (g_clients[user_id].y < WORLD_HEIGHT - VIEW_LIMIT)
        y2 = (g_clients[user_id].y + VIEW_LIMIT) / (WORLD_HEIGHT / SECTOR_COL);
    else
        y2 = g_clients[user_id].y / (WORLD_HEIGHT / SECTOR_COL);

    if (x1 == x2 && y1 == y2)
    {
        vSectors.push_back(sector[x1][y1]);
    }
    else if (x1 == x2)
    {
        vSectors.push_back(sector[x1][y1]);
        vSectors.push_back(sector[x1][y2]);
    }
    else if (y1 == y2)
    {
        vSectors.push_back(sector[x1][y1]);
        vSectors.push_back(sector[x2][y1]);
    }
    else
    {
        vSectors.push_back(sector[x1][y2]);
        vSectors.push_back(sector[x2][y2]);
        vSectors.push_back(sector[x1][y1]);
        vSectors.push_back(sector[x2][y1]);
    }

    for (auto& vSec : vSectors)
    {
        if (vSec.size() != 0)
        {
            for (auto& i : vSec)
            {
                if (user_id == i) continue;
                if (g_clients[i].m_status != ST_ACTIVE) continue;

                if (true == is_near(user_id, i))
                {
                    if (ST_SLEEP == g_clients[i].m_status)
                    {
                        wake_up_npc(i);
                        wake_up_monster(i);
                    }

                    if (!is_npc(i))
                    {
                        //new_viewList.insert(i);
                        if (0 == g_clients[i].view_list.count(user_id))
                        {
                            g_clients[i].view_list.insert(user_id);
                            send_enter_packet(i, user_id);
                        }
                    }

                    if (0 == g_clients[user_id].view_list.count(i))
                    {
                        g_clients[user_id].view_list.insert(i);
                        send_enter_packet(user_id, i);
                    }

                }
            }
        }
    }
    g_clients[user_id].m_status = ST_ACTIVE;
}

void process_packet(int user_id, char* buf)
{
    switch (buf[1]) {
    case CS_LOGIN: {
        cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(buf);
        enter_game(user_id, packet->name);
    }
                 break;
    case CS_MOVE: {
        cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(buf);
        g_clients[user_id].m_iclinet_time = packet->move_time;
        do_move(user_id, packet->direction);

    }
                break;
    case CS_ATTACK:
    {
        cs_packet_attack* packet = reinterpret_cast<cs_packet_attack*>(buf);
        do_attack(user_id);
    }
    break;
    default:
        cout << "Unknown Packet Type Error!\n";
        DebugBreak();
        exit(-1);
    }


}

void initialize_clients()
{
    for (int i = 0; i < MAX_USER; ++i) {
        g_clients[i].m_id = i;
        g_clients[i].m_status = ST_FREE;
    }
}

void disconnect(int user_id)
{
    g_clients[user_id].m_status = ST_ALLOC;

    send_leave_packet(user_id, user_id);

    g_clients[user_id].m_cl.lock();
    for (int i = 0; i < MAX_USER; ++i)
    {

        if (g_clients[i].m_status == ST_ACTIVE)
        {
            if (i != user_id)
            {
                // 시야처리 용도
                if (0 != g_clients[i].view_list.count(user_id))
                {
                    g_clients[i].view_list.erase(user_id);
                    send_leave_packet(i, user_id);
                }
            }
        }

    }

    g_clients[user_id].m_status = ST_FREE;
    g_clients[user_id].view_list.clear();

    g_clients[user_id].m_cl.unlock();

    closesocket(g_clients[user_id].m_s);
    g_clients[user_id].m_s = 0;

    g_DataBase.UpSert_DB(g_clients[user_id]);
}

void recv_packet_construct(int user_id, int io_byte)
{
    CClient& cu = g_clients[user_id];
    EXOVER& r_o = cu.m_recv_over;

    int rest_byte = io_byte;
    char* p = r_o.io_buf;
    int packet_size = 0;
    if (0 != cu.m_prev_size) packet_size = cu.m_packe_buf[0];
    while (rest_byte > 0) {
        if (0 == packet_size) packet_size = *p;
        if (packet_size <= rest_byte + cu.m_prev_size) {
            memcpy(cu.m_packe_buf + cu.m_prev_size, p, packet_size - cu.m_prev_size);
            p += packet_size - cu.m_prev_size;
            rest_byte -= packet_size - cu.m_prev_size;
            packet_size = 0;
            process_packet(user_id, cu.m_packe_buf);
            cu.m_prev_size = 0;
        }
        else {
            memcpy(cu.m_packe_buf + cu.m_prev_size, p, rest_byte);
            cu.m_prev_size += rest_byte;
            rest_byte = 0;
            p += rest_byte;
        }
    }
}

void worker_thread()
{
    while (true) {
        DWORD io_byte;
        ULONG_PTR key;
        WSAOVERLAPPED* over;
        GetQueuedCompletionStatus(g_iocp, &io_byte, &key, &over, INFINITE);

        EXOVER* exover = reinterpret_cast<EXOVER*>(over);
        int user_id = static_cast<int>(key);
        CClient& cl = g_clients[user_id];

        switch (exover->op) {
        case OP_RECV:
            if (0 == io_byte) disconnect(user_id);
            else {
                recv_packet_construct(user_id, io_byte);
                ZeroMemory(&cl.m_recv_over.over, sizeof(cl.m_recv_over.over));
                DWORD flags = 0;
                WSARecv(cl.m_s, &cl.m_recv_over.wsabuf, 1, NULL, &flags, &cl.m_recv_over.over, NULL);
            }
            break;
        case OP_SEND:
            if (0 == io_byte) disconnect(user_id);
            delete exover;
            break;
        case OP_ACCEPT:
        {
            int user_id = -1;
            int i;
            for (i = 0; i < MAX_USER; ++i) {
                g_clients[i].m_cl.lock();
                if (ST_FREE == g_clients[i].m_status) {
                    g_clients[i].m_status = ST_ALLOC;
                    user_id = i;
                    g_clients[i].m_cl.unlock();
                    break;
                }
                g_clients[i].m_cl.unlock();

            }

            SOCKET c_socket = exover->c_socket;

            if (MAX_USER == i) {
                cout << "Max user limit exceeded.\n";
                closesocket(c_socket);
            }

            if (-1 == user_id)
                closesocket(c_socket);
            else {
                CClient& nc = g_clients[user_id];
                nc.m_prev_size = 0;
                nc.m_recv_over.op = OP_RECV;
                ZeroMemory(&nc.m_recv_over.over, sizeof(nc.m_recv_over.over));
                nc.m_recv_over.wsabuf.buf = nc.m_recv_over.io_buf;
                nc.m_recv_over.wsabuf.len = MAX_BUF_SIZE;
                nc.m_s = c_socket;
                nc.x = rand() % WORLD_WIDTH;
                nc.y = rand() % WORLD_HEIGHT;

                nc.m_iFirstX = nc.x;
                nc.m_iFirstY = nc.y;

                CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), g_iocp, user_id, 0);
                DWORD flags = 0;
                int ret;
                g_clients[i].m_cl.lock();
                if (ST_ALLOC == g_clients[i].m_status) {
                    ret = WSARecv(g_clients[i].m_s, &g_clients[i].m_recv_over.wsabuf, 1, NULL,
                        &flags, &g_clients[i].m_recv_over.over, NULL);
                }
                g_clients[i].m_cl.unlock();
                if (SOCKET_ERROR == ret) {
                    int err_no = WSAGetLastError();
                    if (ERROR_IO_PENDING != err_no)
                        error_display("recv", err_no);
                }
                /*   DWORD flags = 0;
                   WSARecv(c_socket, &nc.m_recv_over.wsabuf, 1, NULL, &flags, &nc.m_recv_over.over, NULL);
                */
            }

            c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
            exover->c_socket = c_socket;
            ZeroMemory(&exover->over, sizeof(exover->over));
            AcceptEx(l_socket, c_socket, exover->io_buf, NULL,
                sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, &exover->over);
        }
        break;
        case OP_RANDOM:
        {
            random_move_npc(user_id); bool keep_alive = false;
            //active인 플레이어가 주변에 있으면 계속 깨워두기
            for (int i = 0; i < MAX_USER; ++i)
                if (true == is_near(user_id, i))
                    if (ST_ACTIVE == g_clients[i].m_status)
                    {
                        keep_alive = true; break;
                    }
            if (true == keep_alive) add_timer(user_id, OP_RANDOM, system_clock::now() + 1s);
            else g_clients[user_id].m_status = ST_SLEEP;
            //주위에 이제 아무도 없으면 SLEEP으로 멈춰두기 
            delete exover;

        }
        break;
        case OP_RANDOM_MONSTER:
        {
            random_move_monster(user_id); bool keep_alive = false;
            //active인 플레이어가 주변에 있으면 계속 깨워두기
            for (int i = 0; i < MAX_USER; ++i)
                if (true == is_near(user_id, i))
                    if (ST_ACTIVE == g_clients[i].m_status)
                    {
                        keep_alive = true; break;
                    }
            if (true == keep_alive) add_timer(user_id, OP_RANDOM_MONSTER, system_clock::now() + 1s);
            else g_clients[user_id].m_status = ST_SLEEP;
            //주위에 이제 아무도 없으면 SLEEP으로 멈춰두기 
            delete exover;
        }
        break;
        case OP_NOTIFY_MONSTER:
            break;
        case OP_NOTIFY:
        {
            //g_clients[key].m_cl.lock();//
            //lua_getglobal(g_clients[key].L, "event_player_move");
            //lua_pushnumber(g_clients[key].L, exover->id);
            //lua_pcall(g_clients[key].L, 1, 1, 0);
           // g_clients[key].m_cl.unlock();//
            delete exover;
        }
        break;
        case OP_NPC:
        {
            //g_clients[key].m_cl.lock();//
            //lua_getglobal(g_clients[key].L, "say_good_bye");
            //lua_pushnumber(g_clients[key].L, exover->id);
            //lua_pcall(g_clients[key].L, 1, 0, 0);
            //g_clients[key].m_cl.unlock();//
            delete exover;
        }
        break;
        }
    }
}

int API_get_x(lua_State* L)
{
    int user_id = lua_tointeger(L, -1);
    lua_pop(L, 2);
    int x = g_clients[user_id].x;
    lua_pushnumber(L, x);
    return 1;
}

int API_get_y(lua_State* L)
{
    int user_id = lua_tointeger(L, -1);
    lua_pop(L, 2);
    int y = g_clients[user_id].y;
    lua_pushnumber(L, y);
    return 1;
}

int API_SendMessage(lua_State* L)
{
    int my_id = (int)lua_tointeger(L, -3);
    int user_id = (int)lua_tointeger(L, -2);
    size_t size = strlen((char*)L);
    char* mess = (char*)lua_tolstring(L, -1, &size);
    lua_pop(L, 3);

    send_chat_packet(user_id, my_id, mess, size);
    //this_thread::sleep_for(1s);
    return 0;
}

int API_RunAway(lua_State* L)
{
    int user_id = (int)lua_tointeger(L, -2);
    int my_id = (int)lua_tointeger(L, -1);
    lua_pop(L, 2);

    lua_getglobal(L, "dist");
    int dist = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);
    if (dist == 0) {
        int dir = rand() % 4;
        lua_getglobal(L, "set_dir");
        lua_pushnumber(L, dir);
        lua_pcall(L, 1, 0, 0);
    }

    //add_timer(my_id, OP_NPC_RUN, system_clock::now() + 1s);
    EXOVER* over = new EXOVER;
    over->id = user_id;
    over->op = OP_NPC;
    PostQueuedCompletionStatus(g_iocp, 1, my_id, &over->over);
    return 0;
}

int API_SetPos(lua_State* L)
{
    int x = (int)lua_tointeger(L, -4);
    int y = (int)lua_tointeger(L, -3);
    int npc = (int)lua_tointeger(L, -2);
    int user = (int)lua_tointeger(L, -1);
    lua_pop(L, 4);

    g_clients[npc].x = x;
    g_clients[npc].y = y;
    send_move_packet(user, npc);
    this_thread::sleep_for(1s);
    return 0;
}

void initialize_Monster()
{
    for (int type = 0; type < 4; ++type)
    {
        for (int i = MAX_USER + NUM_NPC+(DIVIDE_MONNSTER * (type)); i < MAX_USER + NUM_NPC + (DIVIDE_MONNSTER * (type+1)); ++i)
        {
            if (type >= 2)
            {
                g_clients[i].m_isFighter = true;
            }
            g_clients[i].x = rand() % WORLD_WIDTH;
            g_clients[i].y = rand() % WORLD_HEIGHT;
            g_clients[i].m_id = i;
            g_clients[i].m_status = ST_SLEEP;
            g_clients[i].m_Monster_level = type;

            g_clients[i].m_shp += 10*type;

            g_clients[i].m_sAttack_Damage += 2* type;
            g_clients[i].m_monster_exp = g_clients[i].m_Monster_level * g_clients[i].m_Monster_level * 2;
        }
    }
}

void initialize_NPC()
{
    for (int i = MAX_USER; i < MAX_USER + NUM_NPC; ++i)
    {
        g_clients[i].x = rand() % WORLD_WIDTH;
        g_clients[i].y = rand() % WORLD_HEIGHT;

        char npc_name[50];
        sprintf_s(npc_name, "N%d", i);
        strcpy_s(g_clients[i].m_name, npc_name);
        g_clients[i].m_status = ST_SLEEP;

        //lua_State* L = g_clients[i].L = luaL_newstate();
        //luaL_openlibs(L);

        //int error = luaL_loadfile(L, "monster.lua");
        //error = lua_pcall(L, 0, 0, 0);

        //lua_getglobal(L, "set_uid");
        //lua_pushnumber(L, i);
        //lua_pcall(L, 1, 1, 0);
        //// lua_pop(L, 1);// eliminate set_uid from stack after call

        //lua_register(L, "API_SendMessage", API_SendMessage);
        //lua_register(L, "API_get_x", API_get_x);
        //lua_register(L, "API_get_y", API_get_y);
        //lua_register(L, "API_SetPos", API_SetPos);
        //lua_register(L, "API_RunAway", API_RunAway);
    }
}

int main()
{
    // Set the ODBC version environment attribute  


    std::wcout.imbue(locale("Korean"));


    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 2), &WSAData);
    g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
    l_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(l_socket), g_iocp, 999, 0);

    SOCKADDR_IN s_address;
    memset(&s_address, 0, sizeof(s_address));
    s_address.sin_family = AF_INET;
    s_address.sin_port = htons(SERVER_PORT);
    s_address.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    ::bind(l_socket, reinterpret_cast<sockaddr*>(&s_address), sizeof(s_address));
    listen(l_socket, SOMAXCONN);

    SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    EXOVER accept_over;

    accept_over.op = OP_ACCEPT;
    accept_over.c_socket = c_socket;
    accept_over.wsabuf.len = static_cast<int>(c_socket);
    ZeroMemory(&accept_over.over, sizeof(accept_over.over));
    AcceptEx(l_socket, c_socket, accept_over.io_buf, NULL, 32, 32, NULL, &accept_over.over);

    cout << "Initializing" << endl;
    initialize_clients();
    initialize_NPC();
    initialize_Monster();
    cout << "Initializing Finish" << endl;

    thread time_thread{ timer_worker };

    vector <thread> worker_threads;
    for (int i = 0; i < 4; ++i) worker_threads.emplace_back(worker_thread);
    for (auto& th : worker_threads) th.join();
    time_thread.join();

    closesocket(l_socket);
    WSACleanup();
}