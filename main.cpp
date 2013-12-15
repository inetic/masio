
#include <boost/asio.hpp>
#include <boost/variant.hpp>
#include <iostream>
#include <memory>

//
// Standard continuation signatures (r in our case is always void):
//
// Cont a : { (a -> r) -> r }
// Cont a >>= (\a -> Cont b) = Cont b
//
// But Cont is actually a Monad transformer parametrized
// over the Error monad. So the actual signature is:
//
// Cont a : { (E a -> r) -> r }
//

template<class A> void capture(const A&) {}
template<class> struct Lambda;

template<class A> struct Error
    : public boost::variant<A, boost::system::error_code>
{
  typedef boost::system::error_code    ErrorCode;
  typedef boost::variant<A, ErrorCode> Super;

  Error(const A& a) : Super(a) {}
  Error(ErrorCode e) : Super(e) {}

  bool is_error() const {
    if (const ErrorCode* pe = boost::get<ErrorCode>(this)) {
      return *pe != ErrorCode(); // ErrorCode() means success.
    }
    return false;
  }

  const A&         value() const { return boost::get<A>(*this); }
  const ErrorCode& error() const { return boost::get<ErrorCode>(*this); }
};

template<class A> struct Cont : public std::enable_shared_from_this<Cont<A>> {
  typedef std::function<void(Error<A>)>    Rest;
  typedef std::function<void(Rest)> Run;
  typedef std::shared_ptr<Cont<A>>  Ptr;

  virtual void run(const Rest& rest) const = 0;

  template<class B>
  typename Cont<B>::Ptr bind(std::function<typename Cont<B>::Ptr (A)> f) {
    using namespace std;
    typedef typename Cont<B>::Rest BRest;

    auto self = this->shared_from_this();

    return make_shared<Lambda<B>>(([self, f](BRest brest) /* -> void */ {
        self->run([brest,f, self](Error<A> ea) {
            if (!ea.is_error()) {
              f(ea.value())->run(brest);
            }
            else {
              brest(Error<B>(ea.error()));
            }
          });
        }));
  }
};

template<class A> struct Lambda : public Cont<A> {
  typedef typename Cont<A>::Rest  Rest;
  typedef typename Cont<A>::Run   Run;

  Lambda() {}
  Lambda(const Run& run) : _run(run) { }

  void run(const Rest& rest) const { _run(rest); }

  Run _run;
};

template<class A> struct Post : public Cont<A> {
  typedef Lambda<A>            Super;
  typedef typename Super::Rest Rest;
  typedef typename Super::Run  Run;

  Post(boost::asio::io_service& ios, Run r)
    : _run(r)
    , _io_service(ios) {}

  void run(const Rest& rest) const {
    auto self = this->shared_from_this();
    _io_service.post([=]() { capture(self); _run(rest); });
  }

  Run _run;
  boost::asio::io_service& _io_service;
};

template<class A>
std::shared_ptr<Post<A>> post( boost::asio::io_service& ios
                             , const typename Cont<A>::Run& run) {
  return std::make_shared<Post<A>>(ios, run);
}

template<class A> struct Return : public Cont<A> {
  typedef Cont<A>              Super;
  typedef typename Super::Rest Rest;
  typedef typename Super::Run  Run;

  Return(const A& a) : value(a) {}

  void run(const Rest& rest) const {
    rest(Error<A>(value));
  }

  A value;
};

template<class A> std::shared_ptr<Return<A>> success(const A& a) {
  using namespace std;
  return make_shared<Return<A>>(a);
}

template<class A>
std::shared_ptr<Lambda<A>> fail(const boost::system::error_code& error) {
  using namespace std;
  return make_shared<Lambda<A>>([error](const typename Cont<A>::Rest& rest){
      rest(Error<A>(error));
      });
}

using namespace std;
namespace asio = boost::asio;

int main() {
  asio::io_service ios;

  Cont<float>::Ptr p = post<int>(ios, [](Cont<int>::Rest rest) {
    rest(Error<int>(10));
  })
  ->bind<float>([&ios](int a) {
    //return fail<float>(boost::asio::error::operation_aborted);
    return post<float>(ios, [a](Cont<float>::Rest rest) {
      rest(Error<float>(2*a + 1));
      });
  })
  ->bind<float>([](float a) {
      return success(a+2);
  });

  p->run([](Error<float> i) { cout << "final: " << i << "\n"; });

  std::cout << "start\n";
  while(ios.poll_one()) {
    std::cout << "polled one\n";
  }
}

