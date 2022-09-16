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

    bool Empty() // �ٸ�, Empty üũ�� �ϰ� �ٸ� �ൿ�� �ϰ� �Ǹ� �� ���̿� �ٸ� �����尡 ħ���� �� �ֱ⿡ ������ �� �� ����.
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
class LockFreeStack // ��� ����� ����
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
    atomic<uint32> _popCount = 0; // pop�� ���� ���� �������� ����
    atomic<Node*> _pendingList;   // ���� �Ǿ�� �� ����(ù ��° ��常 ����ϰ�, �������� Ÿ������� ��.)

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

        value = oldHead->data; // ��� ���� �Ϸ�.
        TryDelete(oldHead);

        return true;
    }


    // TryDelete()�� ȣ�� �� ��
    // 1. ������ �и�
    // 2. Count üũ
    // 3. ��ȥ�ڸ� ����
    // �� ������� �귯��. ���� _PopCount�� 1�̸� ȥ�ڶ�� ��.
    void TryDelete(Node* oldHead)
    {
        // �ڽ� ������ �ܿ� �� �ٸ� �����尡 ���� �ϴ°�?
        if (1u == _Popcount)
        {
            // �ڽ� ������ �ۿ� ����.

            // �̿� ȥ���ΰ�, ���� ����� �ٸ� �����͵鵵 �����غ����� ��.
            // �ٸ� _PopCount�� 1�̶��ص�, PendingList�� ���鵵 ���� �� �ִ����� ��. �׷��� �� �� �� üũ�� �ʿ�����.
            Node* node = _pendingList.exchange(nullptr);
            if (0u == --_popCount) // ����� �ְ� ����. -> ���� ����
            {
                // ���� ���� �����, ������ �����ʹ� �и��ص� ����.
                DeleteNodes(node);
            }
            else                // ������ ���� ����� �ְ� ����.
            {
                // �ٽ� ���� ����.
                ChainPendingNodeList(node);
            }

            // �� �����ʹ� ����. 
            delete oldHead;
            oldHead = nullptr;
        }
        else
        {
            // �ٸ� �����尡 �ִٸ� ���� �������� �ʰ�, ���� ���ุ �ɾ��.
            ChainPendingNode(oldHead);
            --_Popcount;
        }
    }

    void ChainPendingNodeList(Node* first, Node* last)
    {
        last->next = _pendingList;
        // �� �ٸ� �����尡 ����� �� ����.
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

