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

#include "Type.hpp"

class TypeMap {
 public:
  std::optional<Type *> self;
  std::vector<std::string> stack;

  Type *createMap (Type *, Type *);
  Type *alias (const std::string &, Type *);
  Type *arrayOf (Type *);
  Type *enumeration (const std::string &, const std::string &, const std::vector<Type *> &);
  Type *enumerator (const std::string &, const std::string &);
  Type *fn (
    const std::vector<TypeFnParam> &,
    Type *,
    const std::optional<TypeFnMethodInfo> & = std::nullopt
  );
  Type *get (const std::string &);
  bool has (const std::string &);
  void init ();
  bool isSelf (Type *);
  std::string name (const std::string &) const;
  Type *obj (const std::string &, const std::string &, const std::vector<TypeField> & = {});
  Type *opt (Type *);
  Type *ref (Type *);
  Type *unionType (const std::vector<Type *> &);
  Type *unionAdd (Type *, Type *);
  Type *unionSub (const Type *, const Type *);

 private:
  std::size_t _fnIdx = 0;
  std::vector<std::unique_ptr<Type>> _items;
  std::size_t _mapIdx = 0;
  std::size_t _unIdx = 0;
};

#endif
