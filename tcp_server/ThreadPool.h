#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <iostream>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

/**
 * @brief Threading Manager for handling threads more efficiently.
 */
class ThreadPool {

public:
    /**
     * @brief Constructs new thread pool object.
     * @param numThreads the number of threads in the pool
     */
    ThreadPool(size_t numThreads);

    /**
     * @brief Destroys the thread pool object.
     */
    ~ThreadPool();

    /**
     * @brief Queues a task to the task queue.
     */
    void enqueue(std::function<void()> task);

private:
    /**
     * @brief Assigns a task for an available thread
     */
    void workerThread();

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable cv;
    bool stop;
};

#endif // THREADPOOL_H