// ConcurrentStack.h

#pragma once

#include <mutex>

template<typename T>
class LockStack
{
public:
    LockStack() {}

    LockStack(const LockStack&) = delete;
    LockStack& operator=(const LockStack&) = delete;

    void TryPush(T value)
    {
        lock_guard<mutex> lock(_mutex);
        _stack.push(move(value));
        _condVar.notify_one();
    }

    bool TryPop(T& value)
    {
        lock_guard<mutex> lock(_mutex);
        if (_stack.empty()) { return false; }

        value = move(_stack.top());
        _stack.pop();
        return true;
    }

    bool Empty() // 다만, Empty 체크를 하고 다른 행동을 하게 되면 그 사이에 다른 스레드가 침입할 수 있기에 문제가 될 수 있음.
    {
        lock_guard<mutex> lock(_mutex);
        return _stack.empty();
    }

    void WaitPop(T& value)
    {
        unique_lock<mutex> lock(_mutex);
        _condVar.wait(lock, [this]() { return false == _stack.empty(); });
        value = move(_stack.top());
        _stack.pop();
    }

private:
    stack<T> _stack;
    mutex _mutex;
    condition_variable _condVar;

public:

private:

};

template<typename T>
class LockFreeStack // 노드 기반의 스택
{
    struct Node
    {
        Node(const T& value)
            : data(value)
            , next(nullptr)
        {
        }

        T data;
        Node* next;
    };


private:
    atomic<Node*> _head;
    atomic<uint32> _popCount = 0; // pop을 실행 중인 스레드의 개수
    atomic<Node*> _pendingList;   // 삭제 되어야 할 노드들(첫 번째 노드만 기억하고, 나머지는 타고들어가고자 함.)

public:
    void Push(const T& value)
    {
        Node* node = new Node(value);
        node->next = _head;
        while (false == _head.compare_exchange_weak(node->next, node))
        {
        }
    }

    bool TryPop(T& value)
    {
        ++_popCount;

        Node* oldHead = _head;
        while (nullptr != oldHead && false == _head.compare_exchange_weak(oldHead, oldHead->next))
        {
        }

        if (nullptr == oldHead) 
        { 
            --_popCount;
            return false; 
        }

        value = oldHead->data; // 노드 추출 완료.
        TryDelete(oldHead);

        return true;
    }


    // TryDelete()의 호출 될 때
    // 1. 데이터 분리
    // 2. Count 체크
    // 3. 나혼자면 삭제
    // 의 순서대로 흘러감. 따라서 _PopCount가 1이면 혼자라는 것.
    void TryDelete(Node* oldHead)
    {
        // 자신 스레드 외에 또 다른 스레드가 참조 하는가?
        if (1u == _Popcount)
        {
            // 자신 스레드 밖에 없음.

            // 이왕 혼자인거, 삭제 예약된 다른 데이터들도 삭제해보고자 함.
            // 다만 _PopCount가 1이라해도, PendingList의 노드들도 지울 수 있는지는 모름. 그래서 한 번 더 체크가 필요해짐.
            Node* node = _pendingList.exchange(nullptr);
            if (0u == --_popCount) // 끼어든 애가 없음. -> 삭제 진행
            {
                // 지금 누가 끼어들어도, 어차피 데이터는 분리해둔 상태.
                DeleteNodes(node);
            }
            else                // 간발의 차로 끼어든 애가 있음.
            {
                // 다시 갖다 놓자.
                ChainPendingNodeList(node);
            }

            // 내 데이터는 삭제. 
            delete oldHead;
            oldHead = nullptr;
        }
        else
        {
            // 다른 스레드가 있다면 지금 삭제하지 않고, 삭제 예약만 걸어둠.
            ChainPendingNode(oldHead);
            --_Popcount;
        }
    }

    void ChainPendingNodeList(Node* first, Node* last)
    {
        last->next = _pendingList;
        // 또 다른 스레드가 끼어들 수 있음.
        while (false == _pendingList.compare_exchange_weak(last->next, first))
        {
        }
    }

    void ChainPendingNodeList(Node* node)
    {
        Node* last = node;
        while (nullptr != last->next)
        {
            last = last->next;
        }

        ChainPendingNodeList(node, last);
    }

    void ChainPendingNode(Node* node)
    {
        ChainPendingNodeList(node, node);
    }

    static void DeleteNodes(Node* node)
    {
        while (nullptr != node)
        {
            Node* next = node->next;
            delete node;
            node = next;
        }
    }

};

