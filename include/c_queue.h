#ifndef C_QUEUE
#define C_QUEUE

#include <atomic>

#define CQUEUE_SIZE 2048

template <class T>
class CQueue {
    public:
        CQueue();
        ~CQueue();

        void enq(T e);
        T deq();

        int size();
    
    private:
        std::atomic_int             m_head;
        std::atomic_int             m_tail;

        T*                          m_buffer;
};

#endif