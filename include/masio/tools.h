#ifndef __MASIO_TOOLS_H__
#define __MASIO_TOOLS_H__

namespace masio {

// To explicityly capture some variables inside a lambda
// function as it seems a bug in g++ prohibits this when
// [=] is used.
//
// Example:
//   void run_buggy(const Rest& rest) const {
//     auto self = this->shared_from_this();
//     _io_service.post([=, self]() { _run(rest); });
//   }
//
//   void run_ok(const Rest& rest) const {
//     auto self = this->shared_from_this();
//     _io_service.post([=]() { capture(self); _run(rest); });
//   }
template<class A> void capture(const A&) {}


} // masio namespace

#endif // ifndef __MASIO_TOOLS_H__
