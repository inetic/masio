#ifndef __MASIO_RESULT_H__
#define __MASIO_RESULT_H__

namespace masio {

namespace __detail {
  template<class... A> struct Success : std::tuple<A...> {
    Success(const A&... as) : std::tuple<A...>(as...) {}
    Success(const std::tuple<A...>& tuple) : std::tuple<A...>(tuple) {}
  };

  struct Fail    { boost::system::error_code value; };
}

template<class... A> struct result
{
  typedef __detail::Success<A...> Success;
  typedef __detail::Fail          Fail;

  typedef std::tuple<A...>          Tuple;
  typedef boost::system::error_code error_code;

  result() {}
  result(const Success& a) : _is_value(true) { new (&_value) Tuple(a); }
  result(const Fail&    a) : _is_value(false), _e(a.value) {}

  bool is_error() const;
  bool is_value() const;

  const Tuple& values() const;
  Tuple& values();

  template<int I>
  const typename std::tuple_element<I, Tuple>::type&
  value() const {
    return std::get<I>(values());
  }

  error_code error() const;

  const Tuple& operator*() const  { return values(); }
  const Tuple* operator->() const { return &values(); }

  ~result() {
    if (_is_value) {
      reinterpret_cast<Tuple&>(_value).~Tuple();
    }
  }
private:
  typedef typename std::aligned_storage<
    sizeof(Tuple),
    std::alignment_of<Tuple>::value>::type storage;

  bool _is_value;

  union {
    storage    _value;
    error_code _e;
  };
};

template<class... A>
bool result<A...>::is_error() const {
  if (!_is_value) {
    return _e != error_code(); // error_code() means success.
  }
  return false;
}

template<class... A>
bool result<A...>::is_value() const {
  return _is_value;
}

template<class... A>
const typename result<A...>::Tuple& result<A...>::values() const {
  return reinterpret_cast<const Tuple&>(_value);
}

template<class... A>
typename result<A...>::Tuple& result<A...>::values() {
  return reinterpret_cast<Tuple&>(_value);
}

template<class... A>
typename result<A...>::error_code result<A...>::error() const {
  if (!is_error()) { return error_code(); }
  return _e;
}

} // masio namespace

template<class A>
std::ostream& operator<<(std::ostream& os, const masio::result<A>& ea) {
  if (ea.is_error()) {
    return os << "(Fail " << ea.error().message() << ")";
  }
  else {
    return os << "(Success " << ea.value() << ")";
  }
}

#endif // ifndef __MASIO_RESULT_H__

