#include "ThreadPool.h"

#include <iostream>
#include <chrono>
#include <sstream>

using xdecroc::threadLib::ThreadPool;


int main(int argc, char* argv[]) { 

    bool verbose = false;    

    if (argc < 2) {        
        std::cerr << "Usage: " << argv[0] << " noThreads [-v verbose]" << std::endl;
        
        return 1;
    }
    
    std::istringstream ss(argv[1]);
    int tc;
    if (!(ss >> tc))
      std::cerr << "Invalid number " << argv[1] << '\n';

    if (argc == 3) {        
      verbose = true;
    }
    
    ThreadPool pool(tc); 
    int JOBS = 100;

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    std::cout << pool.Size() << " Worker Threads processing " << JOBS << " tasks" << std::endl;

    
    // queue the tasks for the worker threads
    for( int i = 0; i < JOBS; ++i )
        pool.AddJob( [i,&pool,&verbose] { 
            std::this_thread::sleep_for( std::chrono::seconds( 1 ));   
            if (verbose) 
              pool.printTaskDone(i);               
        } );

    pool.JoinAll(); // wait until all task queue complete, then join workers thread back to main
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "elapsed time: " << elapsed_seconds.count() << std::endl;    
}
