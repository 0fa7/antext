#pragma once

#include <list>
#include <mutex>

class mt_work_queue
{
public:
  mt_work_queue();
  ~mt_work_queue();
  mt_work_queue(mt_work_queue& other) = delete;
  mt_work_queue(mt_work_queue&& other) = delete;
  mt_work_queue& operator=(mt_work_queue& rhs) = delete;
  mt_work_queue& operator=(mt_work_queue&& rhs) = delete;

  void push(int64_t item);
  int64_t try_pop();
  bool empty() const;

private:
  std::list<int64_t> list_;
  mutable std::mutex mutex_;
};