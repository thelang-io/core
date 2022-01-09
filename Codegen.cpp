/*!
 * Copyright (c) Aaron Delasy
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "Codegen.hpp"
#include "Error.hpp"

std::tuple<std::string, std::string, std::string> codegenNode (Codegen &, const ASTNode &);

std::string codegenName (const std::string &name) {
  return "__THE_" + name;
}

std::string codegenExprAccess (const ASTExprAccess &exprAccess) {
  if (std::holds_alternative<ASTId>(exprAccess)) {
    auto id = std::get<ASTId>(exprAccess);
    return codegenName(id.v.name);
  }

  auto member = std::get<ASTMember>(exprAccess);
  auto memberObj = codegenExprAccess(*member.obj);

  return memberObj + "->" + codegenName(member.prop);
}

std::string codegenBlock (Codegen &codegen, const ASTBlock &block) {
  auto setUp = std::string();
  auto code = std::string();
  auto cleanUp = std::string();

  codegen.indent += 2;

  for (const auto &node : block) {
    auto [nodeSetUp, nodeCode, nodeCleanUp] = codegenNode(codegen, node);

    setUp += nodeSetUp;
    code += nodeCode;
    cleanUp += nodeCleanUp;
  }

  codegen.indent -= 2;
  return setUp + code + (cleanUp.empty() ? "" : "\n" + cleanUp);
}

std::string codegenType (Codegen &codegen, const std::shared_ptr<Type> &type, bool mut) {
  if (type->isBool()) {
    codegen.headers.stdbool = true;
  }

  if (type->isBool()) return std::string(mut ? "" : "const ") + "bool ";
  if (type->isByte()) return std::string(mut ? "" : "const ") + "unsigned char ";
  if (type->isChar()) return std::string(mut ? "" : "const ") + "char ";
  if (type->isFloat()) return std::string(mut ? "" : "const ") + "double ";
  if (type->isF32()) return std::string(mut ? "" : "const ") + "float ";
  if (type->isF64()) return std::string(mut ? "" : "const ") + "double ";
  if (type->isInt()) return std::string(mut ? "" : "const ") + "long ";
  if (type->isI8()) return std::string(mut ? "" : "const ") + "char ";
  if (type->isI16()) return std::string(mut ? "" : "const ") + "short ";
  if (type->isI32()) return std::string(mut ? "" : "const ") + "long ";
  if (type->isI64()) return std::string(mut ? "" : "const ") + "long long ";
  if (type->isObj()) return std::string(mut ? "" : "const ") + "struct " + codegenName(type->name) + " *";
  if (type->isStr()) return std::string(mut ? "" : "const ") + "struct str *";
  if (type->isU8()) return std::string(mut ? "" : "const ") + "unsigned char ";
  if (type->isU16()) return std::string(mut ? "" : "const ") + "unsigned short ";
  if (type->isU32()) return std::string(mut ? "" : "const ") + "unsigned long ";
  if (type->isU64()) return std::string(mut ? "" : "const ") + "unsigned long long ";

  throw Error("Tried to code generate unknown type");
}

std::string codegenTypeFormat (const std::shared_ptr<Type> &type) {
  if (type->isBool()) return "%s";
  if (type->isByte()) return "%x";
  if (type->isChar()) return "%c";
  if (type->isFloat()) return "%f";
  if (type->isF32()) return "%f";
  if (type->isF64()) return "%f";
  if (type->isInt()) return "%ld";
  if (type->isI8()) return "%d";
  if (type->isI16()) return "%d";
  if (type->isI32()) return "%ld";
  if (type->isI64()) return "%lld";
  if (type->isFn()) return "%s";
  if (type->isObj()) return "%s";
  if (type->isStr()) return "%s";
  if (type->isU8()) return "%u";
  if (type->isU16()) return "%u";
  if (type->isU32()) return "%lu";
  if (type->isU64()) return "%llu";

  throw Error("Tried to code generate unknown type format");
}

std::tuple<std::string, std::string, std::string> codegenNodeExpr (Codegen &codegen, const ASTNodeExpr &nodeExpr) {
  auto setUp = std::string();
  auto code = std::string();
  auto cleanUp = std::string();

  if (nodeExpr.parenthesized) {
    code += "(";
  }

  if (std::holds_alternative<ASTExprAccess>(*nodeExpr.expr)) {
    code += codegenExprAccess(std::get<ASTExprAccess>(*nodeExpr.expr));
  } else if (std::holds_alternative<ASTExprAssign>(*nodeExpr.expr)) {
    auto exprAssign = std::get<ASTExprAssign>(*nodeExpr.expr);
    auto exprAccessLeft = codegenExprAccess(exprAssign.left);
    auto [rightNodeExprSetUp, rightNodeExprCode, rightNodeExprCleanUp] = codegenNodeExpr(codegen, exprAssign.right);

    if (exprAssign.op == AST_EXPR_ASSIGN_COALESCE) { // TODO logic
    } else if (exprAssign.op == AST_EXPR_ASSIGN_LOGICAL_AND) {
      code += exprAccessLeft + " = " + exprAccessLeft + " && " + rightNodeExprCode;
    } else if (exprAssign.op == AST_EXPR_ASSIGN_LOGICAL_OR) {
      code += exprAccessLeft + " = " + exprAccessLeft + " || " + rightNodeExprCode;
    } else if (exprAssign.op == AST_EXPR_ASSIGN_POWER) {
      codegen.headers.math = true;
      code += exprAccessLeft + " = pow(" + exprAccessLeft + ", " + rightNodeExprCode + ")";
    } else {
      code += exprAccessLeft;

      if (exprAssign.op == AST_EXPR_ASSIGN_ADD) code += " += ";
      if (exprAssign.op == AST_EXPR_ASSIGN_BITWISE_AND) code += " &= ";
      if (exprAssign.op == AST_EXPR_ASSIGN_BITWISE_OR) code += " |= ";
      if (exprAssign.op == AST_EXPR_ASSIGN_BITWISE_XOR) code += " ^= ";
      if (exprAssign.op == AST_EXPR_ASSIGN_COALESCE) code += " ?\?= ";
      if (exprAssign.op == AST_EXPR_ASSIGN_DIVIDE) code += " /= ";
      if (exprAssign.op == AST_EXPR_ASSIGN_EQUAL) code += " = ";
      if (exprAssign.op == AST_EXPR_ASSIGN_LEFT_SHIFT) code += " <<= ";
      if (exprAssign.op == AST_EXPR_ASSIGN_MULTIPLY) code += " *= ";
      if (exprAssign.op == AST_EXPR_ASSIGN_REMAINDER) code += " %= ";
      if (exprAssign.op == AST_EXPR_ASSIGN_RIGHT_SHIFT) code += " >>= ";
      if (exprAssign.op == AST_EXPR_ASSIGN_SUBTRACT) code += " -= ";

      code += rightNodeExprCode;
    }

    setUp += rightNodeExprSetUp;
    cleanUp += rightNodeExprCleanUp;
  } else if (std::holds_alternative<ASTExprBinary>(*nodeExpr.expr)) {
    auto exprBinary = std::get<ASTExprBinary>(*nodeExpr.expr);
    auto [leftNodeExprSetUp, leftNodeExprCode, leftNodeExprCleanUp] = codegenNodeExpr(codegen, exprBinary.left);
    auto [rightNodeExprSetUp, rightNodeExprCode, rightNodeExprCleanUp] = codegenNodeExpr(codegen, exprBinary.right);

    if (exprBinary.op == AST_EXPR_BINARY_COALESCE) { // TODO logic
    } else if (exprBinary.op == AST_EXPR_BINARY_POWER) {
      codegen.headers.math = true;
      code += "pow(" + leftNodeExprCode + ", " + rightNodeExprCode + ")";
    } else {
      if (exprBinary.op == AST_EXPR_BINARY_DIVIDE || exprBinary.op == AST_EXPR_BINARY_MULTIPLY) {
        code += "(double) " + leftNodeExprCode;
      } else {
        code += leftNodeExprCode;
      }

      if (exprBinary.op == AST_EXPR_BINARY_ADD) code += " + ";
      if (exprBinary.op == AST_EXPR_BINARY_BITWISE_AND) code += " & ";
      if (exprBinary.op == AST_EXPR_BINARY_BITWISE_OR) code += " | ";
      if (exprBinary.op == AST_EXPR_BINARY_BITWISE_XOR) code += " ^ ";
      if (exprBinary.op == AST_EXPR_BINARY_DIVIDE) code += " / ";
      if (exprBinary.op == AST_EXPR_BINARY_EQUAL) code += " == ";
      if (exprBinary.op == AST_EXPR_BINARY_GREATER_EQUAL) code += " >= ";
      if (exprBinary.op == AST_EXPR_BINARY_GREATER_THAN) code += " > ";
      if (exprBinary.op == AST_EXPR_BINARY_LEFT_SHIFT) code += " << ";
      if (exprBinary.op == AST_EXPR_BINARY_LESS_EQUAL) code += " <= ";
      if (exprBinary.op == AST_EXPR_BINARY_LESS_THAN) code += " < ";
      if (exprBinary.op == AST_EXPR_BINARY_LOGICAL_AND) code += " && ";
      if (exprBinary.op == AST_EXPR_BINARY_LOGICAL_OR) code += " || ";
      if (exprBinary.op == AST_EXPR_BINARY_MULTIPLY) code += " * ";
      if (exprBinary.op == AST_EXPR_BINARY_NOT_EQUAL) code += " != ";
      if (exprBinary.op == AST_EXPR_BINARY_REMAINDER) code += " % ";
      if (exprBinary.op == AST_EXPR_BINARY_RIGHT_SHIFT) code += " >> ";
      if (exprBinary.op == AST_EXPR_BINARY_SUBTRACT) code += " - ";

      if (exprBinary.op == AST_EXPR_BINARY_DIVIDE || exprBinary.op == AST_EXPR_BINARY_MULTIPLY) {
        code += "(double) " + rightNodeExprCode;
      } else {
        code += rightNodeExprCode;
      }
    }

    setUp += leftNodeExprSetUp;
    cleanUp += leftNodeExprCleanUp;
    setUp += rightNodeExprSetUp;
    cleanUp += rightNodeExprCleanUp;
  } else if (std::holds_alternative<ASTExprCall>(*nodeExpr.expr)) {
    auto exprCall = std::get<ASTExprCall>(*nodeExpr.expr);

    if (exprCall.fn->builtin) {
      if (exprCall.fn->name == "print") {
        codegen.headers.stdio = true;

        auto separator = std::string(R"(" ")");
        auto terminator = std::string(R"("\n")");
        auto argsCode = std::string();
        auto argIdx = static_cast<std::size_t>(0);

        for (const auto &exprCallArg : exprCall.args) {
          if (exprCallArg.id != std::nullopt && exprCallArg.id == "separator") {
            auto [nodeExprSetUp, nodeExprCode, nodeExprCleanUp] = codegenNodeExpr(codegen, exprCallArg.expr);

            separator = nodeExprCode + "->c";
            setUp += nodeExprSetUp;
            cleanUp += nodeExprCleanUp;
          } else if (exprCallArg.id != std::nullopt && exprCallArg.id == "terminator") {
            auto [nodeExprSetUp, nodeExprCode, nodeExprCleanUp] = codegenNodeExpr(codegen, exprCallArg.expr);

            terminator = nodeExprCode + "->c";
            setUp += nodeExprSetUp;
            cleanUp += nodeExprCleanUp;
          }
        }

        code += "printf(\"";

        for (const auto &exprCallArg : exprCall.args) {
          if (exprCallArg.id != std::nullopt) {
            continue;
          }

          code += argIdx == 0 ? "" : "%s";
          code += codegenTypeFormat(exprCallArg.expr.type);
          argsCode += argIdx == 0 ? "" : separator + ", ";

          if (exprCallArg.expr.type->isBool()) {
            argsCode += "(";
          }

          auto [nodeExprSetUp, nodeExprCode, nodeExprCleanUp] = codegenNodeExpr(codegen, exprCallArg.expr);
          argsCode += nodeExprCode;

          if (exprCallArg.expr.type->isBool()) {
            argsCode += R"() == true ? "true" : "false")";
          } else if (exprCallArg.expr.type->isStr()) {
            argsCode += "->c";
          }

          argsCode += ", ";
          setUp += nodeExprSetUp;
          cleanUp += nodeExprCleanUp;

          argIdx++;
        }

        code += R"(%s", )" + argsCode + terminator + ")";
      }
    } else {
      auto paramIdx = static_cast<std::size_t>(0);
      code += codegenName(exprCall.fn->name) + "(";

      for (const auto &[exprCallFnParamName, exprCallFnParam] : exprCall.fn->params) {
        auto foundArg = std::optional<ASTExprCallArg>{};

        if (paramIdx < exprCall.args.size() && exprCall.args[paramIdx].id == std::nullopt) {
          foundArg = exprCall.args[paramIdx];
        } else {
          for (const auto &exprCallArg : exprCall.args) {
            if (exprCallArg.id == exprCallFnParamName) {
              foundArg = exprCallArg;
              break;
            }
          }
        }

        code += paramIdx == 0 ? "" : ", ";

        if (foundArg == std::nullopt) {
          if (exprCallFnParam.type->isBool()) {
            code += "false";
          } else if (exprCallFnParam.type->isChar()) {
            code += R"('\0')";
          } else if (exprCallFnParam.type->isStr()) {
            code += R"("")";
          } else {
            code += "0";
          }
        } else {
          auto [nodeExprSetUp, nodeExprCode, nodeExprCleanUp] = codegenNodeExpr(codegen, foundArg->expr);

          code += nodeExprCode;
          setUp += nodeExprSetUp;
          cleanUp += nodeExprCleanUp;
        }

        paramIdx++;
      }

      code += ")";
    }
  } else if (std::holds_alternative<ASTExprCond>(*nodeExpr.expr)) {
    auto exprCond = std::get<ASTExprCond>(*nodeExpr.expr);
    auto [condNodeExprSetUp, condNodeExprCode, condNodeExprCleanUp] = codegenNodeExpr(codegen, exprCond.cond);
    auto [bodyNodeExprSetUp, bodyNodeExprCode, bodyNodeExprCleanUp] = codegenNodeExpr(codegen, exprCond.body);
    auto [altNodeExprSetUp, altNodeExprCode, altNodeExprCleanUp] = codegenNodeExpr(codegen, exprCond.alt);

    code += condNodeExprCode;
    code += " ? ";
    code += bodyNodeExprCode;
    code += " : ";
    code += altNodeExprCode;

    setUp += condNodeExprSetUp;
    setUp += bodyNodeExprSetUp;
    setUp += altNodeExprSetUp;
    cleanUp += condNodeExprCleanUp;
    cleanUp += bodyNodeExprCleanUp;
    cleanUp += altNodeExprCleanUp;
  } else if (std::holds_alternative<ASTExprLit>(*nodeExpr.expr)) {
    auto exprLit = std::get<ASTExprLit>(*nodeExpr.expr);

    if (exprLit.type == AST_EXPR_LIT_INT_OCT) {
      auto val = exprLit.val;

      val.erase(std::remove(val.begin(), val.end(), 'O'), val.end());
      val.erase(std::remove(val.begin(), val.end(), 'o'), val.end());

      code += val;
    } else if (exprLit.type == AST_EXPR_LIT_STR) {
      codegen.functions.str_from_cstr = true;
      auto valSize = std::to_string(exprLit.val.size() - 2);

      code += "str_from_cstr(" + exprLit.val + ", " + valSize + ")";
    } else {
      code += exprLit.val;
    }
  } else if (std::holds_alternative<ASTExprObj>(*nodeExpr.expr)) {
    auto exprObj = std::get<ASTExprObj>(*nodeExpr.expr);
    auto fieldIdx = static_cast<std::size_t>(0);

    code += codegenName(exprObj.obj->name) + "_init((struct " + codegenName(exprObj.obj->name) + ") {";

    for (const auto &[objFieldName, objField] : exprObj.obj->fields) {
      if (!exprObj.props.contains(objFieldName)) {
        continue;
      }

      auto init = exprObj.props[objFieldName];
      auto [initNodeExprSetUp, initNodeExprCode, initNodeExprCleanUp] = codegenNodeExpr(codegen, init);

      code += fieldIdx == 0 ? "" : ", ";
      code += initNodeExprCode;

      setUp += initNodeExprSetUp;
      cleanUp += initNodeExprCleanUp;

      fieldIdx++;
    }

    code += "})";
  } else if (std::holds_alternative<ASTExprUnary>(*nodeExpr.expr)) {
    auto exprUnary = std::get<ASTExprUnary>(*nodeExpr.expr);
    auto [argNodeExprSetUp, argNodeExprCode, argNodeExprCleanUp] = codegenNodeExpr(codegen, exprUnary.arg);

    if (!exprUnary.prefix) {
      code += argNodeExprCode;
    }

    if (exprUnary.op == AST_EXPR_UNARY_BITWISE_NOT) code += "~";
    if (exprUnary.op == AST_EXPR_UNARY_DECREMENT) code += "--";
    if (exprUnary.op == AST_EXPR_UNARY_DOUBLE_LOGICAL_NOT) code += "!!";
    if (exprUnary.op == AST_EXPR_UNARY_INCREMENT) code += "++";
    if (exprUnary.op == AST_EXPR_UNARY_LOGICAL_NOT) code += "!";
    if (exprUnary.op == AST_EXPR_UNARY_NEGATION) code += "-";
    if (exprUnary.op == AST_EXPR_UNARY_PLUS) code += "+";

    if (exprUnary.prefix) {
      code += argNodeExprCode;
    }

    setUp += argNodeExprSetUp;
    cleanUp += argNodeExprCleanUp;
  }

  if (nodeExpr.parenthesized) {
    code += ")";
  }

  return std::make_tuple(setUp, code, cleanUp);
}

std::string codegenNodeIf (Codegen &codegen, const ASTNodeIf &nodeIf) {
  auto code = std::string();
  auto [nodeExprSetUp, nodeExprCode, nodeExprCleanUp] = codegenNodeExpr(codegen, nodeIf.cond);

  code += "if (" + nodeExprCode + nodeExprSetUp + ") {\n";
  code += codegenBlock(codegen, nodeIf.body);
  code += nodeExprCleanUp;

  if (nodeIf.alt != std::nullopt) {
    code += std::string(codegen.indent, ' ') + "} else ";

    if (std::holds_alternative<ASTBlock>(**nodeIf.alt)) {
      code += "{\n";
      code += codegenBlock(codegen, std::get<ASTBlock>(**nodeIf.alt));
      code += std::string(codegen.indent, ' ') + "}";
    } else if (std::holds_alternative<ASTNodeIf>(**nodeIf.alt)) {
      code += codegenNodeIf(codegen, std::get<ASTNodeIf>(**nodeIf.alt));
    }
  } else {
    code += std::string(codegen.indent, ' ') + "}";
  }

  return code;
}

std::tuple<std::string, std::string, std::string> codegenNode (Codegen &codegen, const ASTNode &node) {
  auto setUp = std::string();
  auto code = std::string();
  auto cleanUp = std::string();

  if (std::holds_alternative<ASTNodeBreak>(node)) {
    code += std::string(codegen.indent, ' ') + "break;\n";
  } else if (std::holds_alternative<ASTNodeContinue>(node)) {
    code += std::string(codegen.indent, ' ') + "continue;\n";
  } else if (std::holds_alternative<ASTNodeExpr>(node)) {
    auto nodeExpr = std::get<ASTNodeExpr>(node);
    auto [nodeExprSetUp, nodeExprCode, nodeExprCleanUp] = codegenNodeExpr(codegen, nodeExpr);

    setUp += nodeExprSetUp;
    code += std::string(codegen.indent, ' ') + nodeExprCode + ";\n";
    cleanUp += nodeExprCleanUp;
  } else if (std::holds_alternative<ASTNodeFnDecl>(node)) {
    auto nodeFnDecl = std::get<ASTNodeFnDecl>(node);
    auto nodeFnDeclSetUp = std::string();
    auto nodeFnDeclCode = std::string();
    auto nodeFnDeclCleanUp = std::string();

    codegen.functionDeclarationsCode += codegenType(codegen, nodeFnDecl.fn->returnType, true);
    codegen.functionDeclarationsCode += codegenName(nodeFnDecl.fn->name) + " (";

    if (nodeFnDecl.fn->params.empty()) {
      codegen.functionDeclarationsCode += "void";
    } else {
      auto paramsCode = std::string();
      auto paramIdx = static_cast<std::size_t>(0);

      for (const auto &[_, nodeFnDeclFnParam] : nodeFnDecl.fn->params) {
        auto paramType = codegenType(codegen, nodeFnDeclFnParam.type, true);

        paramsCode += paramIdx == 0 ? "" : ", ";
        paramsCode += paramType.back() == ' ' ? paramType.substr(0, paramType.length() - 1) : paramType;
        paramIdx++;
      }

      codegen.functionDeclarationsCode += paramsCode;
    }

    codegen.functionDeclarationsCode += ");\n";

    nodeFnDeclCode += codegenType(codegen, nodeFnDecl.fn->returnType, true);
    nodeFnDeclCode += codegenName(nodeFnDecl.fn->name) + " (";

    if (nodeFnDecl.fn->params.empty()) {
      nodeFnDeclCode += ") {\n";
    } else {
      auto paramsCode = std::string();
      auto paramIdx = static_cast<std::size_t>(0);

      for (const auto &[nodeFnDeclFnParamName, nodeFnDeclFnParam] : nodeFnDecl.fn->params) {
        paramsCode += paramIdx == 0 ? "" : ", ";
        paramsCode += codegenType(codegen, nodeFnDeclFnParam.type, false);
        paramsCode += codegenName(nodeFnDeclFnParamName);

        paramIdx++;
      }

      nodeFnDeclCode += paramsCode + ") {\n";
    }

    nodeFnDeclCode += nodeFnDeclSetUp;

    auto prevIndent = codegen.indent;
    codegen.indent = 0;

    nodeFnDeclCode += codegenBlock(codegen, nodeFnDecl.body);
    codegen.indent = prevIndent;

    nodeFnDeclCode += nodeFnDeclCleanUp;
    nodeFnDeclCode += "}\n\n";

    codegen.functionDefinitionsCode += nodeFnDeclCode;
  } else if (std::holds_alternative<ASTNodeIf>(node)) {
    auto nodeIf = std::get<ASTNodeIf>(node);

    code += std::string(codegen.indent, ' ');
    code += codegenNodeIf(codegen, nodeIf);
    code += "\n";
  } else if (std::holds_alternative<ASTNodeLoop>(node)) {
    auto nodeLoop = std::get<ASTNodeLoop>(node);
    auto nodeLoopCleanUp = std::string();

    code += std::string(codegen.indent, ' ');

    if (nodeLoop.init == std::nullopt && nodeLoop.cond == std::nullopt && nodeLoop.upd == std::nullopt) {
      code += "while (1)";
    } else if (nodeLoop.init == std::nullopt && nodeLoop.upd == std::nullopt) {
      auto [nodeExprSetUp, nodeExprCode, nodeExprCleanUp] = codegenNodeExpr(codegen, *nodeLoop.cond);

      code += "while (";
      code += nodeExprSetUp;
      code += nodeExprCode;
      code += ")";

      nodeLoopCleanUp += nodeExprCleanUp;
    } else {
      code += "for (";

      if (nodeLoop.init != std::nullopt) {
        auto prevIndent = codegen.indent;
        codegen.indent = 0;

        auto [nodeSetUp, nodeCode, nodeCleanUp] = codegenNode(codegen, **nodeLoop.init);
        codegen.indent = prevIndent;

        code += nodeSetUp;
        code += nodeCode.substr(0, nodeCode.length() - 2);
        nodeLoopCleanUp += nodeCleanUp;
      }

      code += ";";

      if (nodeLoop.cond != std::nullopt) {
        auto [nodeExprSetUp, nodeExprCode, nodeExprCleanUp] = codegenNodeExpr(codegen, *nodeLoop.cond);

        code += " " + nodeExprSetUp + nodeExprCode;
        nodeLoopCleanUp += nodeExprCleanUp;
      }

      code += ";";

      if (nodeLoop.upd != std::nullopt) {
        auto [nodeExprSetUp, nodeExprCode, nodeExprCleanUp] = codegenNodeExpr(codegen, *nodeLoop.upd);

        code += " " + nodeExprSetUp + nodeExprCode;
        nodeLoopCleanUp += nodeExprCleanUp;
      }

      code += ")";
    }

    if (nodeLoop.body.empty() && nodeLoopCleanUp.empty()) {
      code += ";\n";
    } else {
      code += " {\n";
      code += codegenBlock(codegen, nodeLoop.body);
      code += nodeLoopCleanUp;
      code += std::string(codegen.indent, ' ') + "}\n";
    }
  } else if (std::holds_alternative<ASTNodeMain>(node)) {
    auto nodeMain = std::get<ASTNodeMain>(node);
    code += codegenBlock(codegen, nodeMain.body);
  } else if (std::holds_alternative<ASTNodeObjDecl>(node)) {
    codegen.headers.stdio = true;
    codegen.headers.stdlib = true;
    codegen.headers.string = true;

    auto nodeObjDecl = std::get<ASTNodeObjDecl>(node);
    auto nodeObjDeclCode = std::string();

    codegen.structDeclarationsCode += "struct " + codegenName(nodeObjDecl.obj->name) + ";\n";
    codegen.structDefinitionsCode += "struct " + codegenName(nodeObjDecl.obj->name) + " {\n";

    for (const auto &[nodeObjDeclObjFieldName, nodeObjDeclObjFieldType] : nodeObjDecl.obj->fields) {
      codegen.structDefinitionsCode += "  " + codegenType(codegen, nodeObjDeclObjFieldType, true);
      codegen.structDefinitionsCode += codegenName(nodeObjDeclObjFieldName) + ";\n";
    }

    codegen.structDefinitionsCode += "};\n\n";

    codegen.functionDeclarationsCode += "struct " + codegenName(nodeObjDecl.obj->name) + " *";
    codegen.functionDeclarationsCode += codegenName(nodeObjDecl.obj->name) + "_init ";
    codegen.functionDeclarationsCode += "(struct " + codegenName(nodeObjDecl.obj->name) + ");\n";

    nodeObjDeclCode += "struct " + codegenName(nodeObjDecl.obj->name) + " *";
    nodeObjDeclCode += codegenName(nodeObjDecl.obj->name) + "_init ";
    nodeObjDeclCode += "(struct " + codegenName(nodeObjDecl.obj->name) + " x) {\n";
    nodeObjDeclCode += "  size_t l = sizeof(struct " + codegenName(nodeObjDecl.obj->name) + ");\n";
    nodeObjDeclCode += "  " + codegenType(codegen, nodeObjDecl.obj, true) + "n = malloc(l);\n";
    nodeObjDeclCode += "  if (n == NULL) {\n";
    nodeObjDeclCode += R"(    fprintf(stderr, "Error: Failed to allocate %zu bytes for object \")";
    nodeObjDeclCode += nodeObjDecl.name + R"(\"\n", l);)" + "\n";
    nodeObjDeclCode += "    exit(EXIT_FAILURE);\n";
    nodeObjDeclCode += "  }\n";
    nodeObjDeclCode += "  memcpy(n, &x, l);\n";
    nodeObjDeclCode += "  return n;\n";
    nodeObjDeclCode += "}\n\n";

    codegen.functionDefinitionsCode += nodeObjDeclCode;
  } else if (std::holds_alternative<ASTNodeReturn>(node)) {
    auto nodeReturn = std::get<ASTNodeReturn>(node);
    auto [nodeExprSetUp, nodeExprCode, nodeExprCleanUp] = codegenNodeExpr(codegen, nodeReturn.arg);

    code += std::string(codegen.indent, ' ') + nodeExprSetUp;
    code += "return " + nodeExprCode + ";\n";
    code += nodeExprCleanUp;
  } else if (std::holds_alternative<ASTNodeVarDecl>(node)) {
    auto nodeVarDecl = std::get<ASTNodeVarDecl>(node);

    code += std::string(codegen.indent, ' ');
    code += codegenType(codegen, nodeVarDecl.v.type, nodeVarDecl.v.mut) + codegenName(nodeVarDecl.v.name) + " = ";

    if (nodeVarDecl.init == std::nullopt) {
      if (nodeVarDecl.v.type->isBool()) {
        codegen.headers.stdbool = true;
        code += "false";
      } else if (nodeVarDecl.v.type->isChar()) {
        code += "'\\0'";
      } else if (nodeVarDecl.v.type->isStr()) {
        codegen.functions.str_init = true;
        code += "str_init(0)";
      } else {
        code += "0";
      }
    } else if (std::holds_alternative<ASTExprAccess>(*nodeVarDecl.init->expr)) {
      auto exprAccess = std::get<ASTExprAccess>(*nodeVarDecl.init->expr);

      if (std::holds_alternative<ASTId>(exprAccess)) {
        auto id = std::get<ASTId>(exprAccess);

        if (id.v.type->isStr()) {
          codegen.functions.str_clone = true;
          code += "str_clone(" + codegenName(id.v.name) + ")";
        } else {
          code += codegenName(id.v.name);
        }
      }
    } else if (std::holds_alternative<ASTExprLit>(*nodeVarDecl.init->expr)) {
      auto exprLit = std::get<ASTExprLit>(*nodeVarDecl.init->expr);

      if (exprLit.type == AST_EXPR_LIT_BOOL) {
        codegen.headers.stdbool = true;
        code += exprLit.val;
      } else if (exprLit.type == AST_EXPR_LIT_STR) {
        codegen.functions.str_from_cstr = true;
        code += "str_from_cstr(" + exprLit.val + ", " + std::to_string(exprLit.val.length() - 2) + ")";
      } else {
        code += exprLit.val;
      }
    } else {
      auto [initNodeExprSetUp, initNodeExprCode, initNodeExprCleanUp] = codegenNodeExpr(codegen, *nodeVarDecl.init);

      setUp += initNodeExprSetUp;
      code += initNodeExprCode;
      cleanUp += initNodeExprCleanUp;
    }

    code += ";\n";

    if (nodeVarDecl.v.type->isStr()) {
      codegen.functions.str_deinit = true;

      cleanUp += std::string(codegen.indent, ' ');
      cleanUp += "str_deinit((struct str **) &" + codegenName(nodeVarDecl.v.name) + ");\n";
    }
  }

  return std::make_tuple(setUp, code, cleanUp);
}

Codegen codegen (AST *ast) {
  auto codegen = Codegen{};
  auto topLevelCode = std::string();
  auto mainCode = std::string();

  for (const auto &node : ast->nodes) {
    if (std::holds_alternative<ASTNodeMain>(node)) {
      continue;
    }

    auto [nodeSetUp, nodeCode, nodeCleanUp] = codegenNode(codegen, node);

    if (!nodeSetUp.empty() || !nodeCleanUp.empty()) {
      throw Error("Code generator returned set up or clean up data for top level statement");
    }

    topLevelCode += nodeCode;
  }

  for (const auto &node : ast->nodes) {
    if (!std::holds_alternative<ASTNodeMain>(node)) {
      continue;
    }

    auto [nodeSetUp, nodeCode, nodeCleanUp] = codegenNode(codegen, node);

    if (!nodeSetUp.empty() || !nodeCleanUp.empty()) {
      throw Error("Code generator returned set up or clean up data for main function");
    }

    mainCode += nodeCode;
    break;
  }

  auto mainBody = std::string("int main () {\n");
  mainBody += mainCode;
  mainBody += "}\n";

  auto builtinStructDefinitionsCode = std::string();
  auto builtinFunctionDeclarationsCode = std::string();
  auto builtinFunctionDefinitionsCode = std::string();

  if (
    codegen.functions.str_clone ||
    codegen.functions.str_init ||
    codegen.functions.str_from_cstr ||
    codegen.functions.str_deinit
  ) {
    builtinStructDefinitionsCode += "struct str {\n";
    builtinStructDefinitionsCode += "  unsigned char *c;\n";
    builtinStructDefinitionsCode += "  size_t l;\n";
    builtinStructDefinitionsCode += "};\n\n";
  }

  if (codegen.functions.str_clone) {
    codegen.functions.str_init = true;
    codegen.headers.string = true;

    builtinFunctionDeclarationsCode += "struct str *str_clone (const struct str *);\n";

    builtinFunctionDefinitionsCode += "struct str *str_clone (const struct str *n) {\n";
    builtinFunctionDefinitionsCode += "  struct str *s = str_init(n->l);\n";
    builtinFunctionDefinitionsCode += "  memcpy(s->c, n->c, n->l);\n";
    builtinFunctionDefinitionsCode += "  return s;\n";
    builtinFunctionDefinitionsCode += "}\n\n";
  }

  if (codegen.functions.str_from_cstr) {
    codegen.functions.str_init = true;
    codegen.headers.string = true;

    builtinFunctionDeclarationsCode += "struct str *str_from_cstr (const char *, size_t);\n";

    builtinFunctionDefinitionsCode += "struct str *str_from_cstr (const char *c, size_t l) {\n";
    builtinFunctionDefinitionsCode += "  struct str *s = str_init(l);\n";
    builtinFunctionDefinitionsCode += "  memcpy(s->c, c, l);\n";
    builtinFunctionDefinitionsCode += "  return s;\n";
    builtinFunctionDefinitionsCode += "}\n\n";
  }

  if (codegen.functions.str_init) {
    codegen.headers.stdio = true;
    codegen.headers.stdlib = true;

    builtinFunctionDeclarationsCode += "struct str *str_init (size_t);\n";

    builtinFunctionDefinitionsCode += "struct str *str_init (size_t l) {\n";
    builtinFunctionDefinitionsCode += "  size_t z = sizeof(struct str);\n";
    builtinFunctionDefinitionsCode += "  struct str *s = malloc(z);\n\n";
    builtinFunctionDefinitionsCode += "  if (s == NULL) {\n";
    builtinFunctionDefinitionsCode += R"(    fprintf(stderr, "Error: Failed to allocate %zu bytes for string\n", z);)";
    builtinFunctionDefinitionsCode += "\n";
    builtinFunctionDefinitionsCode += "    exit(EXIT_FAILURE);\n";
    builtinFunctionDefinitionsCode += "  }\n\n";
    builtinFunctionDefinitionsCode += "  s->l = l;\n";
    builtinFunctionDefinitionsCode += "  s->c = malloc(s->l);\n\n";
    builtinFunctionDefinitionsCode += "  if (s->c == NULL) {\n";
    builtinFunctionDefinitionsCode += R"(    fprintf(stderr, "Error: )";
    builtinFunctionDefinitionsCode += "Failed to allocate %zu bytes for string content\\n\", s->l);\n";
    builtinFunctionDefinitionsCode += "    exit(EXIT_FAILURE);\n";
    builtinFunctionDefinitionsCode += "  }\n\n";
    builtinFunctionDefinitionsCode += "  return s;\n";
    builtinFunctionDefinitionsCode += "}\n\n";
  }

  if (codegen.functions.str_deinit) {
    codegen.headers.stdlib = true;

    builtinFunctionDeclarationsCode += "void str_deinit (struct str **);\n";

    builtinFunctionDefinitionsCode += "void str_deinit (struct str **s) {\n";
    builtinFunctionDefinitionsCode += "  free((*s)->c);\n";
    builtinFunctionDefinitionsCode += "  free(*s);\n";
    builtinFunctionDefinitionsCode += "  *s = NULL;\n";
    builtinFunctionDefinitionsCode += "}\n\n";
  }

  auto headers = std::string(codegen.headers.math ? "#include <math.h>\n" : "");
  headers += codegen.headers.stdbool ? "#include <stdbool.h>\n" : "";
  headers += codegen.headers.stdio ? "#include <stdio.h>\n" : "";
  headers += codegen.headers.stdlib ? "#include <stdlib.h>\n" : "";
  headers += codegen.headers.string ? "#include <string.h>\n" : "";

  codegen.output += "/*!\n";
  codegen.output += " * Copyright (c) Aaron Delasy\n";
  codegen.output += " *\n";
  codegen.output += " * Unauthorized copying of this file, via any medium is strictly prohibited\n";
  codegen.output += " * Proprietary and confidential\n";
  codegen.output += " */\n\n";
  codegen.output += headers.empty() ? "" : headers + "\n";
  codegen.output += codegen.structDeclarationsCode.empty() ? "" : codegen.structDeclarationsCode + "\n";
  codegen.output += builtinStructDefinitionsCode;
  codegen.output += codegen.structDefinitionsCode;
  codegen.output += builtinFunctionDeclarationsCode.empty() ? "" : builtinFunctionDeclarationsCode + "\n";
  codegen.output += codegen.functionDeclarationsCode.empty() ? "" : codegen.functionDeclarationsCode + "\n";
  codegen.output += builtinFunctionDefinitionsCode;
  codegen.output += codegen.functionDefinitionsCode;
  codegen.output += topLevelCode.empty() ? "" : topLevelCode + "\n";
  codegen.output += mainBody;

  return codegen;
}
