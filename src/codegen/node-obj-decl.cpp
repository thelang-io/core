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

std::string Codegen::_nodeObjDecl (const ASTNode &node, bool root, CodegenPhase phase, std::string &decl, std::string &code) {
  auto nodeObjDecl = std::get<ASTNodeObjDecl>(*node.body);

  if (phase == CODEGEN_PHASE_ALLOC || phase == CODEGEN_PHASE_FULL) {
    this->_typeNameObj(nodeObjDecl.type);
    this->_typeNameObjDef(nodeObjDecl.type);
  }

  if (phase == CODEGEN_PHASE_ALLOC_METHOD || phase == CODEGEN_PHASE_FULL) {
    for (const auto &nodeObjDeclMethod : nodeObjDecl.methods) {
      code += this->_fnDecl(
        nodeObjDeclMethod.var,
        nodeObjDeclMethod.stack,
        nodeObjDeclMethod.params,
        nodeObjDeclMethod.body,
        node,
        CODEGEN_PHASE_ALLOC
      );
    }
  }

  if (phase == CODEGEN_PHASE_INIT || phase == CODEGEN_PHASE_FULL) {
    for (const auto &nodeObjDeclMethod : nodeObjDecl.methods) {
      code += this->_fnDecl(
        nodeObjDeclMethod.var,
        nodeObjDeclMethod.stack,
        nodeObjDeclMethod.params,
        nodeObjDeclMethod.body,
        node,
        phase
      );
    }
  }

  return this->_wrapNode(node, root, phase, decl + code);
}
