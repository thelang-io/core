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

void Codegen::_nodeBreak (std::shared_ptr<CodegenASTStmt> *c, const ASTNode &node) {
  if (this->state.cleanUp.hasCleanUp(CODEGEN_CLEANUP_LOOP)) {
    (*c)->append(
      CodegenASTExprAssign::create(
        CodegenASTExprAccess::create(this->state.cleanUp.currentBreakVar()),
        "=",
        CodegenASTExprLiteral::create("1")
      )->stmt()
    );

    if (!ASTChecker(node).isLast(this->state.fnDeclBody)) {
      (*c)->append(CodegenASTStmtGoto::create(this->state.cleanUp.currentLabel()));
    }
  } else {
    (*c)->append(CodegenASTStmtBreak::create());
  }
}

void Codegen::_nodeBreakAsync (std::shared_ptr<CodegenASTStmt> *c, [[maybe_unused]] const ASTNode &node) {
  (*c)->append(
    CodegenASTExprAssign::create(
      CodegenASTExprUnary::create("*", CodegenASTExprAccess::create(this->state.cleanUp.currentBreakVar())),
      "=",
      CodegenASTExprLiteral::create("1")
    )->stmt()
  );

  (*c)->append(
    this->_genAsyncReturn(
      this->state.cleanUp.hasCleanUp(CODEGEN_CLEANUP_LOOP)
        ? this->state.cleanUp.currentLabelAsync()
        : this->state.asyncCounterLoopBreak
    )
  );
}
