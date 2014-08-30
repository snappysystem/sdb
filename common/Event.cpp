#include "common/Event.h"
#include "common/ThreadPool.h"
#include "common/Logging.h"

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <chrono>

#include <unistd.h>
#include <sys/timerfd.h>

using namespace std;
using namespace std::chrono;

namespace sdb {

void setAlarm(int fd, int64_t abs) {
  int64_t secs = abs / 1000;
  struct itimerspec val;
  val.it_interval.tv_sec = 0;
  val.it_interval.tv_nsec = 0;
  val.it_value.tv_sec = secs;
  val.it_value.tv_nsec = (abs - secs * 1000) * 1000000LL;

  if (0 > timerfd_settime(fd, TFD_TIMER_ABSTIME, &val, nullptr)) {
    LOG(FATAL) << "Fails to set a timer!";
  }
}


class FunctionEvent : virtual public SynchronizedEvent {
 public:

  explicit FunctionEvent(function<void()>&& callback)
    : callback_(move(callback)) {}

  ~FunctionEvent() override {}

  void process() override {
    callback_();
    delete this;
  }

 private:

  function<void()> callback_;
};


class FunctionAlarmEvent : virtual public AlarmEvent {
 public:

  explicit FunctionAlarmEvent(function<void()>&& callback)
    : callback_(move(callback)) {}

  ~FunctionAlarmEvent() override {}

  void handleTimeout() override {
    callback_();
    delete this;
  }

 private:

  function<void()> callback_;
};


uint64_t AlarmEvent::getID() const {
  auto x = (uint64_t)this;
  return (x * 4398042316799ULL + 274876858367ULL);
}


SynchronizedEvent::SynchronizedEvent() {
  if (0 > pipe2(fds_, O_NONBLOCK | O_CLOEXEC)) {
    fds_[0] = fds_[1] = -1;
  }
}

SynchronizedEvent::~SynchronizedEvent() {
  if (fds_[1] >= 0) {
    close(fds_[1]);
  }
  if (fds_[0] >= 0) {
    close(fds_[0]);
  }
}

int SynchronizedEvent::getFD() const {
  return fds_[0];
}

void SynchronizedEvent::handleRead() {
  consumeWrites();
  process();
}

void SynchronizedEvent::notify_one() {
  if (fds_[1] >= 0) {
    if (0 > write(fds_[1], &fds_[0], 1)) {
      LOG(FATAL) << "fails to write to " << fds_[1];
    }
  }
}

void SynchronizedEvent::consumeWrites() {
  static const int maxReads = 64;
  char buf[maxReads];

  while (read(fds_[0], buf, maxReads) > 0);
}


// A wrapper of Linux timerfd API
class TimerEvent : public Event {
 public:

  TimerEvent(EventLoop* ep, ThreadPool* tp)
  : fd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
    ep_(ep),
    tp_(tp) {

    if (ep_ && fd_ >= 0) {
      ep_->addEvent(this, EventLoop::handle_read);
    }
  }

  ~TimerEvent() override {
    if (ep_) {
      ep_->removeEvent(this);
    }
    if (fd_ >= 0) {
      close(fd_);
    }
    for (auto& v : hmap_) {
      for (auto p : v.second) {
        delete p;
      }
    }
  }

  int getFD() const override {
    return fd_;
  }

  void handleRead() override {
    // first consume read buffer
    const int maxRead = 8;
    char buf[maxRead];
    while (read(fd_, buf, maxRead) > 0);

    auto now = steady_clock::now();
    int64_t ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
    vector<AlarmEvent*> tss;

    {
      lock_guard<mutex> l(mt_);
      auto it = hmap_.begin();
      while (it != hmap_.end()) {
        if (it->first > ms) {
          break;
        }

        tss.insert(tss.end(), it->second.begin(), it->second.end());
        ++it;
      }

      auto beg = hmap_.begin();
      if (it != beg) {
        hmap_.erase(beg, it);
      }

      if (it != hmap_.end()) {
        setAlarm(fd_, it->first);
      } else {
        setAlarm(fd_, 0);
      }
    }

    for (auto p : tss) {
      p->handleTimeout();
    }
  }

  void handleWrite() override {
  }

  void handleError() override {
  }


  void addTimeout(int64_t absTimeInMs, AlarmEvent* runnable) {
    int64_t lower = numeric_limits<int64_t>::max();
    lock_guard<mutex> l(mt_);
    auto it = hmap_.begin();
    if (it != hmap_.end()) {
      lower = it->first;
    }
    hmap_[absTimeInMs].push_back(runnable);
    if (absTimeInMs < lower) {
      setAlarm(fd_, absTimeInMs);
    }
  }

  EventLoop* getEventLoop() const {
    return ep_;
  }

 private:

  int fd_;

  EventLoop* ep_;

  ThreadPool* tp_;

  std::map<int64_t, std::vector<AlarmEvent*>> hmap_;

  std::mutex mt_;

};


EventLoop::EventLoop(int maxEvents) : maxEvents_(maxEvents) {
  fd_ = epoll_create(maxEvents_);
  quit_ = false;
}

EventLoop::~EventLoop() {
  if (fd_ >= 0) {
    close(fd_);
  }
}

bool EventLoop::addEvent(Event* handler, int opt) {
  struct epoll_event event;
  event.events = 0;
  if (opt & handle_read) {
    event.events |= EPOLLIN;
  }
  if (opt & handle_write) {
    event.events |= EPOLLOUT;
  }
  if (opt & handle_error) {
    event.events |= EPOLLERR;
  }
  event.data.ptr = (void*)handler;
  return (0 == epoll_ctl(fd_, EPOLL_CTL_ADD, handler->getFD(), &event));
}

bool EventLoop::updateEvent(Event* handler, int opt) {
  struct epoll_event event;
  event.events = 0;
  if (opt & handle_read) {
    event.events |= EPOLLIN;
  }
  if (opt & handle_write) {
    event.events |= EPOLLOUT;
  }
  if (opt & handle_error) {
    event.events |= EPOLLERR;
  }
  event.data.ptr = (void*)handler;
  return (0 == epoll_ctl(fd_, EPOLL_CTL_MOD, handler->getFD(), &event));
}

bool EventLoop::removeEvent(Event* handler) {
  struct epoll_event event;
  event.events = 0;
  event.data.ptr = (void*)handler;
  return (0 == epoll_ctl(fd_, EPOLL_CTL_DEL, handler->getFD(), &event));
}

void EventLoop::loop() {
  vector<struct epoll_event> events(maxEvents_);
  while (!quit_) {
    int num = epoll_wait(fd_, &events[0], maxEvents_, -1);
    for (int i = 0; i < num; ++i) {
      auto& e = events[i];

      if (e.events & EPOLLIN) {
        auto p = (Event*)e.data.ptr;
        p->handleRead();
      }
      if (e.events & EPOLLOUT) {
        auto p = (Event*)e.data.ptr;
        p->handleWrite();
      }
      if (e.events & ~(EPOLLIN | EPOLLOUT)) {
        auto p = (Event*)e.data.ptr;
        p->handleError();
      }
    }
  }
}

int EventLoop::getFD() const {
  return fd_;
}

void EventLoop::submit(function<void()>&& cob) {
  auto p = new FunctionEvent(move(cob));
  p->notify_one();
  addEvent(p, handle_read);
}

void EventLoop::quitLoopSoon() {
  quit_ = true;
  submit([]() {});
}


EventManager::
EventManager(int numLoops, int numTimers)
  : tp_(new ThreadPool(numLoops)),
    timerTp_(new ThreadPool(numTimers)),
    idx_(0) {

  const int maxEvents = 8192;

  for (int i = 0; i < numLoops; ++i) {
    auto loop = new EventLoop(maxEvents);
    epolls_.push_back(loop);
    tp_->submit([loop]() { loop->loop(); });
  }

  for (int i = 0; i < numTimers; ++i) {
    auto loop = new EventLoop(maxEvents);
    timerEpolls_.push_back(loop);
    timerTp_->submit([loop]() { loop->loop(); });
  }

  for (int i = 0; i < numTimers; ++i) {
    timers_.push_back(new TimerEvent(timerEpolls_[i], tp_));
  }
}

EventManager::~EventManager() {
  for (auto p : timers_) {
    delete p;
  }

  delete timerTp_;
  delete tp_;
}

void EventManager::submit(function<void()>&& callback) {
  auto eventLoop = pickEventLoop();
  eventLoop->submit(move(callback));
}

void EventManager::submit(int64_t absTimeoutInMs, function<void()>&& callback) {
  auto event = new FunctionAlarmEvent(move(callback));
  int n = event->getID() % timers_.size();
  timers_[n]->addTimeout(absTimeoutInMs, event);
}

EventLoop* EventManager::pickEventLoop() {
  int n = idx_++ % epolls_.size();
  return epolls_[n];
}

void EventManager::drain() {
  for (auto& p : epolls_) {
    p->quitLoopSoon();
  }
  for (auto& p : timerEpolls_) {
    p->quitLoopSoon();
  }

  timerTp_->drain();
  tp_->drain();
}

}
