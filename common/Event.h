#ifndef COMMON_EVENT_H
#define COMMON_EVENT_H

#include <vector>
#include <map>
#include <string>
#include <functional>
#include <mutex>
#include <future>


namespace sdb {

class EventManager;

class ThreadPool;

class TimerEvent;


// Base class to handle non-blocking (or async) events
// Users of this framework should override
//
//   @handleRead(),
//   @handleWrite(),
//   @handleError()
//
// methods to respond to external events
//
class Event {
 public:

  virtual ~Event() {}

  virtual int getFD() const = 0;

  virtual void handleRead() = 0;

  virtual void handleWrite() = 0;

  virtual void handleError() = 0;
};


// Provide a way to run an Event in an EventLoop.
//
// After the object is added to event thread, calling @notify_one()
// will invoke @process() in the event thread.
class SynchronizedEvent : virtual public Event {
 public:

  SynchronizedEvent();

  ~SynchronizedEvent() override;

  int getFD() const override;

  void handleRead() override;

  void handleWrite() override {}

  void handleError() override {}

  void notify_one();

  virtual void process() = 0;

 protected:

  int fds_[2];

  void consumeWrites();

};


// AlarmEvent can be used as a timer. When the timer sets off,
// @handleTimeout() will be called.
class AlarmEvent : virtual public Event {
 public:

  int getFD() const override { return -1; }

  uint64_t getID() const;

  void handleRead() override {}

  void handleWrite() override {}

  void handleError() override {}

  virtual void handleTimeout() = 0;

};


/// A worker thread to process pending events
class EventLoop {
 public:

  /// when an event is added, use following options to subscribe
  /// notifications
  enum {
    // The object does not interest in any events
    handle_none = 0,

    // If there is a read event, @handleRead should be called
    handle_read = 1,

    // If there is a write event, @handleWrite should be called
    handle_write = 2,

    // If there is an error event, @handleError should be called
    handle_error = 4,
  };

 public:

  explicit EventLoop(int maxEvents);

  ~EventLoop();


  bool addEvent(Event* handler, int opt);

  bool updateEvent(Event* handler, int opt);

  /// For file descriptor based events, closing the descriptor will
  /// automatically removing the event from event thread.
  /// So this method may not be used very often.
  bool removeEvent(Event* handler);

  // submit an async task to run in the event loop
  void submit(std::function<void()>&& fn);

  // intended usage:
  //   future<int> fut = loop.async([]() -> int { return 5; });
  //   int ret = fut.get();
  template <class Fn>
  auto async(Fn&& callback) -> std::future<decltype(callback())> {
    typedef decltype(callback()) return_type;

    auto pr = new std::promise<return_type>();
    auto fut = pr->get_future();

    submit([pr, callback]() {
      auto ret = callback();
      pr->set_value(ret);
      delete pr;
    });

    return fut;
  }

  void loop();

  // get file descriptor corresponding to this event thread
  int getFD() const;

  // issue a quit loop request, but do not wait for it
  void quitLoopSoon();


 private:

  int fd_;

  const int maxEvents_;

  bool quit_;

};


class EventManager {
 public:

  EventManager(int numThreads, int numTimers);

  ~EventManager();


  // let event manage to pick an event loop and submit the callback.
  void submit(std::function<void()>&& callback);

  // let event manager to pick a timer and schedule the task
  void submit(int64_t absTimeoutInMs, std::function<void()>&& callback);

  // let event manager to pick an event loop and submit the task
  // intended usage:
  //   future<int> fut = eventManager.async([]() -> int { return 5; });
  //   int ret = fut.get();
  template <class Fn>
  auto async(Fn&& callback) -> std::future<decltype(callback())> {
    typedef decltype(callback()) return_type;

    auto pr = new std::promise<return_type>();
    auto fut = pr->get_future();

    submit([pr, callback]() {
      auto ret = callback();
      pr->set_value(ret);
      delete pr;
    });

    return fut;
  }

  // let event manager to pick an timer and shcedule the task
  // intended usage:
  //   future<int> fut = eventManager.schedule(absTime, []() { return 5; });
  //   int ret = fut.get();
  template <class Fn>
  auto schedule(int64_t ts, Fn&& cb) -> std::future<decltype(cb())> {
    typedef decltype(cb()) return_type;

    auto pr = new std::promise<return_type>();
    auto fut = pr->get_future();

    submit(ts, [cb, pr]() {
      auto ret = cb();
      pr->set_value(ret);
      delete pr;
    });

    return fut;
  }

  EventLoop* pickEventLoop();

  // suspend all nonblocking threads
  void drain();

 private:

  ThreadPool* tp_;

  ThreadPool* timerTp_;

  std::vector<EventLoop*> epolls_;

  std::vector<EventLoop*> timerEpolls_;

  std::vector<TimerEvent*> timers_;

  int idx_;

};

}

#endif // COMMON_EVENT_H
