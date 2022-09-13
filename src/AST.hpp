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

#ifndef SRC_AST_HPP
#define SRC_AST_HPP

#include "ASTNode.hpp"
#include "Parser.hpp"

struct ASTState {
  bool insideLoopInit = false;
  Type *returnType = nullptr;
};

class AST {
 public:
  Parser *parser;
  Reader *reader;
  TypeMap typeMap;
  VarMap varMap;
  ASTState state;

  static void populateParents (ASTBlock &, ASTNode * = nullptr);

  explicit AST (Parser *);

  virtual ASTBlock gen ();
  virtual std::string xml ();

 private:
  AST (const AST &);
  AST &operator= (const AST &);

  ASTBlock _block (const ParserBlock &, VarStack &);
  void _forwardNode (const ParserBlock &);
  ASTNode _node (const ParserStmt &, VarStack &);
  ASTNodeExpr _nodeExpr (const ParserStmtExpr &, Type *, VarStack &);
  Type *_nodeExprType (const ParserStmtExpr &, Type *);
  Type *_type (const ParserType &);
  ASTNode _wrapNode (const ParserStmt &, const ASTNodeBody &);
  ASTNodeExpr _wrapNodeExpr (const ParserStmtExpr &, Type *, const ASTExpr &);
  Type *_wrapNodeExprType (const ParserStmtExpr &, Type *, Type *);
};

#endif
