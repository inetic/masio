#ifndef __MASIO_H__
#define __MASIO_H__

#include <boost/none.hpp>
#include <boost/asio.hpp>

namespace masio {

using boost::none;
using boost::none_t;

} // masio namespace

#include <masio/monad.h>

#include <masio/canceler.h>
#include <masio/error.h>
#include <masio/bind.h>
#include <masio/return.h>
#include <masio/action.h>
#include <masio/post.h>
#include <masio/fail.h>
#include <masio/wait.h>
#include <masio/pause.h>
#include <masio/may_fail.h>
#include <masio/all.h>
#include <masio/all_or_none.h>
#include <masio/with_canceler.h>

#include <masio/resolve.h>
#include <masio/connect.h>
#include <masio/accept.h>
#include <masio/send.h>
#include <masio/receive.h>

namespace masio {

template<typename MA>
action<typename MA::value_type> forever(const MA& ma) {
  using A = typename MA::value_type;
  return ma >= [ma](const A&) { return forever(ma); };
}

} // masio namespace

#endif // ifndef __MASIO_H__

