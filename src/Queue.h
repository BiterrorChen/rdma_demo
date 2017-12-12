/*************************************************************************
	> File Name: Queue.h
	> Author: chenchao
	> Mail: cqwzchenchao@163.com
	> Created Time: Tue 12 Dec 2017 10:32:38 AM CST
 ************************************************************************/

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdint.h>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <sys/syscall.h>
#include <unistd.h>

template <typename T>
class Queue {
private:
   std::vector<T> queue_;
   std::mutex m_;
   std::condition_variable cond_;
   uint8_t offset_ = 0;
public:
   Queue(){}
   ~Queue(){}
   T pop() {
     std::unique_lock<std::mutex> mlock(m_);
     while (queue_.empty()) {
       cond_.wait(mlock);
     }
     auto item = queue_.front();
     queue_.erase(queue_.begin());
     return item;
   }
   T PopPolling() {
     while (offset_ == 0);
     auto item = queue_.front();
     queue_.erase(queue_.begin());
     __sync_fetch_and_sub(&offset_, 1);
     return item;
   }
   void push(T item) {
     std::unique_lock<std::mutex> mlock(m_);
     queue_.push_back(item);
     mlock.unlock();
     cond_.notify_one();
   }
   void PushPolling(T item) {
     queue_.push_back(item);
     __sync_fetch_and_add(&offset_, 1);
   }
};
#endif
