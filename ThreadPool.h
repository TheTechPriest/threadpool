#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <cmath>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
  void Start();
  template <class F, class... Args>
  auto QueueJob(F &&f, Args &&...args)
      -> std::future<typename std::invoke_result<F, Args...>::type> {
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      jobs.emplace([task]() { (*task)(); });
    }
    mutex_condition.notify_one();
    return res;
  }
  void Stop();
  bool busy();

private:
  void ThreadLoop();

  bool should_terminate{false};
  std::mutex queue_mutex;
  std::condition_variable mutex_condition;
  std::vector<std::thread> threads;
  std::queue<std::function<void()>> jobs;
};

#endif
