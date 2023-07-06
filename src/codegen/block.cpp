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

void Codegen::_block (
  std::shared_ptr<CodegenASTStmt> *c,
  const ASTBlock &nodes,
  bool saveCleanUp,
  const std::shared_ptr<CodegenASTStmt> &cleanupData,
  bool errHandled
) {
  if (ASTChecker(nodes).async() || (this->state.insideAsync && ASTChecker(nodes).hasSyncBreaking())) {
    return this->_blockAsync(c, nodes, saveCleanUp, cleanupData, errHandled);
  }

  auto initialStateCleanUp = this->state.cleanUp;
  auto jumpedBefore = false;

  if (saveCleanUp) {
    this->state.cleanUp = CodegenCleanUp(CODEGEN_CLEANUP_BLOCK, &initialStateCleanUp);

    if (cleanupData != nullptr) {
      this->state.cleanUp.merge(cleanupData);
    }

    if (
      !nodes.empty() &&
      ASTChecker(nodes[0].parent).is<ASTNodeLoop>() &&
      ASTChecker(nodes).has<ASTNodeTry>() &&
      (ASTChecker(nodes).has<ASTNodeBreak>() || ASTChecker(nodes).has<ASTNodeContinue>())
    ) {
      this->state.cleanUp.add();
    }
  }

  for (auto i = static_cast<std::size_t>(0); i < nodes.size(); i++) {
    auto node = nodes[i];
    auto nodeChecker = ASTChecker(node);

    auto throwWrapNode = std::holds_alternative<ASTNodeExpr>(*node.body) ||
      std::holds_alternative<ASTNodeVarDecl>(*node.body);

    if (i < nodes.size() - 1 && nodeChecker.hoistingFriendly() && ASTChecker(nodes[i + 1]).hoistingFriendly()) {
      for (auto j = i; j < nodes.size() && ASTChecker(nodes[j]).hoistingFriendly(); j++) {
        this->_node(c, nodes[j], CODEGEN_PHASE_ALLOC);
      }

      for (auto j = i; j < nodes.size() && ASTChecker(nodes[j]).hoistingFriendly(); j++) {
        this->_node(c, nodes[j], CODEGEN_PHASE_ALLOC_METHOD);
      }

      for (; i < nodes.size() && ASTChecker(nodes[i]).hoistingFriendly(); i++) {
        this->_node(c, nodes[i], CODEGEN_PHASE_INIT);
      }

      i--;
    } else if (std::holds_alternative<ASTNodeObjDecl>(*node.body)) {
      this->_node(c, node, CODEGEN_PHASE_ALLOC);
      this->_node(c, node, CODEGEN_PHASE_ALLOC_METHOD);
      this->_node(c, node, CODEGEN_PHASE_INIT);
    } else if (this->throws && throwWrapNode) {
      auto setJumpArg = CodegenASTExprAccess::create(
        CodegenASTExprAccess::create(CodegenASTExprAccess::create(this->_("err_state")), "buf"),
        jumpedBefore
          ? CodegenASTExprBinary::create(
              CodegenASTExprAccess::create(CodegenASTExprAccess::create(this->_("err_state")), "buf_idx"),
              "-",
              CodegenASTExprLiteral::create("1")
            )
          : CodegenASTExprUnary::create(
              CodegenASTExprAccess::create(CodegenASTExprAccess::create(this->_("err_state")), "buf_idx"),
              "++"
            )
      );

      if (!jumpedBefore) {
        this->state.cleanUp.add(
          CodegenASTExprUnary::create(
            CodegenASTExprAccess::create(
              CodegenASTExprAccess::create(this->_("err_state")),
             "buf_idx"
            ),
            "--"
          )->stmt()
        );
      }

      (*c)->append(
        CodegenASTStmtIf::create(
          CodegenASTExprBinary::create(
            CodegenASTExprCall::create(
              CodegenASTExprAccess::create(this->_("setjmp")),
              {setJumpArg}
            ),
            "!=",
            CodegenASTExprLiteral::create("0")
          ),
          CodegenASTStmtGoto::create(this->state.cleanUp.currentLabel())
        )
      );

      auto saveStateCleanUpJumpUsed = this->state.cleanUp.jumpUsed;
      this->state.cleanUp.jumpUsed = true;
      this->_node(c, node);
      this->state.cleanUp.jumpUsed = saveStateCleanUpJumpUsed;

      jumpedBefore = true;
    } else {
      this->_node(c, node);
    }
  }

  if (saveCleanUp) {
    this->state.cleanUp.gen(c);
    auto nodesChecker = ASTChecker(nodes);
    auto nodesParentChecker = ASTChecker(nodes.empty() ? nullptr : nodes.begin()->parent);

    if (!nodesParentChecker.is<ASTNodeMain>() && this->throws && !this->state.cleanUp.empty() && !errHandled) {
      auto cStmtIfBody = initialStateCleanUp.isClosestJump()
        ? CodegenASTExprCall::create(
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
          )->stmt()
        : CodegenASTStmtGoto::create(initialStateCleanUp.currentLabel());

      (*c)->append(
        CodegenASTStmtIf::create(
          CodegenASTExprBinary::create(
            CodegenASTExprAccess::create(CodegenASTExprAccess::create(this->_("err_state")), "id"),
            "!=",
            CodegenASTExprLiteral::create("-1")
          ),
          cStmtIfBody
        )
      );
    }

    if (!nodesParentChecker.is<ASTNodeLoop>() && this->state.cleanUp.continueVarUsed) {
      auto cStmtIfBody = initialStateCleanUp.hasCleanUp(CODEGEN_CLEANUP_LOOP)
        ? CodegenASTStmtGoto::create(initialStateCleanUp.currentLabel())
        : CodegenASTStmtContinue::create();

      (*c)->append(
        CodegenASTStmtIf::create(
          CodegenASTExprBinary::create(
            CodegenASTExprAccess::create(this->state.cleanUp.currentContinueVar()),
            "==",
            CodegenASTExprLiteral::create("1")
          ),
          cStmtIfBody
        )
      );
    }

    if (this->state.cleanUp.breakVarUsed) {
      auto cStmtIfBody = !nodesParentChecker.is<ASTNodeLoop>() && initialStateCleanUp.hasCleanUp(CODEGEN_CLEANUP_LOOP)
        ? CodegenASTStmtGoto::create(initialStateCleanUp.currentLabel())
        : CodegenASTStmtBreak::create();

      (*c)->append(
        CodegenASTStmtIf::create(
          CodegenASTExprBinary::create(
            CodegenASTExprAccess::create(this->state.cleanUp.currentBreakVar()),
            "==",
            CodegenASTExprLiteral::create("1")
          ),
          cStmtIfBody
        )
      );
    }

    if (this->state.cleanUp.returnVarUsed && !this->state.cleanUp.empty()) {
      (*c)->append(
        CodegenASTStmtIf::create(
          CodegenASTExprBinary::create(
            CodegenASTExprAccess::create("r"),
            "==",
            CodegenASTExprLiteral::create("1")
          ),
          initialStateCleanUp.hasCleanUp(CODEGEN_CLEANUP_FN)
            ? CodegenASTStmtGoto::create(initialStateCleanUp.currentLabel())
            : CodegenASTStmtReturn::create()
        )
      );
    }

    this->state.cleanUp = initialStateCleanUp;
  }
}

void Codegen::_blockAsync (
  std::shared_ptr<CodegenASTStmt> *c,
  const ASTBlock &nodes,
  bool saveCleanUp,
  const std::shared_ptr<CodegenASTStmt> &cleanupData,
  [[maybe_unused]] bool errHandled
) {
  auto initialStateCleanUp = this->state.cleanUp;
  auto code = std::string();

  if (saveCleanUp) {
    this->state.cleanUp = CodegenCleanUp(CODEGEN_CLEANUP_BLOCK, &initialStateCleanUp, true);

    if (cleanupData != nullptr) {
      this->state.cleanUp.merge(cleanupData);
    }
  }

  for (auto i = static_cast<std::size_t>(0); i < nodes.size(); i++) {
    auto &node = nodes[i];
    auto nodeChecker = ASTChecker(node);

    if (i < nodes.size() - 1 && nodeChecker.hoistingFriendly() && ASTChecker(nodes[i + 1]).hoistingFriendly()) {
      for (auto j = i; j < nodes.size() && ASTChecker(nodes[j]).hoistingFriendly(); j++) {
        this->_node(c, nodes[j], CODEGEN_PHASE_ALLOC);
      }

      for (auto j = i; j < nodes.size() && ASTChecker(nodes[j]).hoistingFriendly(); j++) {
        this->_node(c, nodes[j], CODEGEN_PHASE_ALLOC_METHOD);
      }

      for (; i < nodes.size() && ASTChecker(nodes[i]).hoistingFriendly(); i++) {
        this->_node(c, nodes[i], CODEGEN_PHASE_INIT);
      }

      i--;
    } else if (std::holds_alternative<ASTNodeObjDecl>(*node.body)) {
      this->_node(c, node, CODEGEN_PHASE_ALLOC);
      this->_node(c, node, CODEGEN_PHASE_ALLOC_METHOD);
      this->_node(c, node, CODEGEN_PHASE_INIT);
    } else if (!nodeChecker.hasSyncBreaking() && !nodeChecker.hasAwait()) {
      this->_node(c, node);
    } else {
      this->_nodeAsync(c, node);
    }
  }

  if (saveCleanUp) {
    auto nodesChecker = ASTChecker(nodes);
    auto nodesParentChecker = ASTChecker(nodes.empty() ? nullptr : nodes.begin()->parent);

    if (!this->state.cleanUp.empty()) {
      this->state.cleanUp.genAsync(c, this->state.asyncCounter);

      if (this->state.cleanUp.breakVarUsed) {
        (*c)->append(
          CodegenASTStmtIf::create(
            CodegenASTExprBinary::create(
              CodegenASTExprUnary::create("*", CodegenASTExprAccess::create(this->state.cleanUp.currentBreakVar())),
              "==",
              CodegenASTExprLiteral::create("1")
            ),
            CodegenASTStmtReturn::create(initialStateCleanUp.currentLabelAsync())
          )
        );
      }

      if (!nodesParentChecker.is<ASTNodeLoop>() && this->state.cleanUp.continueVarUsed) {
        (*c)->append(
          CodegenASTStmtIf::create(
            CodegenASTExprBinary::create(
              CodegenASTExprUnary::create("*", CodegenASTExprAccess::create(this->state.cleanUp.currentContinueVar())),
              "==",
              CodegenASTExprLiteral::create("1")
            ),
            CodegenASTStmtReturn::create(initialStateCleanUp.currentLabelAsync())
          )
        );
      }

      if (this->state.cleanUp.returnVarUsed && !this->state.cleanUp.empty()) {
        (*c)->append(
          CodegenASTStmtIf::create(
            CodegenASTExprBinary::create(
              CodegenASTExprUnary::create("*", CodegenASTExprAccess::create("r")),
              "==",
              CodegenASTExprLiteral::create("1")
            ),
            CodegenASTStmtReturn::create(initialStateCleanUp.currentLabelAsync())
          )
        );
      }

      // todo catch
    }

    this->state.cleanUp = initialStateCleanUp;
  }
}
