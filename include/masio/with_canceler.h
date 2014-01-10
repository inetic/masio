#ifndef __MASIO_WITH_CANCELER_H__
#define __MASIO_WITH_CANCELER_H__

namespace masio {

template<class MA> class WithCanceler {
public:
  using CancelerPtr = std::shared_ptr<Canceler>;
  using value_type  = typename MA::value_type;

  //typedef Task<A>                     Super;
  //typedef typename Super::CancelerPtr CancelerPtr;
  //typedef typename Super::Rest        Rest;
  //typedef typename Super::Run         Run;
public:
  WithCanceler(const CancelerPtr& canceler, const MA& delegate)
    : _canceler(canceler)
    , _delegate(delegate)
  { }

  template<typename Rest>
  void run(const CancelerPtr& s, const Rest& rest) const {
    s->link_child_canceler(*_canceler);
    auto c = _canceler;
    _delegate.run(_canceler, [c,rest](const Error<value_type>& ea) {
        c->unlink();
        rest(ea);
        });
  }

private:
  CancelerPtr _canceler;
  MA          _delegate;
};

template<typename MA>
WithCanceler<MA>
with_canceler( const std::shared_ptr<Canceler>& canceler
             , const MA&                        delegate) {
  return WithCanceler<MA>(canceler, delegate);
}

} // masio namespace

#endif // ifndef __MASIO_WITH_CANCELER_H__
