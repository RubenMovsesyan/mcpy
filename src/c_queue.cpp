#include "c_queue.h"

#include <thread>

template <class T>
CQueue<T>::CQueue() {
    // initialize the buffer
    m_buffer = new T[CQUEUE_SIZE];

    // start both the head and tail at index 0
    m_head.store(0);
    m_tail.store(0);
}

template <class T>
CQueue<T>::~CQueue() {
    delete m_buffer;
}

template <class T>
void CQueue<T>::enq(T e) {
    // If the queue is full then spin
    while (size() >= CQUEUE_SIZE) { std::this_thread::yield(); }

    // use a temp variable for stack to save clock cycles on getting head index
    int h = m_head.load();

    m_buffer[h] = e;
    m_head.store((h + 1) % CQUEUE_SIZE);
}

template <class T>
T CQueue<T>::deq() {
    T ret;
    // while the queue is empty spin
    while (size == 0) { std::this_thread::yield(); }
    // store as temp var to save time
    int t = m_tail.load();

    ret = m_buffer[t];
    m_tail.store((t + 1) % CQUEUE_SIZE);
    return ret;
}

// This function might not be thread safe
// TAKE CAUTION WHEN USING
template <class T>
void CQueue<T>::clear() {
    m_head.store(m_tail.load());
}

template <class T>
int CQueue<T>::size() {
    int ret;
    ret = m_head.load() - m_tail.load();

    // we need to get the absolute value of ret and return that
    return (ret < 0) ? ret + CQUEUE_SIZE : ret;
}