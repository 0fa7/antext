#pragma once

#include <list>
#include <mutex>

template <typename T> class spsc_queue {
public:
  spsc_queue(){};
  ~spsc_queue(){};
  spsc_queue(spsc_queue &other) = delete;
  spsc_queue(spsc_queue &&other) = delete;
  spsc_queue &operator=(spsc_queue &rhs) = delete;
  spsc_queue &operator=(spsc_queue &&rhs) = delete;

  void push(T item) {
    std::lock_guard lock(mutex_);
    list_.push_back(item);
  }

  bool try_pop(T &item) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!list_.empty()) {
      item = list_.front();
      list_.pop_front();
    } else {
      return false;
    }

    return true;
  }

  bool empty() const {
    std::lock_guard lock(mutex_);
    return list_.empty();
  }

private:
  std::list<T> list_;
  mutable std::mutex mutex_;
};