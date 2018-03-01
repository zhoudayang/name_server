#include <algorithm>
#include <vector>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <muduo/base/Logging.h>
#include "name.h"

using namespace zy;

name::name(const std::string &name) :
    name_(name),
    configurations_()
{

}

void name::add_configuration(const std::string &key, const std::string &value)
{
  configurations_.push_back(std::make_pair(key, value));
}

std::string name::get_response() const
{
  std::string response = "name:" + name_ + "\r\n";

  std::for_each(configurations_.begin(), configurations_.end(), [&response](const Key_value &configuration)
  {
    response += configuration.first + ":" + configuration.second + "\r\n";
  });
  response += "\r\n";

  return response;
}

bool name::set(const std::string &key, const std::string &value)
{
  auto it = std::find_if(configurations_.begin(), configurations_.end(), [&key](const Key_value &configuration)
  {
    return configuration.first == key;
  });
  if (it == configurations_.end())
  {
    return false;
  }
  it->second = value;
  return true;
}

bool name::parse(const std::string &fields)
{
  std::vector<std::string> configurations;
  boost::split_regex(configurations, fields, boost::regex("\r\n"));
  std::for_each(configurations.begin(), configurations.end(), [](std::string &configuration)
  {
    boost::trim(configuration);
  });
  {
    decltype(configurations) filter_configurations;
    std::remove_copy_if(configurations.begin(),
                        configurations.end(),
                        std::back_inserter(filter_configurations),
                        [](const std::string &configuration)
                        {
                          return (configuration.empty() || (configuration.find(':') == std::string::npos));
                        });
    configurations.swap(filter_configurations);
  }
  if (configurations.empty())
  {
    LOG_DEBUG << "parse get empty configurations";
  }
  std::vector<Key_value> key_values;
  std::transform(configurations.begin(), configurations.end(), std::back_inserter(key_values),
                 [](const std::string &configuration)
                 {
                   size_t pos = configuration.find(':');
                   return std::make_pair(configuration.substr(0, pos), configuration.substr(pos + 1));
                 });

  std::transform(key_values.begin(), key_values.end(), std::back_inserter(configurations_),
                 [](const Key_value &config)
                 {
                   return config;
                 });
  return true;
}

std::string name::get_field() const
{
  std::string field;

  std::for_each(configurations_.begin(), configurations_.end(), [&field](const Key_value &configuration)
  {
    field += configuration.first + ":" + configuration.second + "\r\n";
  });
  field += "\r\n";

  return field;
}

bool name::operator==(const name &one) const
{
  if (get_name() == one.get_name() && size() == one.size())
  {
    for (size_t i = 0; i < size(); ++i)
    {
      if (configurations_[i] != one.configurations_[i])
      {
        return false;
      }
    }
    return true;
  }
  return false;
}