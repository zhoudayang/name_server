#include "name_server.h"
#include "command.h"

#include <boost/bind.hpp>
#include <muduo/base/Logging.h>

using namespace zy;

namespace
{
const char *findDCRLF(muduo::net::Buffer *buf)
{
  static const char kDCRLF[] = "\r\n\r\n";
  const char *dcrlf = std::search(buf->peek(), static_cast<const char *>(buf->beginWrite()), kDCRLF, kDCRLF + 4);
  return dcrlf == buf->beginWrite() ? NULL : dcrlf;
}
}

name_server::name_server(muduo::net::EventLoop *loop,
                         const muduo::net::InetAddress &addr,
                         const std::string &redis_ip,
                         uint16_t redis_port)
    : server_(loop, addr, "name_server"),
    command_factory_(new command_factory(this)),
    names_(),
    redis_cli_(redis_ip, redis_port)
{
  server_.setConnectionCallback(
      boost::bind(&name_server::onConnection, this, _1));
  server_.setMessageCallback(boost::bind(&name_server::onMessage, this, _1, _2, _3));
  restore_names();
}

void name_server::restore_names()
{
  auto reply = redis_cli_.execute("hgetall names");
  if (reply->type == REDIS_REPLY_ARRAY)
  {
    assert((reply->elements & 1) == 0);
    for (size_t i = 0; i < reply->elements; i += 2)
    {
      std::string name_str = reply->element[i]->str;
      std::string configurations = reply->element[i + 1]->str;
      name one(name_str);
      if (!one.parse(configurations))
      {
        LOG_DEBUG << name_str << " parse error!";
      }
      else
      {
        names_[name_str] = one;
      }
    }
  }
  else if (reply->type == REDIS_REPLY_ERROR)
  {
    LOG_DEBUG << "restore names error " << muduo::StringPiece(static_cast<const char *>(reply->str), reply->len);
  }
}

void name_server::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
  LOG_INFO << "name server - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort()
           << " is " << (conn->connected() ? "UP" : "DOWN");
}

void name_server::onMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buf,
                            muduo::Timestamp receiveTime)
{
  const char *dcrlf = nullptr;
  if ((dcrlf = findDCRLF(buf)) != nullptr)
  {
    std::string message(buf->peek(), dcrlf);
    buf->retrieveUntil(dcrlf);
    auto executor = command_factory_->get_executor(message);
    if (!executor)
    {
      conn->send("INVALID COMMAND\r\n\r\n");
    }
    else
    {
      conn->send(executor->execute());
    }
    conn->shutdown();
  }
}

void name_server::start()
{
  server_.start();
}