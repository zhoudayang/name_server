#include "command.h"
#include "name_server.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <algorithm>
#include <map>

#include <muduo/base/Logging.h>

using namespace zy;

std::unique_ptr<command> command_factory::get_executor(const std::string &command_str)
{
  std::vector<std::string> command_lines;
  boost::split_regex(command_lines, command_str, boost::regex("\r\n"));
  std::for_each(command_lines.begin(), command_lines.end(), [](std::string &line)
  {
    boost::trim(line);
  });

  {
    std::vector<std::string> filter_command_lines;
    std::remove_copy_if(command_lines.begin(), command_lines.end(),
                        std::back_inserter(filter_command_lines),
                        [](const std::string &line)
                        {
                          return line.empty();
                        });
    command_lines.swap(filter_command_lines);
  }

  std::map<std::string, int> command_tuples
      {
          {"GET", GET},
          {"NEW", NEW},
          {"SET", SET},
          {"DEL", DEL}
      };
  std::string main_command = command_lines.front();
  command_lines.erase(command_lines.begin());

  std::unique_ptr<command> ret;
  // UNKNOWN
  if (!command_tuples.count(main_command))
  {
    return ret;
  }
  int command_type = command_tuples[main_command];
  switch (command_type)
  {
    case GET:ret.reset(new get_command(command_lines, server_));
      break;
    case SET:ret.reset(new set_command(command_lines, server_));
      break;
    case NEW:ret.reset(new new_command(command_lines, server_));
      break;
    case DEL:ret.reset(new del_command(command_lines, server_));
      break;
  }
  return ret;
}

std::vector<std::pair<std::string, std::string>> command::get_configurations()
{
  {
    std::vector<std::string> filter_key_values;
    std::remove_copy_if(key_values_.begin(),
                        key_values_.end(),
                        std::back_inserter(filter_key_values),
                        [](const std::string &line)
                        {
                          return line.find(":") == std::string::npos;
                        });
    key_values_.swap(filter_key_values);
  }
  std::vector<std::pair<std::string, std::string>> result;
  std::transform(key_values_.begin(), key_values_.end(), std::back_inserter(result), [](const std::string &config_line)
  {
    size_t pos = config_line.find(':');
    return std::make_pair(config_line.substr(0, pos), config_line.substr(pos + 1));
  });
  return result;
}

std::string command::get_key() const
{
  if (key_values_.size() != 1)
  {
    LOG_DEBUG << "key_values_ size is not 1";
  }
  return key_values_.front();
}

std::string del_command::execute()
{
  auto key = get_key();
  if (server_->names_.count(key))
  {
    boost::format fmt("HDEL names %1%");
    fmt % key;
    std::string command_str = fmt.str();
    auto reply = server_->redis_cli_.execute(command_str);
    if (reply->type == REDIS_REPLY_ERROR)
    {
      LOG_INFO << "fail to delete " << muduo::StringPiece(static_cast<const char *>(reply->str), reply->len);
      return "FAIL\r\n\r\n";
    }
    server_->names_.erase(key);
  }
  else
  {
    return "NOT EXISTED\r\n\r\n";
  }
  return "OK\r\n\r\n";
}

std::string get_command::execute()
{
  auto key = get_key();
  if (server_->names_.count(key))
  {
    std::string result = server_->names_[key].get_field();
    return result;
  }
  else
  {
    LOG_DEBUG << "no " << key << " in the names";
  }
  return "NONE\r\n\r\n";
}

std::string new_command::execute()
{
  auto configs = get_configurations();

  auto name_pair = configs.front();
  if (name_pair.first != "name")
  {
    LOG_DEBUG << "invalid name " << configs.front().first;
    return "INVALID\r\n\r\n";
  }
  else if (server_->names_.count(name_pair.second))
  {
    LOG_DEBUG << name_pair.second << " already in names_";
    return "EXISTED\r\n\r\n";
  }
  name one(name_pair.second);
  std::for_each(configs.begin() + 1, configs.end(), [&one](const std::pair<std::string, std::string> &pair)
  {
    one.add_configuration(pair.first, pair.second);
  });
  // empty configuration
  if (one.empty())
  {
    return "EMPTY\r\n\r\n";
  }

  auto field = one.get_field();
  boost::format fmt("HSET names %1% %2%");
  fmt % one.get_name() % field;

  auto reply = server_->redis_cli_.execute(fmt.str());
  if (reply->type == REDIS_REPLY_ERROR)
  {
    LOG_DEBUG << "fail to new " << muduo::StringPiece(static_cast<const char *>(reply->str), reply->len);
    return "FAIL\r\n\r\n";
  }

  server_->names_[one.get_name()] = one;
  return "OK\r\n\r\n";
}

std::string set_command::execute()
{
  auto configs = get_configurations();

  auto name_pair = configs.front();
  if (name_pair.first != "name")
  {
    LOG_DEBUG << "invalid name " << configs.front().first;
    return "INVALID\r\n\r\n";
  }
  else if (!server_->names_.count(name_pair.second))
  {
    LOG_DEBUG << name_pair.second << " already in names_";
    return "NOT EXISTED\r\n\r\n";
  }

  name one(name_pair.second);
  std::for_each(configs.begin() + 1, configs.end(), [&one](const std::pair<std::string, std::string> &pair)
  {
    one.add_configuration(pair.first, pair.second);
  });
  // empty configuration
  if (one.empty())
  {
    return "EMPTY\r\n\r\n";
  }

  if (one == server_->names_[one.get_name()])
  {
    return "OK\r\n\r\n";
  }

  auto field = one.get_field();
  boost::format fmt("HSET names %1% %2%");
  fmt % one.get_name() % field;

  auto reply = server_->redis_cli_.execute(fmt.str());
  if (reply->type == REDIS_REPLY_ERROR)
  {
    LOG_DEBUG << "fail to new " << muduo::StringPiece(static_cast<const char *>(reply->str), reply->len);
    return "FAIL\r\n\r\n";
  }

  server_->names_[one.get_name()] = one;
  return "OK\r\n\r\n";
}