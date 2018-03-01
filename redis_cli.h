#pragma once

#include <hiredis/hiredis.h>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

namespace zy
{
class redis_cli : boost::noncopyable
{
 public:
  // set timeout as 2s
  redis_cli(const std::string &ip, uint16_t port);

  std::unique_ptr<redisReply, boost::function<void(redisReply *)>> execute(const std::string &command);

  ~redis_cli();

 private:

  redisContext *context_;
};

}