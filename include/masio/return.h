#ifndef __MASIO_RETURN_H__
#define __MASIO_RETURN_H__

namespace masio {

template<class A> struct Return : public Cont<A> {
  typedef Cont<A>                  Super;
  typedef typename Super::StatePtr StatePtr;
  typedef typename Super::Rest     Rest;
  typedef typename Super::Run      Run;

  Return(const A& a) : value(a) {}

  void run(const StatePtr& state, const Rest& rest) const {
    if (state->canceled()) {
      rest(typename Error<A>::Fail{boost::asio::error::operation_aborted});
      return;
    }
    rest(typename Error<A>::Success{value});
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
  typedef shared_ptr<State> StatePtr;

  return make_shared<Lambda<A>>([error]( const StatePtr& state
                                       , const typename Cont<A>::Rest& rest){
      rest(typename Error<A>::Fail{error});
      });
}

} // masio namespace

#endif // ifndef __MASIO_RETURN_H__
