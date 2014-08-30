#ifndef COMMON_THREADFPOOL_H
#define COMMON_THREADFPOOL_H

#include <memory>
#include <vector>
#include <deque>
#include <string>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>


namespace sdb {


class ThreadPool {
 public:

  // Construct a ThreadPool object with max number of workers
  explicit ThreadPool(int numWorkers);

  ~ThreadPool();


  void submit(std::function<void()>&& closure);

  // A typical usage is to submit a closure R():
  //   future<R> fut = threadPool.async([]() { return R(); });
  //   R ret = fut.get();
  template <class Fn>
  auto async(Fn&& closure) -> std::future<decltype(closure())> {
    typedef decltype(closure()) R;
    auto pr = new std::promise<R>();

    submit([pr, closure]() {
      R ret = closure();
      pr->set_value(ret);
      delete pr;
    });
    return pr->get_future();
  }

  // block waiting until all tasks finish.
  void drain();


 private:

  struct Worker {
    std::mutex mt;
    std::condition_variable cond;
    std::function<void()> cb;
    bool hasFunctor;
    ThreadPool *pool;

    explicit Worker(ThreadPool* p);

    void run();
  };


  int numWorkers_;

  std::mutex mt_;

  std::condition_variable cond_;

  std::vector<std::unique_ptr<Worker> > freed_;

  std::vector<std::unique_ptr<Worker> > running_;

  std::deque<std::function<void()>> pendings_;


  bool start();

  // mark a worker as free state so that we can assign next task to it.
  void setFree(Worker* w);

};

}

#endif // COMMON_THREADFPOOL_H
