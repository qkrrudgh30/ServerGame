// ConcurrentQueue.h

#pragma once

#include <mutex>

template<typename T>
class LockQueue
{
public:
	LockQueue() {}

	LockQueue(const LockQueue&) = delete;
	LockQueue& operator=(const LockQueue&) = delete;

    void TryPush(T value)
    {
        lock_guard<mutex> lock(_mutex);
        _queue.push(move(value));
        _condVar.notify_one();
    }

    bool TryPop(T& value)
    {
        lock_guard<mutex> lock(_mutex);
        if (_queue.empty()) { return false; }

        value = move(_queue.front());
        _queue.pop();
        return true;
    }

    bool Empty() 
    {
        lock_guard<mutex> lock(_mutex);
        return _queue.empty();
    }

    void WaitPop(T& value)
    {
        unique_lock<mutex> lock(_mutex);
        _condVar.wait(lock, [this]() { return false == _queue.empty(); });
        value = move(_queue.front());
        _queue.pop();
    }

private:
    queue<T> _queue;
    mutex _mutex;
    condition_variable _condVar;

public:

private:

};

