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
  Error(const Success& a) : _is_value(true) { new (&_value) A(a.value); }
  Error(const Fail&    a) : _is_value(false), _e(a.value) {}

  bool is_error() const;
  bool is_value() const;

  const A&  value() const;
  ErrorCode error() const;

  const A& operator*() const  { return value(); }
  const A* operator->() const { return &value(); }

  ~Error() {
    if (_is_value) {
      reinterpret_cast<A&>(_value).~A();
    }
  }
private:
  typedef typename std::aligned_storage<
    sizeof(A),
    std::alignment_of<A>::value>::type storage;

  bool _is_value;

  union {
    storage   _value;
    ErrorCode _e;
  };
};

template<class A>
bool Error<A>::is_error() const {
  if (!_is_value) {
    return _e != ErrorCode(); // ErrorCode() means success.
  }
  return false;
}

template<class A>
bool Error<A>::is_value() const {
  return _is_value;
}

template<class A>
const A& Error<A>::value() const {
  return reinterpret_cast<const A&>(_value);
}

template<class A>
typename Error<A>::ErrorCode Error<A>::error() const {
  if (!is_error()) { return ErrorCode(); }
  return _e;
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

