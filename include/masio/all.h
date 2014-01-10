#ifndef __MASIO_ALL_H__
#define __MASIO_ALL_H__

namespace masio {

template<class MA, class MB> class All {
public:
  using CancelerPtr = std::shared_ptr<Canceler>;
  using A           = typename MA::value_type;
  using B           = typename MB::value_type;
  using value_type  = std::pair<Error<A>, Error<B>>;

  //typedef std::vector<Error<A>>       Result;
  //typedef Task<Result>                Super;
  //typedef typename Super::CancelerPtr CancelerPtr;
  //typedef typename Super::Rest        Rest;
  //typedef typename Super::Run         Run;
  //typedef typename Super::Ptr         Ptr;

  //typedef typename Task<A>::Ptr    SubPtr;

public:
  All(const MA& ma, const MB& mb)
    : ma(ma)
    , mb(mb)
  {}

  template<typename Rest>
  void run(const CancelerPtr& canceler, const Rest& rest) const {
    using namespace std;

    auto remaining = make_shared<size_t>(2);
    auto results   = make_shared<value_type>();

    ma.run(canceler, [remaining, results, rest](const Error<A>& ea) {
        results->first = ea;
        if(--*remaining == 0) {
          rest(typename Error<value_type>::Success{*results});
        }
        });

    mb.run(canceler, [remaining, results, rest](const Error<B>& eb) {
        results->second = eb;
        if(--*remaining == 0) {
          rest(typename Error<value_type>::Success{*results});
        }
        });
  }

private:
  MA ma;
  MB mb;
};

template<typename MA, typename MB>
All<MA, MB> all(const MA& ma, const MB& mb) {
  return All<MA, MB>(ma, mb);
}

} // masio namespace

#endif // ifndef __MASIO_ALL_H__

