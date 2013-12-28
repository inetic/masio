#ifndef __MASIO_WITH_SUBSTATE_H__
#define __MASIO_WITH_SUBSTATE_H__

namespace masio {

template<class A> class WithSubstate : public Cont<A> {
public:
  typedef Cont<A>                  Super;
  typedef typename Super::StatePtr StatePtr;
  typedef typename Super::Rest     Rest;
  typedef typename Super::Run      Run;
public:
  WithSubstate(const StatePtr& substate, const typename Super::Ptr& delegate)
    : _substate(substate)
    , _delegate(delegate)
  { }

  void run(const StatePtr& s, const Rest& rest) const override {
    _delegate->run(_substate, rest);
  }

private:
  StatePtr            _substate;
  typename Super::Ptr _delegate;
};

template<class A>
typename WithSubstate<A>::Ptr
with_substate( const typename Cont<A>::StatePtr& substate
             , const typename Cont<A>::Ptr&      delegate) {
  return std::make_shared<WithSubstate<A>>(substate, delegate);
}

} // masio namespace

#endif // ifndef __MASIO_WITH_SUBSTATE_H__
