#ifndef __MASIO_WITH_CANCELER_H__
#define __MASIO_WITH_CANCELER_H__

namespace masio {

template<class A> class WithCanceler : public Cont<A> {
public:
  typedef Cont<A>                     Super;
  typedef typename Super::CancelerPtr CancelerPtr;
  typedef typename Super::Rest        Rest;
  typedef typename Super::Run         Run;
public:
  WithCanceler(const CancelerPtr& canceler, const typename Super::Ptr& delegate)
    : _canceler(canceler)
    , _delegate(delegate)
  { }

  void run(const CancelerPtr& s, const Rest& rest) const override {
    s->link_child_canceler(*_canceler);
    auto c = _canceler;
    _delegate->run(_canceler, [c,rest](const Error<A>& ea) {
        c->unlink();
        rest(ea);
        });
  }

private:
  CancelerPtr         _canceler;
  typename Super::Ptr _delegate;
};

template<class A>
typename WithCanceler<A>::Ptr
with_canceler( const typename Cont<A>::CancelerPtr& canceler
             , const typename Cont<A>::Ptr&      delegate) {
  return std::make_shared<WithCanceler<A>>(canceler, delegate);
}

} // masio namespace

#endif // ifndef __MASIO_WITH_CANCELER_H__
