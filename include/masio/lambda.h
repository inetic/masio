#ifndef __MASIO_LAMBDA_H__
#define __MASIO_LAMBDA_H__

#include <masio/cont.h>

namespace masio {

template<class A> struct Lambda : public Cont<A> {
  typedef typename Cont<A>::CancelerPtr CancelerPtr;
  typedef typename Cont<A>::Rest        Rest;
  typedef typename Cont<A>::Run         Run;

  Lambda() {}
  Lambda(const Run& run) : _run(run) { }

  void run(const CancelerPtr& canceler, const Rest& rest) const {
    _run(canceler, rest);
  }

  Run _run;
};

}

#endif // ifndef __MASIO_LAMBDA_H__
