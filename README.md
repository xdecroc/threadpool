# C++11 Threadpool impl 

worker threads pull from a queue of jobs untill the queue is empty.
jobs are stored in the queue with the type 'void function(void)'
this allows a generic way to adding jobs.
conditional variables are used to suspend threads when the condition
arises and allows notification to resume the thread when teh condition
is met.


# compile/ execute /usage

```bash
make
./threadpl 10

```
Usage: threadpl <noOfthreads>
The first command argument indictes the number of worker threads to be used in preocessing the jobs.

