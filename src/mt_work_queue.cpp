
#include "mt_work_queue.hpp"

mt_work_queue::mt_work_queue()
{

}

mt_work_queue::~mt_work_queue()
{
  
}

void mt_work_queue::push(int64_t item)
{
  std::lock_guard lock(mutex_);
  list_.push_back(item);
}

int64_t mt_work_queue::try_pop()
{
  int64_t item = 0;

  std::lock_guard<std::mutex> lock(mutex_);
  
  if(!list_.empty())
  {
    item = list_.front();
    list_.pop_front();
  }
  
  return item;
}

bool mt_work_queue::empty() const
{
  std::lock_guard lock(mutex_);
  return list_.empty();
}