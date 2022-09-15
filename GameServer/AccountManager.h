// AccountManager.h

#pragma once

#include <mutex>

class Account
{

};

class AccountManager
{
private:
    mutex mMutex;

public:
    static AccountManager* Instance()
    {
        static AccountManager instance;
        return &instance;
    }

    Account* GetAccount(int32 id)
    {
        lock_guard<mutex> guard(mMutex);
        return nullptr;
    }

    void ProcessLogin();

};

