#pragma once


//#define IN_PARALLEL
//#define IN_PARALLEL_CHECK_EXPECTATIONS
#define IN_PARALLEL_LAZY_THREAD_CREATION
//#define IN_PARALLEL_NO_KERNEL
//#define IN_PARALLEL_TEST
//#define IN_PARALLEL_TEST_PLAIN_LOOP


#ifdef IN_PARALLEL

#    pragma push_macro("max")
#    pragma push_macro("min")
#    undef max
#    undef min

#    include <functional>
#    include <algorithm>
#    include <list>
#    include <thread>
#    include <atomic>
#    include <memory>
#    include <tuple>
#    ifndef IN_PARALLEL_NO_KERNEL
#        include <mutex>
#        include <condition_variable>
#    endif
#    ifdef IN_PARALLEL_TEST
#        include <chrono>
#    endif
#    include <assert.h>


// control where to use parallel
#    define DO_PARALLEL_KALQBDEC_1
#    define DO_PARALLEL_KALQBDEC_2
#    define DO_PARALLEL_KALQBDEC_3
#    define DO_PARALLEL_KALQBRECONSTRUCT_1
#    define DO_PARALLEL_KALQBRECONSTRUCT_2
#    define DO_PARALLEL_PROCMINT_1
#    define DO_PARALLEL_PROCMINT_2
#    define DO_PARALLEL_CDCORRECTION_1
#    define DO_PARALLEL_CDCORRECTION_2
#    define DO_PARALLEL_CDCORRECTION_3
#    define DO_PARALLEL_CDCORRECTION_4
#    define DO_PARALLEL_CDCORRECTION_5
#    define DO_PARALLEL_CDCORRECTION_6
#    define DO_PARALLEL_CDCORRECTION_7
#    define DO_PARALLEL_CDCORRECTION_8
#    define DO_PARALLEL_B11_1
#    define DO_PARALLEL_B11_2
#    define DO_PARALLEL_B11_3
#    define DO_PARALLEL_B11_4
#    define DO_PARALLEL_B11_5
#    define DO_PARALLEL_B11_6
#    define DO_PARALLEL_B11_7
#    define DO_PARALLEL_B11_8
#    define DO_PARALLEL_B11_9
#    define DO_PARALLEL_B11_10
#    define DO_PARALLEL_B11_11
#    define DO_PARALLEL_B11_12
#    define DO_PARALLEL_B11_13
#    define DO_PARALLEL_B11_14
#    define DO_PARALLEL_MATH_1
#    define DO_PARALLEL_MATH_2
#    define DO_PARALLEL_MATH_3
#    define DO_PARALLEL_MATH_4
#    define DO_PARALLEL_MATH_5
#    define DO_PARALLEL_EXTRACT
#    define DO_PARALLEL_MAKECOLORSTATS_1
#    define DO_PARALLEL_MAKECOLORSTATS_2
#    define DO_PARALLEL_MAKECOLORSTATS_3


// debug build asserts for parallel constructs
#    ifdef IN_PARALLEL_CHECK_EXPECTATIONS
// check that expression has expected value; doesn't have to have deeper meaning
#        define parallel_expect(x)                                    assert(x)
#        define parallel_expect_order_of_magnitude(x, y)                parallel_expect(round(log10(x)) == round(log10(y)));
#        define parallel_expect_order_of_magnitude_or_less(x, y)        parallel_expect(round(log10(x)) <= round(log10(y)));

#    else
#        define parallel_expect(x)                                    ((void)0)
#        define parallel_expect_order_of_magnitude(x, y)                ((void)0)
#        define parallel_expect_order_of_magnitude_or_less(x, y)        ((void)0)
#    endif


// how to use template:
//parallel_expect(cnt == ...);
//auto &s = get_scheduler();
//
//s.run(start, end, [=](uint32_t tid, int start, int end) {
//});



namespace parallel
{

//const uint32_t cores_cnt = std::thread::hardware_concurrency();
//const uint32_t cores_cnt = 2;
//const uint32_t cores_cnt = 4;
const uint32_t cores_cnt = std::min<uint32_t>(4, std::thread::hardware_concurrency());



#ifdef IN_PARALLEL_NO_KERNEL
class spinlock
{
    std::atomic_flag locked /*= ATOMIC_FLAG_INIT*/;
public:
    void lock()
    {
        std::atomic_flag locked2 = ATOMIC_FLAG_INIT;
        std::swap(locked, locked2);            // VS2013 doesn't support in-class initialization with ATOMIC_FLAG_INIT
        while(locked.test_and_set(std::memory_order_acquire)) { }
    }
    void unlock()
    {
        locked.clear(std::memory_order_release);
    }
};
class spinlock_lock
{
    spinlock &sl;
public:
    spinlock_lock(spinlock &sl) : sl(sl) { sl.lock(); }
    ~spinlock_lock() { sl.unlock(); }
};
class spinsemaphore
{
    blip
};
#endif


class scheduler
{
public:
    using func_t = std::function<void(uint32_t t, int, int)>;        // tid, start, count
//    using func_ptr = std::shared_ptr<func_t>;
    using work_t = std::list<std::tuple<std::function<void(int)>, int, int>>;


    scheduler(uint32_t max_threads = cores_cnt) : in_parallel_cnt(std::max(max_threads, 1u)), threads_cnt(in_parallel_cnt - 1), enabled(threads_cnt > 0), ready_count(0)
    {
#ifndef IN_PARALLEL_LAZY_THREAD_CREATION
        if(enabled)
            create_threads(threads_cnt);
#endif
    }

    ~scheduler()
    {
        threads.clear();
    }

    // returns how many threads will be used for specified number of elements
    uint32_t threads_used_for(uint32_t count) const
    {
        return std::min(count, in_parallel_cnt);
    }

    // basic function to run for loop (from start to end - 1) in chunks equally divided among threads
    void run(int start, int end, const func_t &func)
    {
//        printf("run(%d,%d)\n", start, end);
#ifdef _DEBUG
        static std::atomic<int> _recursion{0};
        ++_recursion;
//        if(_recursion != 1)
//            __debugbreak();
        assert(_recursion == 1);
#endif
#ifdef IN_PARALLEL_TEST
        func_t func2 = [&func](uint32_t tid, int start, int end) {
            if(!(rand() % 10))
                std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10 + 1));
            func(tid, start, end);
        };
#    define func func2
#endif

        assert(end >= start);        // for now, only ascending sequence
        uint32_t count = end - start;
        if(count == 0)
        {
#ifdef _DEBUG
            --_recursion;
#endif
            return;
        }
        else if(count <= in_parallel_cnt)
        {
            ready_count = 0;
            create_threads(count - 1);
            for(uint32_t i = 1; i < count; i++)
                run_on_thread(i, start + i, start + i + 1, func);
            run_on_thread(0, start, start + 1, func);
            if(count > 1)
                wait_to_finish(count - 1);
        }
        else
        {
//            assert(count >= in_parallel_cnt * 2);        only a warning that parallelization is not optimal
            uint32_t count_per_thread = count / in_parallel_cnt;
            uint32_t surplus = count % in_parallel_cnt;
            uint32_t fg_surplus = std::min(surplus, 1u);
            surplus -= fg_surplus;

            ready_count = 0;
            create_threads(threads_cnt);
            for(uint32_t i = 0; i < threads_cnt; i++)
            {
                auto __size = count_per_thread;
                if(surplus > 0)
                {
                    ++__size;
                    --surplus;
                }
                run_on_thread(i + 1, start, start + __size, func);
                start += __size;
            }
            run_on_thread(0, start, start + count_per_thread + fg_surplus, func);
            wait_to_finish(threads_cnt);
        }

#ifdef _DEBUG
        --_recursion;
#endif
#ifdef IN_PARALLEL_TEST
#    undef func
#endif
    }

    // complex function to run different tasks (each in specified quantity) in parallel (tuple<func, start, end>
    void run_tasks(work_t &work, bool clear_on_end = true)
    {
        std::mutex m;
        auto it = work.begin();
        run(0, in_parallel_cnt, [&](uint32_t, int, int) {
            while(true)
            {
                std::unique_lock<std::mutex> l(m);
                while(true)
                {
                    if(it == work.end())
                        return;                        // we had done everything
                    if(std::get<1>(*it) >= std::get<2>(*it))
                        ++it;                        // move to the next task
                    else
                        break;
                }

                int i = std::get<1>(*it)++;
                auto &f = std::get<0>(*it);
                l.unlock();
                f(i);
            }
        });

        if(clear_on_end)
            work.clear();
    }


protected:
    void create_threads(size_t cnt)
    {
        assert(cnt <= threads_cnt);
        for(size_t i = threads.size(); i < cnt; i++)
            threads.emplace_back(ready_count);
    }

    void run_on_thread(uint32_t t, int start, int end, const func_t &func)
    {
        if(t == 0)
            func(t, start, end);
        else
        {
            assert(t <= threads.size());
            auto th = threads.begin();
            std::advance(th, t - 1);
            {
                std::unique_lock<std::mutex> l(th->m);
                th->job = [=, &func] { func(t, start, end); };
                th->cv.notify_one();
            }
        }
    }

    struct mythread
    {
        mythread(std::atomic<uint32_t> &ready_count)
        {
            std::atomic<bool> lock_held{false};
            t = std::thread([&] { running_thread(ready_count, lock_held); });
            while(!lock_held) {}
        }
        mythread(const mythread &) = delete;
        mythread &operator =(const mythread &) = delete;
        ~mythread()
        {
            {
                std::unique_lock<std::mutex> l(m);
                exit = true;
                cv.notify_one();
            }
            t.join();
        }

        void running_thread(std::atomic<uint32_t> &ready_count, std::atomic<bool> &lock_held)
        {
#ifndef IN_PARALLEL_NO_KERNEL
            std::unique_lock<std::mutex> l(m);
#else
            spinlock_lock l(sl);
#endif
            lock_held = true;        // now the thread that launched may continue so initial notify won't be missed

            while(true)
            {
                cv.wait(l);
                if(exit)
                    break;
//                assert(!!job);
                if(job)
                {
                    job();
                    job = nullptr;
                    ++ready_count;
                }
            }
            assert(!job);
        }

        std::thread t;
#ifndef IN_PARALLEL_NO_KERNEL
        std::mutex m;
        std::condition_variable cv;
#else
        spinlock sl;
#endif
        std::function<void()> job;
        std::atomic<bool> exit { false };
    };


    void wait_to_finish(uint32_t wait_count)
    {
        while(ready_count != wait_count)
        {
            // hog the main thread to resume as quickly as possible
            assert(ready_count <= wait_count);
        }
    }


    const uint32_t in_parallel_cnt;
    const uint32_t threads_cnt;
    const bool enabled;
    std::list<mythread> threads;
    std::atomic<uint32_t> ready_count;
};


extern std::unique_ptr<scheduler> s;
#ifdef _DEBUG
extern std::thread::id main_thread_id;
#endif
#ifdef IN_PARALLEL_IMPL
std::unique_ptr<scheduler> s;
#    ifdef _DEBUG
std::thread::id main_thread_id;
#    endif
#endif

inline scheduler &get_scheduler()
{
//    static scheduler s;
    assert(s);
    return *s;
}

inline void init_scheduler(uint32_t cores = cores_cnt)
{
    assert(!s);
#ifdef _DEBUG
    main_thread_id = std::this_thread::get_id();
#endif
    s = std::make_unique<scheduler>(cores);

//#ifndef NDEBUG
//    printf("Parallel enabled, %d cores (debug)!\n", cores);
//#else
//    printf("Parallel enabled, %d cores (release)!\n", cores);
//#endif
}
inline void exit_scheduler()
{
    s.reset();
}

#ifdef _DEBUG
inline bool is_main_thread()
{
    return main_thread_id == std::this_thread::get_id();
}
#endif

}
using parallel::get_scheduler;
using parallel::init_scheduler;
using parallel::exit_scheduler;
#ifdef _DEBUG
using parallel::is_main_thread;
#endif


#    pragma pop_macro("max")
#    pragma pop_macro("min")

#endif
