#include "name_server.h"
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>

using namespace zy;

int main()
{
  std::string redis_ip = "127.0.0.1";
  uint16_t redis_port = 6379;
  muduo::net::EventLoop loop;
  muduo::net::InetAddress listenAddr(8787);
  name_server server(&loop, listenAddr, redis_ip, redis_port);
  LOG_INFO << "name server is working now!";
  server.start();
  loop.loop();
}