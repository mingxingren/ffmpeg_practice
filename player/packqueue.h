#ifndef PACKQUEUE_H
#define PACKQUEUE_H

#include <QMutex>
#include <QQueue>

// this PackQueue has mutex lock
template <typename T>
class PackQueue
{
public:
    PackQueue()
    {

    }
    void enqueue(const T &t) {
        QMutexLocker lock(&m_Mutex);
        m_queue.enqueue(t);
    }

    T dequeue() {
        QMutexLocker lock(&m_Mutex);
        return m_queue.dequeue();
    }

    int size() {
        QMutexLocker lock(&m_Mutex);
        return m_queue.size();
    }

    inline T &head() {
        QMutexLocker lock(&m_Mutex);
        return m_queue.head();
    }

private:
    QMutex m_Mutex;
    QQueue<T> m_queue;
};

#endif // PACKQUEUE_H
