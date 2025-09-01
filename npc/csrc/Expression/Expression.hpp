#pragma once

#include <optional>
#include <string>
#include <string_view>
class Expression {
public:
  static std::optional<Expression> generateExpression(std::string_view expr);
  Expression(const Expression &);
  Expression(Expression &&);

private:
  Expression();

  struct OneToken {
    std::string display;
    int type;
    int catagory;
    int priority;
    union {
      long long value;
      int regid;
    } data;
  };
};