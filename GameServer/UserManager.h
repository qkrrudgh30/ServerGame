// UserManager.h

#pragma once

#include <mutex>

class User
{

};

class UserManager
{
private:
    mutex mMutex;

public:
    static UserManager* Instance()
    {
        static UserManager instance;
        return &instance;
    }

    User* GetUser(int32 id)
    {
        lock_guard<mutex> guard(mMutex);
        return nullptr;
    }

    void ProcessSave();

};

