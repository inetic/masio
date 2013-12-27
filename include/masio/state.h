#ifndef __MASIO_STATE_H__
#define __MASIO_STATE_H__

#include <boost/intrusive/list.hpp>

namespace masio {

class State : public boost::intrusive::list_base_hook
                      <boost::intrusive::link_mode
                        <boost::intrusive::auto_unlink>> {

  typedef boost::intrusive::list<State, 
            boost::intrusive::constant_time_size<false>> Children;

public:
  State() : _canceled(false) {}

  void cancel() {
    _canceled = true;
    for (auto& c : _children) {
      c.cancel();
    }
  }

  bool canceled() const { return _canceled; }

  std::shared_ptr<State> make_substate() {
    auto substate = std::make_shared<State>();
    _children.push_back(*substate);
    return substate;
  }

private:
  Children   _children;
  bool       _canceled;
};

} // masio namespace

#endif // ifndef __MASIO_STATE_H__
