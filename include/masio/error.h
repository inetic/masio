#ifndef __MASIO_ERROR_H__
#define __MASIO_ERROR_H__

#include <boost/variant.hpp>

namespace masio {

template<class A> struct Error
    : public boost::variant<A, boost::system::error_code>
{
  typedef boost::system::error_code    ErrorCode;
  typedef boost::variant<A, ErrorCode> Super;

  Error(const A& a)  : Super(a) {}
  Error(ErrorCode e) : Super(e) {}

  bool is_error() const {
    if (const ErrorCode* pe = boost::get<ErrorCode>(this)) {
      return *pe != ErrorCode(); // ErrorCode() means success.
    }
    return false;
  }

  bool is_value() const { return boost::get<A>(this) != nullptr; }

  const A& value() const { return boost::get<A>(*this); }

  ErrorCode error() const {
    if (!is_error()) { return ErrorCode(); }
    return boost::get<ErrorCode>(*this);
  }
};

}

#endif // ifndef __MASIO_ERROR_H__

