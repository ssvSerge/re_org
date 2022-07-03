#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>
// Wrapper for condition variable/mutex.

namespace utils
{
    class thread_event
    {
    public:
        thread_event()
        {
            m_bLockReady = false;
        }
        bool wait()
        {
            // Wait until main() sends data
            std::unique_lock<std::mutex> lk(m_mutex);
            m_cv.wait(lk, [this] { return m_bLockReady; });
            m_bLockReady = false;
            return true;
        }

        bool wait_for(uint32_t milliseconds)
        {
            std::unique_lock<std::mutex> lk(m_mutex);
            if (false == m_cv.wait_for(lk, std::chrono::milliseconds(milliseconds), [this] { return m_bLockReady; }))
                return false;
            m_bLockReady = false;
            return true;
        }

        bool notify_one()
        {
            {
                std::lock_guard<std::mutex> lk(m_mutex);
                m_bLockReady = true;
            }
            m_cv.notify_one();
            return true;
        }

    private:
        std::condition_variable m_cv;
        std::mutex m_mutex;
        bool m_bLockReady;
    };

    // A threadsafe-queue.
    template <class T>
    class safe_queue
    {
    public:
        safe_queue(void)
            : q(), m(), c()
        {
        }

        ~safe_queue(void)
        {
        }

        // Add an element to the queue.
        void enqueue(T t)
        {
            //std::lock_guard<std::mutex> lock(m);
            std::unique_lock<std::mutex> lock(m);
            //m.lock();
            q.push(t);
            //m.unlock();
            c.notify_all();
        }

        // Get the "front"-element.
        // If the queue is empty, wait till a element is avaiable.
        T dequeue(void)
        {
            std::unique_lock<std::mutex> lock(m);
            auto wait_suc = c.wait_for(lock, std::chrono::milliseconds(1000), [this]{return !q.empty();});
            if (!wait_suc){
                T val = T();
                return val;
            }
            T val = std::move(q.front());
            q.pop();
            return val;
        }
        void clear()
        {
            std::unique_lock<std::mutex> lock(m);
            while (false == q.empty()){
                dequeue();
            }
        }
        std::queue<T>& get_queue_unsafe() { return q;}
        bool empty(){
            std::unique_lock<std::mutex> lock(m);
            return q.empty();}
    private:
        std::queue<T> q;
        mutable std::mutex m;
        std::condition_variable c;
    };

} // namespace utils
