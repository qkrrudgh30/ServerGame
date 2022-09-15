// GameServer.cpp

#include "pch.h"
#include "CorePch.h"

#include <iostream>
#include <thread>
#include <mutex> 
#include <atomic>
#include <future>
#include <Windows.h>

int32 x = 0;
int32 y = 0;
int32 r1 = 0;
int32 r2 = 0;

volatile bool ready;

void Thread_1()
{
    while(false == ready) {}

    y = 1;  // Store y
    r1 = x; // Load  x
}

void Thread_2()
{
    while (false == ready) {}

    x = 1;  // Store x
    r2 = y; // Load  y
}

int main()
{
    int32 nCount = 0;

    while (true)
    {
        ready = false;
        ++nCount;

        x = y = r1 = r2 = 0;

        thread t1(Thread_1);
        thread t2(Thread_2);

        ready = true;

        t1.join();
        t2.join();

        if (0 == r1 && 0 == r2) { break; } // 이런 상황이 안생기는게 맞는데, 어떤 컴퓨터는 멈추게됨.
    }

    cout << nCount << "번 만에 빠져나옴!" << endl;

    return 0;
}
