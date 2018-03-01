#include "redis_cli.h"
#include <muduo/base/Logging.h>

#include <boost/bind.hpp>
using namespace zy;

namespace
{
void deleter(redisReply *ptr)
{
  freeReplyObject(ptr);
}
}

redis_cli::redis_cli(const std::string &ip, uint16_t port) :
    context_(nullptr)
{
  struct timeval timeout{2, 0};
  context_ = redisConnectWithTimeout(ip.c_str(), static_cast<int>(port), timeout);
  if ((nullptr == context_) || context_->err)
  {
    if (context_)
    {
      LOG_FATAL << "connect error: " << context_->errstr;
    }
    else
    {
      LOG_FATAL << "can not allocate redis context";
    }
  }
}

redis_cli::~redis_cli()
{
  if (context_)
  {
    redisFree(context_);
  }
}

std::unique_ptr<redisReply, boost::function<void(redisReply *)>> redis_cli::execute(const std::string &command)
{
  redisReply *reply = static_cast<redisReply *>(redisCommand(context_, command.c_str()));
  return std::unique_ptr<redisReply, boost::function<void(redisReply *)>>(reply, boost::bind(&deleter, _1));
}
