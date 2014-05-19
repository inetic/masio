#ifndef __MASIO_H__
#define __MASIO_H__

#include <boost/asio.hpp>

#include <masio/tools.h>
#include <masio/monad.h>
#include <masio/result.h>
#include <masio/debug.h>

#include <masio/return.h>
#include <masio/bind.h>
#include <masio/fail.h>
#include <masio/action.h>
#include <masio/all.h>
#include <masio/may_fail.h>
#include <masio/all_or_none.h>

#include <masio/post.h>
#include <masio/wait.h>
#include <masio/pause.h>

#include <masio/resolve.h>
#include <masio/connect.h>
#include <masio/accept.h>
#include <masio/send.h>
#include <masio/receive.h>

namespace masio {

template<class MA>
action<> forever(const MA& ma) {
  return ma >= drop_args([ma]() {
      return forever(ma);
      });
}

} // masio namespace

#endif // ifndef __MASIO_H__

