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

#ifndef SRC_TYPE_MAP_HPP
#define SRC_TYPE_MAP_HPP

#include <functional>
#include "Type.hpp"

enum TypeMapPhase {
  TYPE_MAP_DECL,
  TYPE_MAP_DEF
};

class TypeMap {
 public:
  std::optional<Type *> self;
  std::vector<std::string> stack;

  Type *createAlias (const std::string &, Type *);
  Type *createArr (Type *);
  Type *createEnum (const std::string &, const std::string &, const std::vector<Type *> &);
  Type *createFn (const std::vector<TypeFnParam> &, Type *, const std::optional<TypeFnMethodInfo> & = std::nullopt, bool = false);
  Type *createMap (Type *, Type *);
  Type *createObj (const std::string &, const std::string &, const std::vector<TypeField> & = {}, bool = false);
  Type *createOpt (Type *);
  Type *createRef (Type *);
  Type *createUnion (const std::vector<Type *> &);
  Type *enumerator (const std::string &, const std::string &);
  Type *get (const std::string &);
  bool has (const std::string &);
  void init ();
  bool isSelf (Type *);
  std::string name (const std::string &) const;
  Type *unionAdd (Type *, Type *);
  Type *unionSub (const Type *, const Type *);

 private:
  std::size_t _builtinIdx = 0;
  std::size_t _fnIdx = 0;
  std::vector<std::unique_ptr<Type>> _items;
  std::size_t _mapIdx = 0;
  std::size_t _unIdx = 0;

  void _initType (const std::string &, TypeMapPhase, const std::optional<std::function<void (Type *, Type *)>> & = std::nullopt);
  void _anyType (TypeMapPhase);
  void _boolType (TypeMapPhase);
  void _byteType (TypeMapPhase);
  void _charType (TypeMapPhase);
  void _floatType (TypeMapPhase);
  void _intType (TypeMapPhase);
  void _strType (TypeMapPhase);
  void _voidType (TypeMapPhase);
  void _bufferModule (TypeMapPhase);
  void _fsModule (TypeMapPhase);
  void _globalsModule (TypeMapPhase);
  void _pathModule (TypeMapPhase);
  void _processModule (TypeMapPhase);
  void _requestModule (TypeMapPhase);
  void _urlModule (TypeMapPhase);
};

#endif
