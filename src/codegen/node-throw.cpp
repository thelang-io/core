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

#include "../Codegen.hpp"

CodegenASTStmt &Codegen::_nodeThrow (CodegenASTStmt &c, const ASTNode &node) {
  auto nodeThrow = std::get<ASTNodeThrow>(*node.body);
  auto argTypeInfo = this->_typeInfo(nodeThrow.arg.type);
  auto cArg = this->_nodeExpr(nodeThrow.arg, argTypeInfo.type, node, c);
  auto argNodeExprDef = this->_typeDef(argTypeInfo.type);

  c.append(
    CodegenASTExprCall::create(
      CodegenASTExprAccess::create(this->_("error_assign")),
      {
        CodegenASTExprUnary::create("&", CodegenASTExprAccess::create(this->_("err_state"))),
        CodegenASTExprAccess::create(this->_(argNodeExprDef)),
        CodegenASTExprCast::create(CodegenASTType::create("void *"), cArg),
        CodegenASTExprCast::create(
          CodegenASTType::create("void (*) (void *)"),
          CodegenASTExprUnary::create("&", CodegenASTExprAccess::create(this->_(argTypeInfo.typeName + "_free")))
        ),
        CodegenASTExprLiteral::create(std::to_string(node.start.line)),
        CodegenASTExprLiteral::create(std::to_string(node.start.col + 1))
      }
    ).stmt()
  );

  if (this->state.cleanUp.isClosestJump()) {
    c.append(
      CodegenASTExprCall::create(
        CodegenASTExprAccess::create(this->_("longjmp")),
        {
          CodegenASTExprAccess::create(
            CodegenASTExprAccess::create(CodegenASTExprAccess::create(this->_("err_state")), "buf"),
            CodegenASTExprBinary::create(
              CodegenASTExprAccess::create(CodegenASTExprAccess::create(this->_("err_state")), "buf_idx"),
              "-",
              CodegenASTExprLiteral::create("1")
            )
          ),
          CodegenASTExprAccess::create(CodegenASTExprAccess::create(this->_("err_state")), "id")
        }
      ).stmt()
    );
  } else {
    c.append(CodegenASTStmtGoto::create(this->state.cleanUp.currentLabel()));
  }

  return c;
}

CodegenASTStmt &Codegen::_nodeThrowAsync (CodegenASTStmt &c, const ASTNode &node) {
  return this->_nodeThrow(c, node);
}
