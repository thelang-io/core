/*!
 * Copyright (c) 2018 Aaron Delasy
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm>
#include "../Codegen.hpp"
#include "../config.hpp"

void Codegen::_fnDeclInitErrorHandling (std::shared_ptr<CodegenASTStmt> *c, const std::string &name) {
  (*c)->append(
    CodegenASTExprCall::create(
      CodegenASTExprAccess::create(this->_("error_stack_push")),
      {
        this->_genErrState(false, false),
        CodegenASTExprLiteral::create(R"(")" + this->reader->path + R"(")"),
        CodegenASTExprLiteral::create(R"(")" + name + R"(")"),
        CodegenASTExprAccess::create(CodegenASTExprAccess::create("p"), "line", true),
        CodegenASTExprAccess::create(CodegenASTExprAccess::create("p"), "col", true)
      }
    )->stmt()
  );

  if (!this->state.insideAsync) {
    this->state.cleanUp.add(
      CodegenASTStmtIf::create(
        CodegenASTExprBinary::create(
          this->_genErrState(false, false, "id"),
          "!=",
          CodegenASTExprLiteral::create("-1")
        ),
        CodegenASTExprCall::create(
          CodegenASTExprAccess::create(this->_("longjmp")),
          {
            CodegenASTExprAccess::create(
              this->_genErrState(false, false, "buf"),
              CodegenASTExprBinary::create(
                this->_genErrState(false, false, "buf_idx"),
                "-",
                CodegenASTExprLiteral::create("1")
              )
            ),
            this->_genErrState(false, false, "id")
          }
        )->stmt()
      )
    );
  }

  this->state.cleanUp.add(
    CodegenASTExprCall::create(
      CodegenASTExprAccess::create(this->_("error_stack_pop")),
      {this->_genErrState(false, false)}
    )->stmt()
  );
}

void Codegen::_fnDecl (
  std::shared_ptr<CodegenASTStmt> *c,
  std::shared_ptr<Var> var,
  const std::vector<std::shared_ptr<Var>> &stack,
  const std::vector<ASTFnDeclParam> &params,
  const std::optional<ASTBlock> &body,
  const ASTNode &node,
  CodegenPhase phase
) {
  if (body == std::nullopt) {
    return;
  }

  auto codeName = var->codeName;
  auto typeName = Codegen::typeName(codeName);
  auto varTypeInfo = this->_typeInfo(var->type);
  auto fnType = std::get<TypeFn>(var->type->body);
  auto paramsName = varTypeInfo.typeName + "P";
  auto contextName = typeName + "X";
  auto isAsyncMain = codeName == "async_main";
  auto hasSelfParam = fnType.isMethod && fnType.callInfo.isSelfFirst;
  auto hasParams = !params.empty() || hasSelfParam || this->throws;
  auto asyncBreakNodesCount = Codegen::countAsyncLoopDepth<ASTNodeBreak>(fnType.async ? *body : ASTBlock{}, 0);
  auto asyncContinueNodesCount = Codegen::countAsyncLoopDepth<ASTNodeContinue>(fnType.async ? *body : ASTBlock{}, 0);
  auto asyncBodyDeclarations = Codegen::filterAsyncDeclarations(fnType.async ? ASTChecker::flattenNode(*body) : ASTBlock{});
  auto awaitExprCalls = !fnType.async ? std::vector<ASTNodeExpr>{} : ASTChecker::flattenExpr(ASTChecker::flattenNodeExprs(*body));
  auto hasAsyncReturn = fnType.async && ASTChecker(*body).has<ASTNodeReturn>();

  awaitExprCalls.erase(std::remove_if(awaitExprCalls.begin(), awaitExprCalls.end(), [] (const auto &it) -> bool {
    return !std::holds_alternative<ASTExprAwait>(*it.body) || it.type->isVoid();
  }), awaitExprCalls.end());

  auto hasStack = !stack.empty() ||
    !asyncBodyDeclarations.empty() ||
    hasAsyncReturn ||
    asyncBreakNodesCount > 0 ||
    asyncContinueNodesCount > 0 ||
    !awaitExprCalls.empty();

  if (phase == CODEGEN_PHASE_ALLOC || phase == CODEGEN_PHASE_FULL) {
    auto initialStateCleanUp = this->state.cleanUp;
    auto initialStateContextVars = this->state.contextVars;
    auto initialStateInsideAsync = this->state.insideAsync;
    auto initialStateReturnType = this->state.returnType;
    auto initialStateAsyncCounter = this->state.asyncCounter;
    auto contextEntityIdx = 0;

    this->state.cleanUp = CodegenCleanUp(CODEGEN_CLEANUP_FN, &initialStateCleanUp, fnType.async);
    this->state.insideAsync = fnType.async;

    if (hasStack) {
      contextEntityIdx = this->_apiEntity(contextName, CODEGEN_ENTITY_OBJ, [&] (auto &decl, auto &def) {
        decl += "struct " + contextName + ";";
        def += "struct " + contextName + " {" EOL;

        for (const auto &contextVar : stack) {
          auto contextVarName = Codegen::name(contextVar->codeName);
          auto contextVarTypeInfo = this->_typeInfo(contextVar->type);
          auto typeRefCode = contextVar->mut ? contextVarTypeInfo.typeRefCode : contextVarTypeInfo.typeRefCodeConst;

          def += "  " + typeRefCode + contextVarName + ";" EOL;
        }

        for (const auto &asyncVar : asyncBodyDeclarations) {
          auto contextVarName = Codegen::name(asyncVar->codeName);
          auto typeInfo = this->_typeInfo(asyncVar->type);
          def += "  " + typeInfo.typeCode + contextVarName + ";" EOL;
        }

        if (hasAsyncReturn) {
          def += "  unsigned char r;" EOL;
        }

        for (auto i = static_cast<std::size_t>(0); i < asyncBreakNodesCount; i++) {
          def += "  unsigned char b" + std::to_string(i + 1) + ";" EOL;
        }

        for (auto i = static_cast<std::size_t>(0); i < asyncContinueNodesCount; i++) {
          def += "  unsigned char c" + std::to_string(i + 1) + ";" EOL;
        }

        for (const auto &awaitExpr : awaitExprCalls) {
          auto exprAwait = std::get<ASTExprAwait>(*awaitExpr.body);
          auto typeInfo = this->_typeInfo(awaitExpr.type);

          def += "  " + typeInfo.typeCode + "t" + std::to_string(exprAwait.id) + ";" EOL;
        }

        def += "};";
        return 0;
      });
    }

    this->_apiEntity(typeName, CODEGEN_ENTITY_FN, [&] (auto &decl, auto &def) {
      this->varMap.save();
      auto cBody = CodegenASTStmtCompound::create();

      if (hasParams) {
        cBody->append(
          CodegenASTStmtVarDecl::create(
            CodegenASTType::create("struct " + this->_(paramsName) + " *"),
            CodegenASTExprAccess::create("p"),
            CodegenASTExprAccess::create("pp")
          )
        );
      }

      if (hasParams) {
        this->state.cleanUp.add(
          CodegenASTExprCall::create(
            CodegenASTExprAccess::create(this->_("free")),
            {CodegenASTExprAccess::create("pp")}
          )->stmt()
        );
      }

      if (this->throws && fnType.async) {
        this->state.cleanUp.add(
          CodegenASTExprCall::create(
            CodegenASTExprAccess::create(this->_("free")),
            {this->_genErrState(false, false)}
          )->stmt()
        );
      }

      if (fnType.async) {
        this->state.cleanUp.add(
          CodegenASTExprCall::create(
            CodegenASTExprAccess::create(this->_("threadpool_job_deinit")),
            {CodegenASTExprAccess::create("job")}
          )->stmt()
        );

        this->state.cleanUp.add(
          CodegenASTStmtIf::create(
            CodegenASTExprBinary::create(
              CodegenASTExprAccess::create(CodegenASTExprAccess::create("job"), "parent", true),
              "!=",
              CodegenASTExprAccess::create(this->_("NULL"))
            ),
            CodegenASTExprCall::create(
              CodegenASTExprAccess::create(this->_("threadpool_insert")),
              {
                CodegenASTExprAccess::create("tp"),
                CodegenASTExprAccess::create(CodegenASTExprAccess::create("job"), "parent", true)
              }
            )->stmt(),
            this->throws
              ? CodegenASTStmtIf::create(
                CodegenASTExprBinary::create(
                  this->_genErrState(false, false, "id"),
                  "!=",
                  CodegenASTExprLiteral::create("-1")
                ),
                CodegenASTExprCall::create(
                  CodegenASTExprAccess::create(this->_("threadpool_error_assign")),
                  {
                    CodegenASTExprAccess::create("tp"),
                    this->_genErrState(false, false)
                  }
                )->stmt()
              )
              : CodegenASTStmtNull::create()
          )
        );

        if (this->throws) {
          this->state.cleanUp.add(
            CodegenASTStmtIf::create(
              CodegenASTExprBinary::create(
                CodegenASTExprBinary::create(
                  this->_genErrState(false, false, "id"),
                  "!=",
                  CodegenASTExprLiteral::create("-1")
                ),
                "&&",
                CodegenASTExprBinary::create(
                  CodegenASTExprAccess::create(CodegenASTExprAccess::create("job"), "parent", true),
                  "!=",
                  CodegenASTExprAccess::create(this->_("NULL"))
                )
              ),
              CodegenASTExprCall::create(
                CodegenASTExprAccess::create(this->_("threadpool_error_assign_parent")),
                {
                  CodegenASTExprAccess::create("tp"),
                  CodegenASTExprAccess::create(CodegenASTExprAccess::create("job"), "parent", true),
                  this->_genErrState(false, false)
                }
              )->stmt()
            )
          );
        }
      }

      if (this->throws && !fnType.async) {
        this->_fnDeclInitErrorHandling(&cBody, var->name);
      }

      if (hasStack) {
        cBody->append(
          CodegenASTStmtVarDecl::create(
            CodegenASTType::create("struct " + this->_(contextName) + " *"),
            CodegenASTExprAccess::create("x"),
            CodegenASTExprAccess::create("px")
          )
        );

        for (const auto &contextVar : stack) {
          auto contextVarName = Codegen::name(contextVar->codeName);
          auto contextVarTypeInfo = this->_typeInfo(contextVar->type);
          this->state.contextVars.insert(contextVarName);

          cBody->append(
            CodegenASTStmtVarDecl::create(
              CodegenASTType::create(contextVar->mut ? contextVarTypeInfo.typeRefCode : contextVarTypeInfo.typeRefCodeConst),
              CodegenASTExprAccess::create(contextVarName),
              CodegenASTExprAccess::create(CodegenASTExprAccess::create("x"), contextVarName, true)
            )
          );
        }

        for (const auto &asyncVar : asyncBodyDeclarations) {
          auto contextVarName = Codegen::name(asyncVar->codeName);
          auto typeInfo = this->_typeInfo(asyncVar->type);
          this->state.contextVars.insert(contextVarName);

          cBody->append(
            CodegenASTStmtVarDecl::create(
              CodegenASTType::create(typeInfo.typeRefCode),
              CodegenASTExprAccess::create(contextVarName),
              CodegenASTExprUnary::create(
                "&",
                CodegenASTExprAccess::create(CodegenASTExprAccess::create("x"), contextVarName, true)
              )
            )
          );
        }

        if (hasAsyncReturn) {
          cBody->append(
            CodegenASTStmtVarDecl::create(
              CodegenASTType::create("unsigned char *"),
              CodegenASTExprAccess::create("r"),
              CodegenASTExprUnary::create(
                "&",
                CodegenASTExprAccess::create(CodegenASTExprAccess::create("x"), "r", true)
              )
            )
          );
        }

        for (auto i = static_cast<std::size_t>(0); i < asyncBreakNodesCount; i++) {
          auto contextVarName = "b" + std::to_string(i + 1);

          cBody->append(
            CodegenASTStmtVarDecl::create(
              CodegenASTType::create("unsigned char *"),
              CodegenASTExprAccess::create(contextVarName),
              CodegenASTExprUnary::create(
                "&",
                CodegenASTExprAccess::create(CodegenASTExprAccess::create("x"), contextVarName, true)
              )
            )
          );
        }

        for (auto i = static_cast<std::size_t>(0); i < asyncContinueNodesCount; i++) {
          auto contextVarName = "c" + std::to_string(i + 1);

          cBody->append(
            CodegenASTStmtVarDecl::create(
              CodegenASTType::create("unsigned char *"),
              CodegenASTExprAccess::create(contextVarName),
              CodegenASTExprUnary::create(
                "&",
                CodegenASTExprAccess::create(CodegenASTExprAccess::create("x"), contextVarName, true)
              )
            )
          );
        }

        for (const auto &awaitExpr : awaitExprCalls) {
          auto exprAwait = std::get<ASTExprAwait>(*awaitExpr.body);
          auto contextVarName = "t" + std::to_string(exprAwait.id);
          auto typeInfo = this->_typeInfo(awaitExpr.type);

          cBody->append(
            CodegenASTStmtVarDecl::create(
              CodegenASTType::create(typeInfo.typeRefCode),
              CodegenASTExprAccess::create(contextVarName),
              CodegenASTExprUnary::create(
                "&",
                CodegenASTExprAccess::create(CodegenASTExprAccess::create("x"), contextVarName, true)
              )
            )
          );
        }
      }

      if (hasSelfParam) {
        auto selfTypeInfo = this->_typeInfo(fnType.callInfo.selfType);

        cBody->append(
          CodegenASTStmtVarDecl::create(
            CodegenASTType::create(fnType.callInfo.isSelfMut ? selfTypeInfo.typeCode : selfTypeInfo.typeCodeConst),
            CodegenASTExprAccess::create(Codegen::name(fnType.callInfo.selfCodeName)),
            CodegenASTExprAccess::create(CodegenASTExprAccess::create("p"), "self", true)
          )
        );
      }

      if (!params.empty()) {
        auto paramIdx = static_cast<std::size_t>(0);

        for (const auto &param : params) {
          auto paramName = Codegen::name(param.var->codeName);
          auto paramTypeInfo = this->_typeInfo(param.var->type);
          auto paramIdxStr = std::to_string(paramIdx);

          cBody->append(
            CodegenASTStmtVarDecl::create(
              CodegenASTType::create(param.var->mut ? paramTypeInfo.typeCode : paramTypeInfo.typeCodeConst),
              CodegenASTExprAccess::create(paramName),
              param.init == std::nullopt
                ? CodegenASTExprAccess::create(CodegenASTExprAccess::create("p"), "n" + paramIdxStr, true)
                : CodegenASTExprCond::create(
                  CodegenASTExprBinary::create(
                    CodegenASTExprAccess::create(CodegenASTExprAccess::create("p"), "o" + paramIdxStr, true),
                    "==",
                    CodegenASTExprLiteral::create("1")
                  ),
                  CodegenASTExprAccess::create(CodegenASTExprAccess::create("p"), "n" + paramIdxStr, true),
                  this->_nodeExpr(*param.init, paramTypeInfo.type, node, &cBody)
                )
            )
          );

          if (paramTypeInfo.type->shouldBeFreed()) {
            this->state.cleanUp.add(
              this->_genFreeFn(paramTypeInfo.type, CodegenASTExprAccess::create(paramName))->stmt()
            );
          }

          paramIdx++;
        }
      }

      if (hasSelfParam && fnType.callInfo.selfType->shouldBeFreed()) {
        this->state.cleanUp.add(
          this->_genFreeFn(
            fnType.callInfo.selfType,
            CodegenASTExprAccess::create(Codegen::name(fnType.callInfo.selfCodeName))
          )->stmt()
        );
      }

      if (fnType.async && !body->empty()) {
        cBody = cBody->append(CodegenASTStmtSwitch::create(CodegenASTExprAccess::create("step")));
        cBody = cBody->append(CodegenASTStmtCase::create(
          CodegenASTExprLiteral::create("0"),
          CodegenASTStmtCompound::create()
        ));
      }

      if (this->throws && fnType.async && !isAsyncMain) {
        this->_fnDeclInitErrorHandling(&cBody, var->name);
      }

      auto returnTypeInfo = this->_typeInfo(fnType.returnType);

      this->state.returnType = returnTypeInfo.type;
      this->state.asyncCounter = 0;
      this->_block(&cBody, *body, false);
      this->varMap.restore();

      if (!returnTypeInfo.type->isVoid() && this->state.cleanUp.valueVarUsed) {
        if (this->state.insideAsync) {
          cBody->exit()->exit()->exit()->prepend(
            CodegenASTStmtVarDecl::create(
              CodegenASTType::create(returnTypeInfo.typeRefCode),
              CodegenASTExprAccess::create("v"),
              CodegenASTExprAccess::create("pv")
            )
          );

          this->state.cleanUp.genAsync(&cBody, this->state.asyncCounter);
        } else {
          cBody->prepend(
            CodegenASTStmtVarDecl::create(
              CodegenASTType::create(returnTypeInfo.typeCode),
              CodegenASTExprAccess::create("v")
            )
          );

          this->state.cleanUp.gen(&cBody);
        }
      } else if (this->state.insideAsync) {
        this->state.cleanUp.genAsync(&cBody, this->state.asyncCounter);
      } else {
        this->state.cleanUp.gen(&cBody);
      }

      if (!returnTypeInfo.type->isVoid() && this->state.cleanUp.valueVarUsed && !this->state.insideAsync) {
        cBody->append(CodegenASTStmtReturn::create(CodegenASTExprAccess::create("v")));
      }

      if (this->state.cleanUp.returnVarUsed && !this->state.insideAsync) {
        auto returnVarDecl = CodegenASTStmtVarDecl::create(
          CodegenASTType::create("unsigned char"),
          CodegenASTExprAccess::create(this->state.cleanUp.currentReturnVar()),
          CodegenASTExprLiteral::create("0")
        );
        cBody->prepend(returnVarDecl);
      }

      if (fnType.async && !body->empty()) {
        cBody = cBody->exit()->exit()->exit();
      }

      decl += (this->state.insideAsync ? "void " : returnTypeInfo.typeCode) + typeName + " (";
      def += (this->state.insideAsync ? "void " : returnTypeInfo.typeCode) + typeName + " (";

      if (fnType.async) {
        decl += this->_apiEval("_{threadpool_t} *, _{threadpool_job_t} *, ");
        def += this->_apiEval("_{threadpool_t} *tp, _{threadpool_job_t} *job, ");
      }

      decl += "void *";
      def += "void *px";

      if (hasParams || fnType.async) {
        decl += ", void *";
        def += ", void *pp";
      }

      if (fnType.async) {
        decl += ", void *, int";
        def += ", void *pv, int step";
      }

      decl += ");";
      def += ") " + cBody->str(0, false);

      return hasStack ? contextEntityIdx : 0;
    });

    if (hasStack) {
      this->_apiEntity(typeName + "_alloc", CODEGEN_ENTITY_FN, [&] (auto &decl, auto &def) {
        decl += "void " + typeName + "_alloc (" + varTypeInfo.typeRefCode + ", struct _{" + contextName + "});";
        def += "void " + typeName + "_alloc (" + varTypeInfo.typeRefCode + "n, struct _{" + contextName + "} x) {" EOL;
        def += "  _{size_t} l = sizeof(struct _{" + contextName + "});" EOL;
        def += "  struct _{" + contextName + "} *r = _{alloc}(l);" EOL;
        def += "  _{memcpy}(r, &x, l);" EOL;
        def += "  n->f = &_{" + typeName + "};" EOL;
        def += "  n->x = r;" EOL;
        def += "  n->l = l;" EOL;
        def += "}";

        return 0;
      });
    }

    this->state.cleanUp = initialStateCleanUp;
    this->state.contextVars = initialStateContextVars;
    this->state.insideAsync = initialStateInsideAsync;
    this->state.returnType = initialStateReturnType;
    this->state.asyncCounter = initialStateAsyncCounter;
  }

  auto fnName = var->builtin ? codeName : Codegen::name(codeName);

  if ((phase == CODEGEN_PHASE_ALLOC || phase == CODEGEN_PHASE_FULL) && !hasStack) {
    auto cAssignRightSide = CodegenASTExprCast::create(
      CodegenASTType::create(varTypeInfo.typeCodeTrimmed),
      CodegenASTExprInitList::create({
        CodegenASTExprUnary::create("&", CodegenASTExprAccess::create(this->_(typeName))),
        CodegenASTExprAccess::create(this->_("NULL")),
        CodegenASTExprLiteral::create("0"),
      })
    );

    if (this->state.insideAsync) {
      (*c)->append(
        CodegenASTExprAssign::create(
          CodegenASTExprUnary::create("*", CodegenASTExprAccess::create(fnName)),
          "=",
          cAssignRightSide
        )->stmt()
      );
    } else {
      (*c)->append(
        CodegenASTStmtVarDecl::create(
          CodegenASTType::create("const " + varTypeInfo.typeCode),
          CodegenASTExprAccess::create(fnName),
          cAssignRightSide
        )
      );
    }
  } else if ((phase == CODEGEN_PHASE_ALLOC || phase == CODEGEN_PHASE_FULL) && !this->state.insideAsync) {
    (*c)->append(
      CodegenASTStmtVarDecl::create(
        CodegenASTType::create("const " + varTypeInfo.typeCode),
        CodegenASTExprAccess::create(fnName)
      )
    );
  }

  if ((phase == CODEGEN_PHASE_INIT || phase == CODEGEN_PHASE_FULL) && hasStack) {
    auto contextVarsInitList = std::vector<std::shared_ptr<CodegenASTExpr>>{};

    for (const auto &contextVar : stack) {
      auto contextVarName = Codegen::name(contextVar->codeName);

      if (this->state.contextVars.contains(contextVarName)) {
        contextVarsInitList.push_back(
          CodegenASTExprAccess::create(contextVarName)
        );
      } else {
        contextVarsInitList.push_back(
          CodegenASTExprUnary::create("&", CodegenASTExprAccess::create(contextVarName))
        );
      }
    }

    (*c)->append(
      CodegenASTExprCall::create(
        CodegenASTExprAccess::create(this->_(typeName + "_alloc")),
        {
          CodegenASTExprCast::create(
            CodegenASTType::create(varTypeInfo.typeRefCode),
            this->state.insideAsync
              ? CodegenASTExprAccess::create(fnName)
              : CodegenASTExprUnary::create("&", CodegenASTExprAccess::create(fnName))
          ),
          CodegenASTExprCast::create(
            CodegenASTType::create("struct " + this->_(contextName)),
            CodegenASTExprInitList::create(contextVarsInitList)
          )
        }
      )->stmt()
    );

    if (varTypeInfo.type->shouldBeFreed()) {
      this->state.cleanUp.add(
        this->_genFreeFn(
          varTypeInfo.type,
          (this->async && node.parent != nullptr)
            ? CodegenASTExprUnary::create("*", CodegenASTExprAccess::create(fnName))
            : CodegenASTExprAccess::create(fnName)
        )->stmt()
      );
    }
  }
}
