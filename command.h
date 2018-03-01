#pragma once

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <memory>

namespace zy
{
class name_server;

class command
{
 public:
  command(const std::vector<std::string> &key_values, name_server *server) :
      key_values_(key_values),
      server_(server)
  {

  }

  virtual ~command()
  {

  }

  virtual std::string execute() = 0;

 protected:
  // for get and del
  std::vector<std::pair<std::string, std::string>> get_configurations();

  std::string get_key() const;

  std::vector<std::string> key_values_;
  name_server *server_;
};

class get_command : public command
{
 public:
  get_command(const std::vector<std::string> &key_values, name_server *server) :
      command(key_values, server)
  {

  }

  std::string execute() override;
};

class set_command : public command
{
 public:
  set_command(const std::vector<std::string> &key_values, name_server *server) :
      command(key_values, server)
  {

  }

  std::string execute() override;
};

class new_command : public command
{
 public:
  new_command(const std::vector<std::string> &key_values, name_server *server) :
      command(key_values, server)
  {

  }

  std::string execute() override;
};

class del_command : public command
{
 public:
  del_command(const std::vector<std::string> &key_values, name_server *server) :
      command(key_values, server)
  {

  }

  std::string execute() override;
};

class command_factory : boost::noncopyable
{
 public:
  enum
  {
    GET,
    SET,
    NEW,
    DEL,
    UNKNOWN
  } COMMAND_TYPE;

  command_factory(name_server *server) :
      server_(server)
  {

  }

  std::unique_ptr<command> get_executor(const std::string &command_str);

 private:
  name_server *server_;
};

}