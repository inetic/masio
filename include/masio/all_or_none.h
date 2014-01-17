#ifndef __MASIO_ALL_OR_NONE_H__
#define __MASIO_ALL_OR_NONE_H__

namespace masio {

template<class MA, class MB> class AllOrNone
  : public monad< AllOrNone<MA, MB>
                , std::pair< Error<typename MA::value_type>
                           , Error<typename MB::value_type>>> {
public:
  using A           = typename MA::value_type;
  using B           = typename MB::value_type;
  using value_type  = std::pair<Error<A>, Error<B>>;

public:
  AllOrNone(const MA& ma, const MB& mb)
    : ma(ma)
    , mb(mb)
  {}

  template<typename Rest>
  void run(Canceler& canceler, const Rest& rest) const {
    using namespace std;
    using error_code = boost::system::error_code;
    using Success = typename Error<value_type>::Success;
    using Fail    = typename Error<value_type>::Fail;

    struct Data {
      size_t     remaining;
      value_type results;
      error_code first_error;

      Data() : remaining(2) {}
    };

    auto data = make_shared<Data>();

    ma.run(canceler, [data, &canceler, rest](const Error<A>& ea) {
        data->results.first = ea;

        if (ea.is_error() && !data->first_error) {
          data->first_error = ea.error();
          canceler.cancel();
        }

        if(--data->remaining == 0) {
          rest(Success{data->results});
        }
        });

    mb.run(canceler, [data, &canceler, rest](const Error<B>& eb) {
        data->results.second = eb;

        if (eb.is_error() && !data->first_error) {
          data->first_error = eb.error();
          canceler.cancel();
        }

        if(--data->remaining == 0) {
          rest(Success{data->results});
        }
        });
  }

private:
  MA ma;
  MB mb;
};

template<typename MA, typename MB>
AllOrNone<MA, MB> all_or_none(const MA& ma, const MB& mb) {
  return AllOrNone<MA, MB>(ma, mb);
}

} // masio namespace

#endif // ifndef __MASIO_ALL_OR_NONE_H__

