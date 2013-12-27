#ifndef __MASIO_STATE_H__
#define __MASIO_STATE_H__

#include <boost/intrusive/list.hpp>

namespace masio {

class State : public boost::intrusive::list_base_hook
                      <boost::intrusive::link_mode
                        <boost::intrusive::auto_unlink>> {
private:
  typedef boost::intrusive::list_base_hook
            <boost::intrusive::link_mode
              <boost::intrusive::auto_unlink>> auto_unlink_hook;

  typedef boost::intrusive::list<State, 
            boost::intrusive::constant_time_size<false>> Children;

  struct Canceler : public auto_unlink_hook {
    Canceler(const std::function<void()>& cancel)
      : cancel(cancel) {}

    void unlink() { auto_unlink_hook::unlink(); }

    std::function<void()> cancel;
  };

  typedef boost::intrusive::list<Canceler,
            boost::intrusive::constant_time_size<false>> Cancelers;
public:
  State() : _canceled(false) {}

  void link_canceler(Canceler& canceler) {
    _cancelers.push_back(canceler);
  }

  void cancel() {
    _canceled = true;

    for (auto& c : _cancelers) {
      c.cancel();
    }

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
  Cancelers  _cancelers;
};

} // masio namespace

#endif // ifndef __MASIO_STATE_H__
