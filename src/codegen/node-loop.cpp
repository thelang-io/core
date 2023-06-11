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
#include "../config.hpp"

std::string Codegen::_nodeLoop (const ASTNode &node, bool root, CodegenPhase phase, std::string &decl, std::string &code) {
  auto nodeLoop = std::get<ASTNodeLoop>(*node.body);
  auto initialCleanUp = this->state.cleanUp;
  auto initialIndent = this->indent;

  this->varMap.save();
  this->state.cleanUp = CodegenCleanUp(CODEGEN_CLEANUP_BLOCK, &initialCleanUp);
  this->state.cleanUp.breakVarIdx += 1;

  if (nodeLoop.init == std::nullopt && nodeLoop.cond == std::nullopt && nodeLoop.upd == std::nullopt) {
    code = std::string(this->indent, ' ') + "while (1)";
  } else if (nodeLoop.init == std::nullopt && nodeLoop.upd == std::nullopt) {
    code = std::string(this->indent, ' ') + "while (" + this->_nodeExpr(*nodeLoop.cond, this->ast->typeMap.get("bool"), node, decl, true) + ")";
  } else if (nodeLoop.init != std::nullopt) {
    this->indent += 2;
    auto initCode = this->_node(*nodeLoop.init);
    this->indent = initialIndent;

    if (this->state.cleanUp.hasCleanUp(CODEGEN_CLEANUP_BLOCK)) {
      code = std::string(this->indent, ' ') + "{" EOL + initCode;
      code += std::string(this->indent + 2, ' ') + "for (;";

      this->indent += 2;
    } else {
      code = std::string(this->indent, ' ') + "for (" + this->_node(*nodeLoop.init, false) + ";";
    }

    code += (nodeLoop.cond == std::nullopt ? "" : " " + this->_nodeExpr(*nodeLoop.cond, this->ast->typeMap.get("bool"), node, decl, true)) + ";";
    code += (nodeLoop.upd == std::nullopt ? "" : " " + this->_nodeExpr(*nodeLoop.upd, nodeLoop.upd->type, node, decl, true)) + ")";
  } else {
    code = std::string(this->indent, ' ') + "for (;";
    code += (nodeLoop.cond == std::nullopt ? "" : " " + this->_nodeExpr(*nodeLoop.cond, this->ast->typeMap.get("bool"), node, decl, true)) + ";";
    code += " " + this->_nodeExpr(*nodeLoop.upd, nodeLoop.upd->type, node, decl, true) + ")";
  }

  auto saveCleanUp = this->state.cleanUp;
  this->state.cleanUp = CodegenCleanUp(CODEGEN_CLEANUP_LOOP, &saveCleanUp);

  auto bodyCode = this->_block(nodeLoop.body);
  code += " {" EOL;

  if (this->state.cleanUp.breakVarUsed) {
    code += std::string(this->indent + 2, ' ') + "unsigned char " + this->state.cleanUp.currentBreakVar() + " = 0;" EOL;
  }

  code += bodyCode + std::string(this->indent, ' ') + "}" EOL;

  if (nodeLoop.init != std::nullopt && !saveCleanUp.empty()) {
    code += saveCleanUp.gen(this->indent);

    if (saveCleanUp.returnVarUsed) {
      code += std::string(this->indent, ' ') + "if (r == 1) goto " + initialCleanUp.currentLabel() + ";" EOL;
    }

    this->indent = initialIndent;
    code += std::string(this->indent, ' ') + "}" EOL;
  }

  this->state.cleanUp.breakVarIdx -= 1;
  this->state.cleanUp = initialCleanUp;
  this->varMap.restore();

  return this->_wrapNode(node, root, phase, decl + code);
}
