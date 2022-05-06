/*!
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "ParserExpr.hpp"

std::string memberObjXml (const std::shared_ptr<ParserMemberObj> &exprAccessBody, std::size_t indent) {
  if (std::holds_alternative<Token>(*exprAccessBody)) {
    auto id = std::get<Token>(*exprAccessBody);
    return std::string(indent, ' ') + id.xml() + "\n";
  }

  auto member = std::get<ParserMember>(*exprAccessBody);
  auto result = std::string(indent, ' ') + R"(<slot name="obj">)" "\n";

  result += memberObjXml(member.obj, indent + 2);
  result += std::string(indent, ' ') + "</slot>\n";
  result += std::string(indent, ' ') + R"(<slot name="prop">)" "\n";
  result += std::string(indent + 2, ' ') + member.prop.xml() + "\n";
  result += std::string(indent, ' ') + "</slot>\n";

  return result;
}

std::string ParserExprAccess::xml (std::size_t indent) const {
  auto result = std::string();

  result += std::string(indent, ' ') + "<ExprAccess>\n";
  result += memberObjXml(this->body, indent + 2);
  result += std::string(indent, ' ') + "</ExprAccess>";

  return result;
}

std::string ParserStmtExpr::xml (std::size_t indent) const {
  auto result = std::string(indent, ' ') + "<StmtExpr";

  result += R"( parenthesized=")" + std::string(this->parenthesized ? "true" : "false");
  result += R"(" start=")" + this->start.str();
  result += R"(" end=")" + this->end.str() + R"(">)" "\n";

  indent += 2;

  if (std::holds_alternative<ParserExprAccess>(*this->body)) {
    auto exprAccess = std::get<ParserExprAccess>(*this->body);
    result += exprAccess.xml(indent) + "\n";
  } else if (std::holds_alternative<ParserExprAssign>(*this->body)) {
    auto exprAssign = std::get<ParserExprAssign>(*this->body);

    result += std::string(indent, ' ') + "<ExprAssign>\n";
    result += std::string(indent + 2, ' ') + R"(<slot name="left">)" "\n";
    result += exprAssign.left.xml(indent + 4) + "\n";
    result += std::string(indent + 2, ' ') + "</slot>\n";
    result += std::string(indent + 2, ' ') + R"(<slot name="op">)" "\n";
    result += std::string(indent + 4, ' ') + exprAssign.op.xml() + "\n";
    result += std::string(indent + 2, ' ') + "</slot>\n";
    result += std::string(indent + 2, ' ') + R"(<slot name="right">)" "\n";
    result += exprAssign.right.xml(indent + 4) + "\n";
    result += std::string(indent + 2, ' ') + "</slot>\n";
    result += std::string(indent, ' ') + "</ExprAssign>\n";
  } else if (std::holds_alternative<ParserExprBinary>(*this->body)) {
    auto exprBinary = std::get<ParserExprBinary>(*this->body);

    result += std::string(indent, ' ') + "<ExprBinary>\n";
    result += std::string(indent + 2, ' ') + R"(<slot name="left">)" "\n";
    result += exprBinary.left.xml(indent + 4) + "\n";
    result += std::string(indent + 2, ' ') + "</slot>\n";
    result += std::string(indent + 2, ' ') + R"(<slot name="op">)" "\n";
    result += std::string(indent + 4, ' ') + exprBinary.op.xml() + "\n";
    result += std::string(indent + 2, ' ') + "</slot>\n";
    result += std::string(indent + 2, ' ') + R"(<slot name="right">)" "\n";
    result += exprBinary.right.xml(indent + 4) + "\n";
    result += std::string(indent + 2, ' ') + "</slot>\n";
    result += std::string(indent, ' ') + "</ExprBinary>\n";
  } else if (std::holds_alternative<ParserExprCall>(*this->body)) {
    auto exprCall = std::get<ParserExprCall>(*this->body);

    result += std::string(indent, ' ') + "<ExprCall>\n";
    result += std::string(indent + 2, ' ') + R"(<slot name="callee">)" "\n";
    result += exprCall.callee.xml(indent + 4) + "\n";
    result += std::string(indent + 2, ' ') + "</slot>\n";

    if (!exprCall.args.empty()) {
      result += std::string(indent + 2, ' ') + R"(<slot name="args">)" "\n";

      for (const auto &exprCallArg : exprCall.args) {
        result += std::string(indent + 4, ' ') + "<ExprCallArg>\n";

        if (exprCallArg.id != std::nullopt) {
          result += std::string(indent + 6, ' ') + R"(<slot name="id">)" "\n";
          result += std::string(indent + 8, ' ') +  exprCallArg.id->xml()  + "\n";
          result += std::string(indent + 6, ' ') + "</slot>\n";
        }

        result += std::string(indent + 6, ' ') + R"(<slot name="expr">)" "\n";
        result += exprCallArg.expr.xml(indent + 8)  + "\n";
        result += std::string(indent + 6, ' ') + "</slot>\n";
        result += std::string(indent + 4, ' ') + "</ExprCallArg>\n";
      }

      result += std::string(indent + 2, ' ') + "</slot>\n";
    }

    result += std::string(indent, ' ') + "</ExprCall>\n";
  } else if (std::holds_alternative<ParserExprCond>(*this->body)) {
    auto exprCond = std::get<ParserExprCond>(*this->body);

    result += std::string(indent, ' ') + "<ExprCond>\n";
    result += std::string(indent + 2, ' ') + R"(<slot name="cond">)" "\n";
    result += exprCond.cond.xml(indent + 4) + "\n";
    result += std::string(indent + 2, ' ') + "</slot>\n";
    result += std::string(indent + 2, ' ') + R"(<slot name="body">)" "\n";
    result += exprCond.body.xml(indent + 4) + "\n";
    result += std::string(indent + 2, ' ') + "</slot>\n";
    result += std::string(indent + 2, ' ') + R"(<slot name="alt">)" "\n";
    result += exprCond.alt.xml(indent + 4) + "\n";
    result += std::string(indent + 2, ' ') + "</slot>\n";
    result += std::string(indent, ' ') + "</ExprCond>\n";
  } else if (std::holds_alternative<ParserExprLit>(*this->body)) {
    auto exprLit = std::get<ParserExprLit>(*this->body);

    result += std::string(indent, ' ') + "<ExprLit>\n";
    result += std::string(indent + 2, ' ') + exprLit.body.xml() + "\n";
    result += std::string(indent, ' ') + "</ExprLit>\n";
  } else if (std::holds_alternative<ParserExprObj>(*this->body)) {
    auto exprObj = std::get<ParserExprObj>(*this->body);

    result += std::string(indent, ' ') + "<ExprObj>\n";
    result += std::string(indent + 2, ' ') + R"(<slot name="id">)" "\n";
    result += std::string(indent + 4, ' ') + exprObj.id.xml() + "\n";
    result += std::string(indent + 2, ' ') + "</slot>\n";

    if (!exprObj.props.empty()) {
      result += std::string(indent + 2, ' ') + R"(<slot name="props">)" "\n";

      for (const auto &exprObjProp : exprObj.props) {
        result += std::string(indent + 4, ' ') + "<ExprObjProp>\n";
        result += std::string(indent + 6, ' ') + R"(<slot name="id">)" "\n";
        result += std::string(indent + 8, ' ') + exprObjProp.id.xml() + "\n";
        result += std::string(indent + 6, ' ') + "</slot>\n";
        result += std::string(indent + 6, ' ') + R"(<slot name="init">)" "\n";
        result += exprObjProp.init.xml(indent + 8) + "\n";
        result += std::string(indent + 6, ' ') + "</slot>\n";
        result += std::string(indent + 4, ' ') + "</ExprObjProp>\n";
      }

      result += std::string(indent + 2, ' ') + "</slot>\n";
    }

    result += std::string(indent, ' ') + "</ExprObj>\n";
  } else if (std::holds_alternative<ParserExprUnary>(*this->body)) {
    auto exprUnary = std::get<ParserExprUnary>(*this->body);

    result += std::string(indent, ' ') + R"(<ExprUnary prefix=")" + std::string(exprUnary.prefix ? "true" : "false") + R"(">)" "\n";
    result += std::string(indent + 2, ' ') + R"(<slot name="arg">)" "\n";
    result += exprUnary.arg.xml(indent + 4) + "\n";
    result += std::string(indent + 2, ' ') + "</slot>\n";
    result += std::string(indent + 2, ' ') + R"(<slot name="op">)" "\n";
    result += std::string(indent + 4, ' ') + exprUnary.op.xml() + "\n";
    result += std::string(indent + 2, ' ') + "</slot>\n";
    result += std::string(indent, ' ') + "</ExprUnary>\n";
  }

  indent -= 2;
  return result + std::string(indent, ' ') + "</StmtExpr>";
}
