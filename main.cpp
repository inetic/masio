
#include <boost/asio.hpp>
#include <iostream>
#include <memory>

// Cont a : { (a -> r) -> r }
// Cont a >>= (\a -> Cont b) = Cont b

template<class A> void capture(const A&) {}
template<class> struct Lambda;

template<class A> struct Cont : public std::enable_shared_from_this<Cont<A>> {
  typedef std::function<void(A)>    Rest;
  typedef std::function<void(Rest)> Run;
  typedef std::shared_ptr<Cont<A>>  Ptr;

  virtual void run(const Rest& rest) const = 0;

  template<class B>
  typename Cont<B>::Ptr bind(std::function<typename Cont<B>::Ptr (A)> f) {
    using namespace std;
    typedef typename Cont<B>::Rest BRest;

    auto self = this->shared_from_this();

    return make_shared<Lambda<B>>(([self, f](BRest brest) /* -> void */ {
        self->run([brest,f](A a) { return f(a)->run(brest); });
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
    rest(value);
  }

  A value;
};

template<class A> std::shared_ptr<Return<A>> success(const A& a) {
  using namespace std;
  return make_shared<Return<A>>(a);
}

using namespace std;
namespace asio = boost::asio;

int main() {
  asio::io_service ios;

  Cont<float>::Ptr p = post<int>(ios, [](Cont<int>::Rest rest) {
    rest(10);
  })
  ->bind<float>([&ios](int a) {
    return post<float>(ios, [a](Cont<int>::Rest rest) {
      rest(2*a + 1);
      });
  })
  ->bind<float>([](float a) {
      return success(a+2);
      //return make_shared<Lambda<float>>([a](Cont<float>::Rest rest){
      //  rest(a+1);
      //  });
        });

  p->run([](int i) { cout << "final: " << i << "\n"; });

  std::cout << "start\n";
  while(ios.poll_one()) {
    std::cout << "polled one\n";
  }
}

