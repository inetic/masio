#ifndef __MASIO_CANCELER_H__
#define __MASIO_CANCELER_H__

#include <boost/intrusive/list.hpp>

namespace masio {

class Canceler : public boost::intrusive::list_base_hook
                      <boost::intrusive::link_mode
                        <boost::intrusive::auto_unlink>> {
private:
  typedef boost::intrusive::list_base_hook
            <boost::intrusive::link_mode
              <boost::intrusive::auto_unlink>> auto_unlink_hook;

  typedef boost::intrusive::list<Canceler, 
            boost::intrusive::constant_time_size<false>> Children;

  struct CancelAction : public auto_unlink_hook {
    CancelAction(const std::function<void()>& cancel)
      : cancel(cancel) {}

    void unlink() { auto_unlink_hook::unlink(); }

    std::function<void()> cancel;
  };

  typedef boost::intrusive::list<CancelAction,
            boost::intrusive::constant_time_size<false>> CancelActions;
public:
  Canceler() : _canceled(false) {}

  Canceler(const Canceler&) = delete;
  Canceler& operator=(const Canceler&) = delete;

  void link_cancel_action(CancelAction& cancel_action) {
    _cancel_actions.push_back(cancel_action);
  }

  void cancel() {
    if (_canceled) return;

    _canceled = true;

    for (auto& c : _cancel_actions) {
      c.cancel();
    }

    for (auto& c : _children) {
      c.cancel();
    }
  }

  bool canceled() const { return _canceled; }

  void link_child_canceler(Canceler& canceler) {
    if (canceler.is_linked()) {
      throw std::runtime_error("Canceler already linked");
    }
    _children.push_back(canceler);
  }

  void unlink() { auto_unlink_hook::unlink(); }

private:
  Children       _children;
  bool           _canceled;
  CancelActions  _cancel_actions;
};

} // masio namespace

#endif // ifndef __MASIO_CANCELER_H__
