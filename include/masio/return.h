#ifndef __MASIO_RETURN_H__
#define __MASIO_RETURN_H__

namespace masio {

template<class A> struct Return : public Cont<A> {
  typedef Cont<A>                     Super;
  typedef typename Super::CancelerPtr CancelerPtr;
  typedef typename Super::Rest        Rest;
  typedef typename Super::Run         Run;

  Return(const A& a) : value(a) {}

  void run(const CancelerPtr& canceler, const Rest& rest) const {
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
  typedef shared_ptr<Canceler> CancelerPtr;

  return make_shared<Lambda<A>>([error]( const CancelerPtr& canceler
                                       , const typename Cont<A>::Rest& rest){
      rest(typename Error<A>::Fail{error});
      });
}

} // masio namespace

#endif // ifndef __MASIO_RETURN_H__
