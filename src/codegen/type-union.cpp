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

std::string Codegen::_typeNameUnion (Type *type) {
  auto typeName = Codegen::typeName(type->name);

  for (const auto &entity : this->entities) {
    if (entity.name == typeName) {
      return typeName;
    }
  }

  auto subTypes = std::get<TypeUnion>(type->body).subTypes;

  this->_apiEntity(typeName, CODEGEN_ENTITY_OBJ, [&] (auto &decl, auto &def) {
    decl += "struct " + typeName + ";";
    def += "struct " + typeName + " {" EOL;
    def += "  int t;" EOL;
    def += "  union {" EOL;

    for (const auto &subType : subTypes) {
      auto subTypeInfo = this->_typeInfo(subType);
      def += "    " + subTypeInfo.typeCode + "v" + this->_typeDefIdx(subType) + ";" EOL;
    }

    def += "  };" EOL;
    def += "};";

    return 0;
  });

  this->_apiEntity(typeName + "_alloc", CODEGEN_ENTITY_FN, [&] (auto &decl, auto &def) {
    decl += "struct _{" + typeName + "} " + typeName + "_alloc (int, ...);";
    def += "struct _{" + typeName + "} " + typeName + "_alloc (int t, ...) {" EOL;
    def += "  struct _{" + typeName + "} r = {t};" EOL;
    def += "  _{va_list} args;" EOL;
    def += "  _{va_start}(args, t);" EOL;

    for (const auto &subType : subTypes) {
      auto subTypeInfo = this->_typeInfo(subType);

      def += "  if (t == _{" + this->_typeDef(subType) + "}) ";
      def += "r.v" + this->_typeDefIdx(subType) + " = _{va_arg}(args, " + subType->vaArgCode(subTypeInfo.typeCodeTrimmed) + ");" EOL;
    }

    def += "  _{va_end}(args);" EOL;
    def += "  return r;" EOL;
    def += "}";

    return 0;
  });

  auto freeFnEntityIdx = this->_apiEntity(typeName + "_free", CODEGEN_ENTITY_FN, [&] (auto &decl, auto &def) {
    decl += "void " + typeName + "_free (struct _{" + typeName + "});";
    def += "void " + typeName + "_free (struct _{" + typeName + "} n) {" EOL;

    for (const auto &subType : subTypes) {
      if (!subType->shouldBeFreed()) {
        continue;
      }

      def += "  if (n.t == _{" + this->_typeDef(subType) + "}) ";
      def += this->_genFreeFn(subType, "n.v" + this->_typeDefIdx(subType)) + ";" EOL;
    }

    def += "}";

    return 0;
  });

  auto copyFnEntityIdx = this->_apiEntity(typeName + "_copy", CODEGEN_ENTITY_FN, [&] (auto &decl, auto &def) {
    decl += "struct _{" + typeName + "} " + typeName + "_copy (const struct _{" + typeName + "});";
    def += "struct _{" + typeName + "} " + typeName + "_copy (const struct _{" + typeName + "} n) {" EOL;
    def += "  struct _{" + typeName + "} r = {n.t};" EOL;

    for (const auto &subType : subTypes) {
      auto subTypeProp = "v" + this->_typeDefIdx(subType);

      def += "  if (n.t == _{" + this->_typeDef(subType) + "}) r." + subTypeProp + " = ";
      def += this->_genCopyFn(subType, "n." + subTypeProp) + ";" EOL;
    }

    def += "  return r;" EOL;
    def += "}";

    return freeFnEntityIdx;
  });

  this->_apiEntity(typeName + "_eq", CODEGEN_ENTITY_FN, [&] (auto &decl, auto &def) {
    decl += "_{bool} " + typeName + "_eq (struct _{" + typeName + "}, struct _{" + typeName + "});";
    def += "_{bool} " + typeName + "_eq (struct _{" + typeName + "} n1, struct _{" + typeName + "} n2) {" EOL;
    def += "  _{bool} r = n1.t == n2.t;" EOL;
    def += "  if (r) {" EOL;

    for (const auto &subType : subTypes) {
      auto subTypeProp = "v" + this->_typeDefIdx(subType);

      def += "    if (n1.t == _{" + this->_typeDef(subType) + "}) ";
      def += "r = " + this->_genEqFn(subType, "n1." + subTypeProp, "n2." + subTypeProp) + ";" EOL;
    }

    def += "  }" EOL;
    def += "  " + this->_genFreeFn(type, "n1") + ";" EOL;
    def += "  " + this->_genFreeFn(type, "n2") + ";" EOL;
    def += "  return r;" EOL;
    def += "}";

    return copyFnEntityIdx + 1;
  });

  this->_apiEntity(typeName + "_ne", CODEGEN_ENTITY_FN, [&] (auto &decl, auto &def) {
    decl += "_{bool} " + typeName + "_ne (struct _{" + typeName + "}, struct _{" + typeName + "});";
    def += "_{bool} " + typeName + "_ne (struct _{" + typeName + "} n1, struct _{" + typeName + "} n2) {" EOL;
    def += "  _{bool} r = n1.t != n2.t;" EOL;
    def += "  if (!r) {" EOL;

    for (const auto &subType : subTypes) {
      auto subTypeProp = "v" + this->_typeDefIdx(subType);

      def += "    if (n1.t == _{" + this->_typeDef(subType) + "}) ";
      def += "r = " + this->_genEqFn(subType, "n1." + subTypeProp, "n2." + subTypeProp, true) + ";" EOL;
    }

    def += "  }" EOL;
    def += "  " + this->_genFreeFn(type, "n1") + ";" EOL;
    def += "  " + this->_genFreeFn(type, "n2") + ";" EOL;
    def += "  return r;" EOL;
    def += "}";

    return 0;
  });

  this->_apiEntity(typeName + "_realloc", CODEGEN_ENTITY_FN, [&] (auto &decl, auto &def) {
    decl += "struct _{" + typeName + "} " + typeName + "_realloc (struct _{" + typeName + "}, struct _{" + typeName + "});";
    def += "struct _{" + typeName + "} " + typeName + "_realloc (struct _{" + typeName + "} n1, struct _{" + typeName + "} n2) {" EOL;
    def += "  " + this->_genFreeFn(type, "n1") + ";" EOL;
    def += "  return n2;" EOL;
    def += "}";

    return 0;
  });

  this->_apiEntity(typeName + "_str", CODEGEN_ENTITY_FN, [&] (auto &decl, auto &def) {
    decl += "_{struct str} " + typeName + "_str (struct _{" + typeName + "});";
    def += "_{struct str} " + typeName + "_str (struct _{" + typeName + "} n) {" EOL;
    def += "  _{struct str} r;" EOL;

    for (const auto &subType : subTypes) {
      auto subTypeProp = "v" + this->_typeDefIdx(subType);

      def += "  if (n.t == _{" + this->_typeDef(subType) + "}) r = ";
      def += this->_genStrFn(subType, "n." + subTypeProp, true, false) + ";" EOL;
    }

    def += "  " + this->_genFreeFn(type, "n") + ";" EOL;
    def += "  return r;" EOL;
    def += "}";

    return 0;
  });

  return typeName;
}
