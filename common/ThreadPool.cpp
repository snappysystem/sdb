#include "common/ThreadPool.h"
#include "common/Logging.h"
#include <string>
#include <thread>

using namespace std;


namespace sdb {

ThreadPool::ThreadPool(int numWorkers) : numWorkers_(numWorkers) {
  start();
}

ThreadPool::~ThreadPool() {
}

bool ThreadPool::start() {
  lock_guard<mutex> g(mt_);
  for (int i = 0; i < numWorkers_; ++i) {
    pthread_t tid;
    freed_.emplace_back(new Worker(this));
    pthread_create(
      &tid,
      nullptr,
      [](void* p) -> void* {
        Worker* w = (Worker*)p;
        w->run();
        return nullptr;
      },
      (void*)freed_.back().get());
  }

  return true;
}

void ThreadPool::submit(function<void()>&& c) {
  lock_guard<mutex> l(mt_);
  if (freed_.empty()) {
    pendings_.push_back(move(c));
    return;
  }

  auto w = std::move(freed_.back());
  freed_.pop_back();

  {
    lock_guard<mutex> l(w->mt);
    w->hasFunctor = true;
    w->cb = move(c);
  }

  w->cond.notify_one();
  running_.push_back(std::move(w));
}

void ThreadPool::drain() {
  while (true) {
    unique_lock<mutex> l(mt_);
    while (!running_.empty()) {
      cond_.wait(l);
    }

    return;
  }
}

void ThreadPool::setFree(Worker* w) {
  lock_guard<mutex> l(mt_);

  if (!pendings_.empty()) {
    w->hasFunctor = true;
    w->cb = move(pendings_.front());
    pendings_.pop_front();
    return;
  }

  auto it = running_.begin();
  for (; it != running_.end(); ++it) {
    if (it->get() == w) {
      break;
    }
  }

  if (it == running_.end()) {
    LOG(FATAL) << "Fails to find worker to free!";
  }

  auto p = std::move(*it);
  running_.erase(it);
  freed_.push_back(std::move(p));

  cond_.notify_one();
}


ThreadPool::Worker::Worker(ThreadPool* p)
  : hasFunctor(false), pool(p) {
}

void ThreadPool::Worker::run() {
  while (true) {
    function<void()> fn;

    {
      unique_lock<mutex> l(mt);
      if (!hasFunctor) {
        cond.wait(l);
        continue;
      }

      fn = move(cb);
      hasFunctor = false;
    }

    try {
      fn();
    } catch (std::exception& e) {
      LOG(ERROR) << "pool worker " << hex << (uint64_t)this
                 << " caught exception " << e.what();
    } catch (...) {
      LOG(ERROR) << "pool worker " << hex << (uint64_t)this
                 << " caught unexpected exception!";
    }

    pool->setFree(this);
  }
}

}
