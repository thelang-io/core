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

#include <algorithm>
#include "CodegenCleanUp.hpp"
#include "Error.hpp"

CodegenCleanUp::CodegenCleanUp (CodegenCleanUpType t, CodegenCleanUp *p, bool isAsync) {
  this->type = t;
  this->async = isAsync;

  if (p != nullptr) {
    this->parent = p;
    this->labelIdx = this->parent->labelIdx;
    this->breakVarIdx = this->parent->breakVarIdx;
    this->continueVarIdx = this->parent->continueVarIdx;
  }
}

void CodegenCleanUp::add (const std::shared_ptr<CodegenASTStmt> &stmt) {
  if (this->empty() || this->_data.back().labelUsed) {
    if (this->async) {
      this->_data.push_back({"", {}, false, std::make_shared<std::size_t>(1)});
    } else {
      this->_data.push_back({"L" + std::to_string(this->labelIdx)});
      this->labelIdx += 1;

      auto p = this->parent;

      while (p != nullptr) {
        p->labelIdx = this->labelIdx;
        p = p->parent;
      }
    }
  }

  this->_data.back().content.insert(this->_data.back().content.begin(), stmt);
}

std::string CodegenCleanUp::currentBreakVar () {
  this->breakVarUsed = true;

  if (this->type == CODEGEN_CLEANUP_LOOP) {
    return "b" + std::to_string(this->breakVarIdx);
  } else if (this->parent == nullptr) {
    throw Error("tried getting break var on nullptr in CodegenCleanUp");
  }

  return this->parent->currentBreakVar();
}

std::string CodegenCleanUp::currentContinueVar () {
  this->continueVarUsed = true;

  if (this->type == CODEGEN_CLEANUP_LOOP) {
    return "c" + std::to_string(this->continueVarIdx);
  } else if (this->parent == nullptr) {
    throw Error("tried getting continue var on nullptr in CodegenCleanUp");
  }

  return this->parent->currentContinueVar();
}

std::string CodegenCleanUp::currentLabel () {
  this->valueVarUsed = true;

  if (!this->empty()) {
    this->_data.back().labelUsed = true;
    return this->_data.back().label;
  } else if (this->parent == nullptr) {
    throw Error("tried getting current label on nullptr in CodegenCleanUp");
  }

  return this->parent->currentLabel();
}

std::shared_ptr<std::size_t> CodegenCleanUp::currentLabelAsync () {
  this->valueVarUsed = true;

  if (!this->empty()) {
    this->_data.back().labelUsed = true;
    return this->_data.back().asyncCounter;
  } else if (this->parent == nullptr) {
    throw Error("tried getting current label async on nullptr in CodegenCleanUp");
  }

  return this->parent->currentLabelAsync();
}

std::string CodegenCleanUp::currentReturnVar () {
  this->returnVarUsed = true;

  if (this->type == CODEGEN_CLEANUP_FN) {
    return "r";
  } else if (this->parent == nullptr) {
    throw Error("tried getting return var on nullptr in CodegenCleanUp");
  }

  return this->parent->currentReturnVar();
}

std::string CodegenCleanUp::currentValueVar () {
  this->valueVarUsed = true;

  if (this->type == CODEGEN_CLEANUP_FN) {
    return "v";
  } else if (this->parent == nullptr) {
    throw Error("tried getting value var on nullptr in CodegenCleanUp");
  }

  return this->parent->currentValueVar();
}

bool CodegenCleanUp::empty () const {
  if (this->_data.empty()) {
    return true;
  }

  return std::all_of(this->_data.begin(), this->_data.end(), [&] (const auto &it) -> bool {
    return CodegenASTStmt::emptyVector(it.content);
  });
}

void CodegenCleanUp::gen (std::shared_ptr<CodegenASTStmt> *c) const {
  if (this->_data.empty()) {
    return;
  }

  for (auto idx = this->_data.size() - 1;; idx--) {
    auto item = this->_data[idx];

    if (item.labelUsed) {
      (*c)->append(CodegenASTStmtLabel::create(item.label));
    }

    if (!item.content.empty()) {
      (*c)->merge(item.content);
    }

    if (idx == 0) {
      break;
    }
  }
}

void CodegenCleanUp::genAsync (std::shared_ptr<CodegenASTStmt> *c, std::size_t &counter) const {
  if (this->_data.empty()) {
    return;
  }

  for (auto idx = this->_data.size() - 1;; idx--) {
    auto item = this->_data[idx];

    if (item.labelUsed) {
      *c = (*c)->increaseAsyncCounter(counter);
      *item.asyncCounter = counter;
    }

    if (!item.content.empty()) {
      (*c)->merge(item.content);
    }

    if (idx == 0) {
      break;
    }
  }
}

bool CodegenCleanUp::hasCleanUp (CodegenCleanUpType t) const {
  return !this->empty() || (this->type != t && this->parent != nullptr && this->parent->hasCleanUp(t));
}

void CodegenCleanUp::merge (const std::shared_ptr<CodegenASTStmt> &stmt) {
  if (stmt->isCompound()) {
    auto stmtCompound = stmt->asCompound();

    for (auto it = stmtCompound.body.rbegin(); it != stmtCompound.body.rend(); it++) {
      this->add(*it);
    }
  } else {
    this->add(stmt);
  }
}
