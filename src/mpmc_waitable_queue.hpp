#pragma once

#include <condition_variable>
#include <list>
#include <mutex>

template <typename T> class mpmc_waitable_queue {
public:
  mpmc_waitable_queue(){};
  ~mpmc_waitable_queue(){};
  mpmc_waitable_queue(mpmc_waitable_queue &other) = delete;
  mpmc_waitable_queue(mpmc_waitable_queue &&other) = delete;
  mpmc_waitable_queue &operator=(mpmc_waitable_queue &rhs) = delete;
  mpmc_waitable_queue &operator=(mpmc_waitable_queue &&rhs) = delete;

  void push(T item) {
    std::unique_lock lock(mutex_);
    list_.push_back(item);
    lock.unlock();
    cvar_.notify_one();
  }

  void wait_pop(T &item) {
    std::unique_lock<std::mutex> lock(mutex_);

    while (list_.empty()) {
      cvar_.wait(lock);
    }

    item = list_.front();
    list_.pop_front();
  }

  bool empty() const {
    std::lock_guard lock(mutex_);
    return list_.empty();
  }

private:
  std::condition_variable cvar_;
  std::list<T> list_;
  std::mutex mutex_;
};