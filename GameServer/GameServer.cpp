// GameServer.cpp

#include "pch.h"
#include "CorePch.h"

#include <iostream>
#include <thread>
#include <mutex> 

vector<int32> g_vData; 
mutex m; 

void Push()
{
    lock_guard<mutex> lockGuard(m);
    for (uint32 i = 0; i < 10'000; ++i)
    {
        g_vData.push_back(i); 
    }
}

int main()
{
    std::thread t1(Push);
    std::thread t2(Push);

    t1.join();
    t2.join();

    cout << g_vData.size() << endl; 

    return 0;
}
