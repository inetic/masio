#ifndef __MASIO_LAMBDA_H__
#define __MASIO_LAMBDA_H__

namespace masio {

template<class A> struct Lambda : public Task<A> {
  typedef typename Task<A>::CancelerPtr CancelerPtr;
  typedef typename Task<A>::Rest        Rest;
  typedef typename Task<A>::Run         Run;

  Lambda() {}
  Lambda(const Run& run) : _run(run) { }

  void run(const CancelerPtr& canceler, const Rest& rest) const {
    _run(canceler, rest);
  }

  Run _run;
};

}

#endif // ifndef __MASIO_LAMBDA_H__
