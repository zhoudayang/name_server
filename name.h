#pragma once

#include <string>
#include <vector>

namespace zy
{
//! 要求在配置之中，没有字符 :
class name
{
 public:
  typedef std::pair<std::string, std::string> Key_value;

  name() = default;

  name(const std::string &name);

  std::string get_name() const { return name_; }

  void add_configuration(const std::string &key, const std::string &value);

  bool set(const std::string &key, const std::string &value);

  std::string get_response() const;

  std::string get_field() const;

  bool parse(const std::string &response);

  ~name() = default;

  bool empty() const { return configurations_.empty(); }

  bool operator==(const name &one) const;

  size_t size() const { return configurations_.size(); }
 private:

  std::string name_;
  std::vector<Key_value> configurations_;
};
}