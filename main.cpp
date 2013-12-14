
#include <boost/asio.hpp>
#include <iostream>
#include <memory>

// Cont a : { (a -> r) -> r }
// Cont a >>= (\a -> Cont b) = Cont b

template<class A> struct Cont : public std::enable_shared_from_this<Cont<A>> {
  typedef std::function<void(A)>    Rest;
  typedef std::function<void(Rest)> Run;
  typedef std::shared_ptr<Cont<A>>  Ptr;

  Cont() {}
  Cont(const Run& run) : run(run) { }

  Run run;

  template<class B>
  typename Cont<B>::Ptr bind(std::function<typename Cont<B>::Ptr (A)> f) {
    using namespace std;
    typedef typename Cont<B>::Rest BRest;

    auto self = this->shared_from_this();

    return make_shared<Cont<B>>(([self, f](BRest brest) /* -> void */ {
        self->run([brest,f](A a) { return f(a)->run(brest); });
        }));
  }
};

template<class A> struct Post : public Cont<A> {
  typedef typename Cont<A>::Rest Rest;
  typedef typename Cont<A>::Run  Run;

  Post(boost::asio::io_service& ios, Run r)
    : Cont<A>([=](const Rest& rest) { post(r, rest); })
    , _io_service(ios) {}

  void post(const Run& run, const Rest& rest) const {
    _io_service.post([run, rest]() { run(rest); });
  }

  boost::asio::io_service& _io_service;
};

template<class A>
std::shared_ptr<Post<A>> post( boost::asio::io_service& ios
                             , const typename Cont<A>::Run& run) {
  return std::make_shared<Post<A>>(ios, run);
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
  });

  p->run([](int i) { cout << "final: " << i << "\n"; });

  std::cout << "start\n";
  while(ios.poll_one()) {
    std::cout << "polled one\n";
  }
}

