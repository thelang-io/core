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

void Codegen::_nodeIf (std::shared_ptr<CodegenASTStmt> *c, const ASTNode &node) {
  auto nodeIf = std::get<ASTNodeIf>(*node.body);
  auto initialStateTypeCasts = this->state.typeCasts;
  auto [bodyTypeCasts, altTypeCasts] = this->_evalTypeCasts(nodeIf.cond, node);
  auto cCond = this->_nodeExpr(nodeIf.cond, this->ast->typeMap.get("bool"), node, c);
  auto cBody = CodegenASTStmtCompound::create();

  (*c)->append(CodegenASTStmtIf::create(cCond, cBody));
  *c = cBody;

  bodyTypeCasts.merge(this->state.typeCasts);
  bodyTypeCasts.swap(this->state.typeCasts);
  this->varMap.save();
  this->_block(c, nodeIf.body);
  this->varMap.restore();
  this->state.typeCasts = initialStateTypeCasts;

  if (nodeIf.alt != std::nullopt) {
    altTypeCasts.merge(this->state.typeCasts);
    altTypeCasts.swap(this->state.typeCasts);

    auto cAlt = CodegenASTStmtCompound::create();

    if (std::holds_alternative<ASTBlock>(*nodeIf.alt)) {
      this->varMap.save();
      this->_block(&cAlt, std::get<ASTBlock>(*nodeIf.alt));
      this->varMap.restore();
    } else {
      this->_node(&cAlt, std::get<ASTNode>(*nodeIf.alt));
      cAlt = cAlt->asCompound().body[0];
    }

    (*c)->exit()->setIfAlt(cAlt);
  }

  *c = (*c)->exit()->exit();
  this->state.typeCasts = initialStateTypeCasts;
}

void Codegen::_nodeIfAsync (std::shared_ptr<CodegenASTStmt> *c, const ASTNode &node) {
  auto nodeIf = std::get<ASTNodeIf>(*node.body);
  auto initialStateTypeCasts = this->state.typeCasts;
  auto afterBodyAsyncCounter = std::make_shared<std::size_t>(0);
  auto afterAltAsyncCounter = std::make_shared<std::size_t>(0);
  auto [bodyTypeCasts, altTypeCasts] = this->_evalTypeCasts(nodeIf.cond, node);
  auto cCond = this->_nodeExpr(nodeIf.cond, this->ast->typeMap.get("bool"), node, c);

  (*c)->append(
    CodegenASTStmtIf::create(
      CodegenASTExprUnary::create("!", cCond->wrap()),
      this->_genAsyncReturn(afterBodyAsyncCounter)
    )
  );

  bodyTypeCasts.merge(this->state.typeCasts);
  bodyTypeCasts.swap(this->state.typeCasts);
  this->varMap.save();
  this->_block(c, nodeIf.body);
  this->varMap.restore();
  this->state.typeCasts = initialStateTypeCasts;

  if (!(*c)->endsWith<CodegenASTStmtReturn>() && nodeIf.alt != std::nullopt) {
    (*c)->append(this->_genAsyncReturn(afterAltAsyncCounter));
  }

  *c = (*c)->increaseAsyncCounter(this->state.asyncCounter);
  *afterBodyAsyncCounter = this->state.asyncCounter;

  if (nodeIf.alt != std::nullopt) {
    altTypeCasts.merge(this->state.typeCasts);
    altTypeCasts.swap(this->state.typeCasts);

    if (std::holds_alternative<ASTBlock>(*nodeIf.alt)) {
      this->varMap.save();
      this->_block(c, std::get<ASTBlock>(*nodeIf.alt));
      this->varMap.restore();
    } else if (std::holds_alternative<ASTNode>(*nodeIf.alt)) {
      this->_nodeAsync(c, std::get<ASTNode>(*nodeIf.alt));
    }

    *c = (*c)->increaseAsyncCounter(this->state.asyncCounter);
  }

  *afterAltAsyncCounter = this->state.asyncCounter;
  this->state.typeCasts = initialStateTypeCasts;
}
