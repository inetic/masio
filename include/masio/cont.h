#ifndef __MASIO_CONT_H__
#define __MASIO_CONT_H__

namespace masio {

template<class A> void capture(const A&) {}
template<class> struct Lambda;

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

} // masio namespace

#endif // ifndef __MASIO_CONT_H__
