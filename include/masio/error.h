#ifndef __MASIO_ERROR_H__
#define __MASIO_ERROR_H__

#include <boost/variant.hpp>

namespace masio {

namespace __detail {
  template<class A> struct Success { A value; };
  struct Fail    { boost::system::error_code value; };
}

template<class A> struct Error
    : public boost::variant<__detail::Success<A>, __detail::Fail>
{
  typedef __detail::Success<A>          Success;
  typedef __detail::Fail                Fail;

  typedef boost::system::error_code     ErrorCode;
  typedef boost::variant<Success, Fail> Super;

  Error() {}
  Error(const Success& a) : Super(a) {}
  Error(const Fail&    a) : Super(a) {}

  bool is_error() const;
  bool is_value() const;

  const A&  value() const;
  ErrorCode error() const;
};

template<class A>
bool Error<A>::is_error() const {
  if (const Fail* pe = boost::get<Fail>(this)) {
    return pe->value != ErrorCode(); // ErrorCode() means success.
  }
  return false;
}

template<class A>
bool Error<A>::is_value() const {
  return boost::get<Success>(this) != nullptr;
}

template<class A>
const A& Error<A>::value() const {
  return boost::get<Success>(*this).value;
}

template<class A>
typename Error<A>::ErrorCode Error<A>::error() const {
  if (!is_error()) { return ErrorCode(); }
  return boost::get<Fail>(*this).value;
}

template<class A>
std::ostream& operator<<(std::ostream& os, const Error<A>& ea) {
  if (ea.is_error()) {
    os << "(Fail " << ea.error().message() << ")";
  }
  else {
    os << "(Success " << ea.value() << ")";
  }
}

} // masio namespace

#endif // ifndef __MASIO_ERROR_H__

