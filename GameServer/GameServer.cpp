// GameServer.cpp

#include "pch.h"
#include "CorePch.h"
#include "ConcurrentQueue.h"
#include "ConcurrentStack.h"

#include <iostream>
#include <thread>
#include <mutex> 
#include <atomic>
#include <future>
#include <Windows.h>

LockQueue<int32> q;
LockStack<int32> s;

void Push()
{
    while (true)
    {
        int32 value = rand() % 100;
        s.TryPush(value);

        // this_thread::sleep_for(10ms); 극한 상황 연출을 위해 주석처리.
    }
}

void Pop()
{
    while (true)
    {
        int32 data = 0;
        if (true == s.TryPop(data))
        {
            cout << data << endl;
        }

        // q.WaitPop(data); 만약 데이터가 일정하지 않은 간격으로 들어온다면.
        // cout << data << endl; 
    }
}

int main()
{
    thread t1(Push);
    thread t2(Pop);

    t1.join();
    t2.join();

    return 0;
}
