#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <iostream>

typedef std::unique_lock<std::mutex> ulock;
typedef std::lock_guard<std::mutex> glock;

namespace xdecroc {
namespace threadLib {

/**
 *  Simple c++11 ThreadPool impl
 *  creates a number of threads given from the command line
 *  worker threads pull from the task queue until queue is exhausted
 *
 *  compile:
 *  g++ main.cpp -o _exec -std=c++11 -pthread
 *  
 *  usage: ./_exec <noOfThreads> 
 */

class ThreadPool {
    
    std::vector<std::thread> threads;
    std::queue<std::function<void(void)>> queue;

    int noThreads;
    std::atomic_int         tasksRemaining;
    std::atomic_bool        exitF;
    std::atomic_bool        finished;
    std::condition_variable tasksQEmpty_conVar;
    std::condition_variable mt_Qempty_var;
    std::mutex              wait_mutex;
    std::mutex              queue_mutex;
    std::mutex              cout_mutex;

    /**
     *  Get the next job in the queue and run it.
     *  Notify the main thread that a job has completed.
     */
    void Task() {
        while( !exitF ) {
            next_job()();
            --tasksRemaining;
            mt_Qempty_var.notify_one();
        }
    }

    /**
     *  Get the next job; pop the first item in the queue, 
     *  otherwise wait for a signal from the main thread (new job Added/exitF) .
     */
    std::function<void(void)> next_job() {
        std::function<void(void)> res;
        ulock job_lock( queue_mutex );

        // Wait for a job if we don't have any.
        tasksQEmpty_conVar.wait( job_lock, [this]() ->bool { return queue.size() || exitF; } );
        
        // Get job from the queue
        if( !exitF ) {
            res = queue.front();
            queue.pop();
        }
        else { // exitF signal to quit, 'inject' a job into the queue to keep tasksRemaining accurate.
            res = []{};
            ++tasksRemaining;
        }
        return res;
    }

public:
    ThreadPool(int tc)
        : tasksRemaining( 0 )
        , exitF( false )
        ,noThreads(tc)
        , finished( false ) 
    {
        for( unsigned i = 0; i < noThreads; ++i )
            threads.push_back(std::move( std::thread( [this,i]{ this->Task(); } ) ));
    }

    
    /**
     *  JoinAll on deconstruction
     */
    ~ThreadPool() {
        JoinAll();
    }

    /**
     *  Get the number of threads in this pool
     */
    inline unsigned Size() const {
        return noThreads;
    }

    /**
     *  Get the number of jobs left in the queue.
     */
    inline unsigned JobsRemaining() {
        glock guard( queue_mutex );
        return queue.size();
    }

    /**
     *  protect cout shared resource
     */
     void printTaskDone(int i) {
       glock lg( cout_mutex );
       std::cout << "task" << i << " done."<< std::endl;
     }
     


    /**
     *  Add a new job to the pool. If there are no jobs in the queue,
     *  a thread is woken up to take the job. If all threads are busy,
     *  the job is added to the end of the queue.
     */
    void AddJob( std::function<void(void)> job ) {
        glock guard( queue_mutex );
        queue.emplace( job );
        ++tasksRemaining;
        tasksQEmpty_conVar.notify_one();
    }

    /**
     *  Join with all threads. Block until all threads have completed.
     *  Params: WaitForAll: If true, will wait for the queue to empty 
     *          before joining with threads. If false, will complete
     *          current jobs, then inform the threads to exit.
     *  The queue will be empty after this call, and the threads will
     *  be done. After invoking `ThreadPool::JoinAll`, the pool can no
     *  longer be used. If you need the pool to exist past completion
     *  of jobs, look to use `ThreadPool::WaitAll`.
     */
    void JoinAll( bool WaitForAll = true ) {
        if( !finished ) {
            if( WaitForAll ) {
                WaitAll();
            }

            // note that we're done, and wake up any thread that's
            // waiting for a new job
            exitF = true;
            tasksQEmpty_conVar.notify_all();

            for( auto &x : threads )
                if( x.joinable() )
                    x.join();
            finished = true;
        }
    }

    /**
     *  Wait for the pool to empty before continuing. 
     *  This does not call `std::thread::join`, it only waits until
     *  all jobs have finshed executing.
     */
    void WaitAll() {
        if( tasksRemaining > 0 ) {
            ulock lk( wait_mutex );
            mt_Qempty_var.wait( lk, [this]{ return this->tasksRemaining == 0; } );
            lk.unlock();
        }
    }
};

} // namespace threadLib
} // namespace xdecroc

#endif //THREADPOOL_H
