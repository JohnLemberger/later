#ifndef _CALLBACK_REGISTRY_H_
#define _CALLBACK_REGISTRY_H_

#include <Rcpp.h>
#include <queue>
#include <boost/function.hpp>
#include "timestamp.h"
#include "optional.h"
#include "threadutils.h"

typedef boost::function0<void> Task;

class Callback {

public:
  Callback(Timestamp when, Task func) : when(when), func(func) {}
  
  bool operator<(const Callback& other) const {
    return this->when < other.when;
  }
  
  bool operator>(const Callback& other) const {
    return this->when > other.when;
  }
  
  void operator()() const {
    func();
  }

  Timestamp when;
  
private:
  Task func;
};

// Stores R function callbacks, ordered by timestamp.
class CallbackRegistry {
private:
  std::priority_queue<Callback,std::vector<Callback>,std::greater<Callback> > queue;
  mutable Mutex mutex;
  mutable ConditionVariable condvar;

public:
  CallbackRegistry();

  // Add a function to the registry, to be executed at `secs` seconds in
  // the future (i.e. relative to the current time).
  void add(Rcpp::Function func, double secs);
  
  // Add a C function to the registry, to be executed at `secs` seconds in
  // the future (i.e. relative to the current time).
  void add(void (*func)(void*), void* data, double secs);
  
  // The smallest timestamp present in the registry, if any.
  // Use this to determine the next time we need to pump events.
  Optional<Timestamp> nextTimestamp() const;
  
  // Is the registry completely empty?
  bool empty() const;
  
  // Is anything ready to execute?
  bool due(const Timestamp& time = Timestamp()) const;
  
  // Pop and return an ordered list of functions to execute now.
  std::vector<Callback> take(size_t max = -1, const Timestamp& time = Timestamp());
  
  bool wait(double timeoutSecs) const;
};

#endif // _CALLBACK_REGISTRY_H_
