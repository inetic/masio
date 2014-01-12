#ifndef __MASIO_H__
#define __MASIO_H__

#include <boost/none.hpp>
#include <boost/asio.hpp>

namespace masio {

using boost::none;
using boost::none_t;

} // masio namespace

#include <masio/canceler.h>
#include <masio/error.h>
#include <masio/bind.h>
#include <masio/return.h>
#include <masio/task.h>
#include <masio/post.h>
#include <masio/fail.h>
#include <masio/wait.h>
#include <masio/pause.h>
#include <masio/may_fail.h>
#include <masio/all.h>
#include <masio/with_canceler.h>

#include <masio/connect.h>
#include <masio/accept.h>
#include <masio/send.h>
#include <masio/receive.h>

namespace masio {

template<typename MA>
task<typename MA::value_type> forever(const MA& ma) {
  using A = typename MA::value_type;
  return ma >= [ma](const A& a) { return forever(ma); };
}

} // masio namespace

#endif // ifndef __MASIO_H__

