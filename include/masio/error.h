#ifndef __MASIO_ERROR_H__
#define __MASIO_ERROR_H__

namespace masio {

namespace __detail {
  template<class A> struct Success { A value; };
  struct Fail    { boost::system::error_code value; };
}

template<class A> struct Error
{
  typedef __detail::Success<A>          Success;
  typedef __detail::Fail                Fail;

  typedef boost::system::error_code     ErrorCode;

  Error() {}
  Error(const Success& a) : s(a), is_success(true)  {}
  Error(const Fail&    a) : f(a), is_success(false) {}

  bool is_error() const;
  bool is_value() const;

  const A&  value() const;
  ErrorCode error() const;

  const A& operator*() const  { return value(); }
  const A* operator->() const { return &value(); }

private:
  union {
    Success s;
    Fail    f;
  };

  bool is_success;
};

template<class A>
bool Error<A>::is_error() const {
  if (!is_success) {
    return f.value != ErrorCode(); // ErrorCode() means success.
  }
  return false;
}

template<class A>
bool Error<A>::is_value() const {
  return is_success;
}

template<class A>
const A& Error<A>::value() const {
  return s.value;
}

template<class A>
typename Error<A>::ErrorCode Error<A>::error() const {
  if (!is_error()) { return ErrorCode(); }
  return f.value;
}

} // masio namespace

template<class A>
std::ostream& operator<<(std::ostream& os, const masio::Error<A>& ea) {
  if (ea.is_error()) {
    return os << "(Fail " << ea.error().message() << ")";
  }
  else {
    return os << "(Success " << ea.value() << ")";
  }
}

#endif // ifndef __MASIO_ERROR_H__

