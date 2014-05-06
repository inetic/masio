#ifndef __MASIO_WITH_CANCELER_H__
#define __MASIO_WITH_CANCELER_H__

namespace masio {

template<class MA> class WithCanceler
  : public monad<typename MA::value_type> {
public:
  using value_type  = typename MA::value_type;

public:
  WithCanceler(Canceler& canceler, const MA& delegate)
    : _canceler(canceler)
    , _delegate(delegate)
  { }

  template<typename Rest>
  void execute(Canceler& s, const Rest& rest) const {
    s.link_child_canceler(_canceler);
    auto& c = _canceler;
    _delegate.execute(_canceler, [&c,rest](const result<value_type>& ea) {
        c.unlink();
        rest(ea);
        });
  }

private:
  Canceler& _canceler;
  MA        _delegate;
};

template<typename MA>
WithCanceler<MA>
with_canceler( Canceler& canceler
             , const MA& delegate) {
  return WithCanceler<MA>(canceler, delegate);
}

} // masio namespace

#endif // ifndef __MASIO_WITH_CANCELER_H__
