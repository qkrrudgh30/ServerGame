// AccountManager.cpp

#include "pch.h"
#include "AccountManager.h"
#include "UserManager.h"

void AccountManager::ProcessLogin()
{
    // accountLock
    lock_guard<mutex> guard(mMutex);

    // userLock
    User* user = UserManager::Instance()->GetUser(100);

    // TODO
}
