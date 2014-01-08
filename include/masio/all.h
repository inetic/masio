#ifndef __MASIO_ALL_H__
#define __MASIO_ALL_H__

namespace masio {

template<class A> class All : public Task<std::vector<Error<A>>> {
public:
  typedef std::vector<Error<A>>       Result;
  typedef Task<Result>                Super;
  typedef typename Super::CancelerPtr CancelerPtr;
  typedef typename Super::Rest        Rest;
  typedef typename Super::Run         Run;
  typedef typename Super::Ptr         Ptr;

  typedef typename Task<A>::Ptr    SubPtr;

public:
  All(const SubPtr& c1, const SubPtr& c2)
    : _tasks{c1, c2}
  {}

  void run(const CancelerPtr& canceler, const Rest& rest) const override {
    using namespace std;

    auto self      = Super::shared_from_this();

    auto remaining = make_shared<size_t>(_tasks.size());
    auto results   = make_shared<Result>(*remaining);

    auto ci = _tasks.begin();
    auto ri = results->begin();

    for (; ci != _tasks.end(); ++ci, ++ri) {
      (*ci)->run(canceler, [self, remaining, results, ri, rest]
                           (const Error<A> e) {
          *ri = e;
          --*remaining;
          if (*remaining == 0) {
            rest(typename Error<Result>::Success{*results});
          }
          });
    }
  }

private:
  std::list<SubPtr> _tasks;
};

template<class A>
typename All<A>::Ptr all( const typename Task<A>::Ptr& a
                        , const typename Task<A>::Ptr& b) {
  return std::make_shared<All<A>>(a, b);
}

} // masio namespace

#endif // ifndef __MASIO_ALL_H__

