#pragma once

#include <muduo/net/TcpServer.h>
#include <boost/noncopyable.hpp>
#include <string>
#include <map>

#include "name.h"
#include "redis_cli.h"

namespace zy
{
class command_factory;
class name_server : boost::noncopyable
{
 public:
  typedef boost::function<void(const std::string &)> Command_Callback;
  name_server(muduo::net::EventLoop *loop,
              const muduo::net::InetAddress &addr,
              const std::string &redis_ip,
              uint16_t redis_port);

  ~name_server() = default;

  void start();

  friend class get_command;
  friend class set_command;
  friend class del_command;
  friend class new_command;

 private:

  void onConnection(const muduo::net::TcpConnectionPtr &conn);

  void onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp receiveTime);

  void restore_names();

  muduo::net::TcpServer server_;
  std::shared_ptr<command_factory> command_factory_;
  std::map<std::string, name> names_;
  redis_cli redis_cli_;
};

}
