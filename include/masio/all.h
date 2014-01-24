#ifndef __MASIO_ALL_H__
#define __MASIO_ALL_H__

namespace masio {

template<class MA, class MB> class All
  : public monad<All<MA, MB>, std::pair< result<typename MA::value_type>
                                       , result<typename MB::value_type>>> {
public:
  using A           = typename MA::value_type;
  using B           = typename MB::value_type;
  using value_type  = std::pair<result<A>, result<B>>;

public:
  All(const MA& ma, const MB& mb)
    : ma(ma)
    , mb(mb)
  {}

  template<typename Rest>
  void execute(Canceler& canceler, const Rest& rest) const {
    using namespace std;
    using Success = typename result<value_type>::Success;

    auto remaining = make_shared<size_t>(2);
    auto results   = make_shared<value_type>();

    ma.execute(canceler, [remaining, results, rest](const result<A>& ea) {
        results->first = ea;
        if(--*remaining == 0) {
          rest(Success{*results});
        }
        });

    mb.execute(canceler, [remaining, results, rest](const result<B>& eb) {
        results->second = eb;
        if(--*remaining == 0) {
          rest(Success{*results});
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

