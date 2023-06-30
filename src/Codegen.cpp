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

#include "Codegen.hpp"
#include <filesystem>
#include <fstream>
#include "codegen-api.hpp"
#include "config.hpp"

const auto banner = std::string(
  "/*!" EOL
  " * Copyright (c) 2018 Aaron Delasy" EOL
  " *" EOL
  " * Unauthorized copying of this file, via any medium is strictly prohibited" EOL
  " * Proprietary and confidential" EOL
  " */" EOL
  EOL
);

std::string getCompilerFromPlatform (const std::string &platform) {
  if (platform == "macos") {
    return "o64-clang";
  } else if (platform == "windows") {
    return "x86_64-w64-mingw32-gcc";
  }

  return "clang";
}

std::string getOSFromPlatform (const std::string &platform) {
  if (platform != "default") {
    return platform;
  }

  #if defined(OS_WINDOWS)
    return "windows";
  #elif defined(OS_MACOS)
    return "macos";
  #elif defined(OS_LINUX)
    return "linux";
  #else
    return "unknown";
  #endif
}

void Codegen::compile (
  const std::string &path,
  const std::tuple<std::string, std::vector<std::string>> &result,
  const std::string &platform,
  bool debug
) {
  auto code = std::get<0>(result);
  auto flags = std::get<1>(result);
  auto f = std::ofstream(path + ".c");

  f << code;
  f.close();

  auto depsDir = Codegen::getEnvVar("DEPS_DIR");
  auto compiler = getCompilerFromPlatform(platform);
  auto flagsStr = std::string();
  auto libraries = std::string();

  if (!depsDir.empty()) {
    flagsStr += " -I\"" + depsDir + "/include\"";
    flagsStr += " -L\"" + depsDir + "/lib\"";
  }

  auto targetOS = getOSFromPlatform(platform);

  for (const auto &flag : flags) {
    if (
      flag.starts_with("A:") ||
      (flag.starts_with("L:") && targetOS == "linux") ||
      (flag.starts_with("M:") && targetOS == "macos") ||
      (flag.starts_with("U:") && targetOS != "windows") ||
      (flag.starts_with("W:") && targetOS == "windows")
    ) {
      flagsStr += " " + flag.substr(2);
    }
  }

  auto cmd = compiler + " " + path + ".c " + libraries + "-w -o " + path + flagsStr + " -O0 -g";
  cmd += targetOS == "linux" && debug ? "dwarf-4" : "";
  auto returnCode = std::system(cmd.c_str());
  std::filesystem::remove(path + ".c");

  if (returnCode != 0) {
    throw Error("failed to compile generated code");
  }
}

std::string Codegen::name (const std::string &name) {
  return "__THE_0_" + name;
}

std::string Codegen::stringifyFlags (const std::vector<std::string> &flags) {
  auto result = std::string();
  auto idx = static_cast<std::size_t>(0);

  for (const auto &flag : flags) {
    if (!flag.empty()) {
      result += (idx++ == 0 ? "" : " ") + flag;
    }
  }

  return result;
}

std::string Codegen::typeName (const std::string &name) {
  return "__THE_1_" + name;
}

Codegen::Codegen (AST *a) {
  this->ast = a;
  this->reader = this->ast->reader;
}

std::tuple<std::string, std::vector<std::string>> Codegen::gen () {
  auto nodes = this->ast->gen();
  this->async = ASTChecker(nodes).async(false);
  this->throws = ASTChecker(nodes).throws(false);

  for (const auto &[name, item] : codegenAPI) {
    this->api[name] = item;
  }

  this->_typeNameObj(this->ast->typeMap.get("error_Error"));
  this->_typeNameObj(this->ast->typeMap.get("fs_Stats"));
  this->_typeNameObj(this->ast->typeMap.get("request_Header"));
  this->_typeNameObj(this->ast->typeMap.get("request_Request"));
  this->_typeNameObj(this->ast->typeMap.get("request_Response"));
  this->_typeNameObj(this->ast->typeMap.get("url_URL"));

  for (auto &[name, item] : this->api) {
    for (const auto &entityDependency : item.entityDependencies) {
      this->_apiEval("_{" + entityDependency + "}", 0, name, &item.dependencies);
    }
  }

  this->_typeNameObjDef(this->ast->typeMap.get("error_Error"));
  this->_typeNameObjDef(this->ast->typeMap.get("fs_Stats"));
  this->_typeNameObjDef(this->ast->typeMap.get("request_Header"));
  this->_typeNameObjDef(this->ast->typeMap.get("request_Request"), {
    std::pair<std::string, std::string>{"free", "  _{request_close}(&n);" EOL}
  });
  this->_typeNameObjDef(this->ast->typeMap.get("request_Response"));
  this->_typeNameObjDef(this->ast->typeMap.get("url_URL"));

  auto cMain = CodegenASTStmtCompound::create();

  if (this->async) {
    cMain.append(
      CodegenASTStmtVarDecl::create(
        CodegenASTType::create(this->_("threadpool_t") + " *"),
        CodegenASTExprAccess::create("tp"),
        CodegenASTExprCall::create(
          CodegenASTExprAccess::create(this->_("threadpool_init")),
          {CodegenASTExprLiteral::create("5")}
        )
      )
    );
  }

  if (this->throws) {
    cMain.append(
      CodegenASTExprCall::create(
        CodegenASTExprAccess::create(this->_("error_stack_push")),
        {
          CodegenASTExprUnary::create("&", CodegenASTExprAccess::create(this->_("err_state"))),
          CodegenASTExprLiteral::create(R"(")" + this->reader->path + R"(")"),
          CodegenASTExprLiteral::create(R"("main")"),
          CodegenASTExprLiteral::create("0"),
          CodegenASTExprLiteral::create("0")
        }
      ).stmt()
    );

    this->state.cleanUp.merge(
      CodegenASTStmtCompound::create({
        CodegenASTExprCall::create(
          CodegenASTExprAccess::create(this->_("error_stack_pop")),
          {CodegenASTExprUnary::create("&", CodegenASTExprAccess::create(this->_("err_state")))}
        ).stmt(),
        CodegenASTStmtIf::create(
          CodegenASTExprBinary::create(
            CodegenASTExprAccess::create(CodegenASTExprAccess::create(this->_("err_state")), "id"),
            "!=",
            CodegenASTExprLiteral::create("-1")
          ),
          CodegenASTStmtCompound::create({
            CodegenASTStmtVarDecl::create(
              CodegenASTType::create("struct " + this->_("error_Error") + " *"),
              CodegenASTExprAccess::create("err"),
              CodegenASTExprAccess::create(CodegenASTExprAccess::create(this->_("err_state")), "ctx")
            ),
            CodegenASTExprCall::create(
              CodegenASTExprAccess::create(this->_("fprintf")),
              {
                CodegenASTExprAccess::create(this->_("stderr")),
                CodegenASTExprLiteral::create(R"("Uncaught Error: %.*s" )" + this->_("THE_EOL")),
                CodegenASTExprCast::create(
                  CodegenASTType::create("int"),
                  CodegenASTExprAccess::create(
                    CodegenASTExprAccess::create(CodegenASTExprAccess::create("err"), "__THE_0_stack", true),
                    "l"
                  )
                ),
                CodegenASTExprAccess::create(
                  CodegenASTExprAccess::create(CodegenASTExprAccess::create("err"), "__THE_0_stack", true),
                  "d"
                )
              }
            ).stmt(),
            CodegenASTExprCall::create(
              CodegenASTExprAccess::create(CodegenASTExprAccess::create(this->_("err_state")), "_free"),
              {CodegenASTExprAccess::create(CodegenASTExprAccess::create(this->_("err_state")), "ctx")}
            ).stmt(),
            CodegenASTExprCall::create(
              CodegenASTExprAccess::create(this->_("exit")),
              {CodegenASTExprAccess::create(this->_("EXIT_FAILURE"))}
            ).stmt()
          })
        )
      })
    );
  }

  this->_block(&cMain, nodes, false);

  if (this->async) {
    this->state.cleanUp.merge(
      CodegenASTStmtCompound::create({
        CodegenASTExprCall::create(
          CodegenASTExprAccess::create(this->_("threadpool_wait")),
          {CodegenASTExprAccess::create("tp")}
        ).stmt(),
        CodegenASTExprCall::create(
          CodegenASTExprAccess::create(this->_("threadpool_deinit")),
          {CodegenASTExprAccess::create("tp")}
        ).stmt()
      })
    );
  }

  this->state.cleanUp.gen(&cMain);

  auto defineCode = std::string();
  auto enumDeclCode = std::string();
  auto fnDeclCode = std::string();
  auto fnDefCode = std::string();
  auto structDeclCode = std::string();
  auto structDefCode = std::string();

  for (const auto &entity : this->entities) {
    if (!entity.active || entity.decl.empty()) {
      continue;
    }

    if (entity.type == CODEGEN_ENTITY_DEF) {
      defineCode += entity.decl + EOL;
    } else if (entity.type == CODEGEN_ENTITY_ENUM) {
      enumDeclCode += entity.decl + EOL;
    } else if (entity.type == CODEGEN_ENTITY_FN) {
      fnDeclCode += entity.decl + EOL;
      fnDefCode += entity.def + EOL;
    } else if (entity.type == CODEGEN_ENTITY_OBJ) {
      structDeclCode += entity.decl + EOL;
      structDefCode += entity.def + EOL;
    }
  }

  auto builtinDefineCode = std::string();
  auto builtinExternCode = std::string();
  auto builtinWinExternCode = std::string();
  auto builtinFnDeclCode = std::string();
  auto builtinFnDefCode = std::string();
  auto builtinStructDeclCode = std::string();
  auto builtinStructDefCode = std::string();
  auto builtinVarCode = std::string();

  if (this->builtins.definitions) {
    builtinDefineCode += "#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__)" EOL;
    builtinDefineCode += "  #define THE_OS_WINDOWS" EOL;
    builtinDefineCode += R"(  #define THE_EOL "\r\n")" EOL;
    builtinDefineCode += R"(  #define THE_PATH_SEP "\\")" EOL;
    builtinDefineCode += "#else" EOL;
    builtinDefineCode += "  #if defined(__APPLE__)" EOL;
    builtinDefineCode += "    #define THE_OS_MACOS" EOL;
    builtinDefineCode += "  #elif defined(__linux__)" EOL;
    builtinDefineCode += "    #define THE_OS_LINUX" EOL;
    builtinDefineCode += "  #endif" EOL;
    builtinDefineCode += R"(  #define THE_EOL "\n")" EOL;
    builtinDefineCode += R"(  #define THE_PATH_SEP "/")" EOL;
    builtinDefineCode += "#endif" EOL;
  }

  if (this->builtins.externSystemFunction036) {
    builtinWinExternCode += "  extern unsigned char NTAPI SystemFunction036 (void *, unsigned long);" EOL;
  }

  if (this->builtins.typeAny) {
    builtinStructDefCode += "struct any {" EOL;
    builtinStructDefCode += "  int t;" EOL;
    builtinStructDefCode += "  void *d;" EOL;
    builtinStructDefCode += "  size_t l;" EOL;
    builtinStructDefCode += "  struct any (*_copy) (const struct any);" EOL;
    builtinStructDefCode += "  void (*_free) (struct any);" EOL;
    builtinStructDefCode += "};" EOL;
  }

  if (this->builtins.typeBuffer) {
    builtinStructDefCode += "struct buffer {" EOL;
    builtinStructDefCode += "  unsigned char *d;" EOL;
    builtinStructDefCode += "  size_t l;" EOL;
    builtinStructDefCode += "};" EOL;
  }

  if (this->builtins.typeErrStack) {
    builtinStructDefCode += "typedef struct {" EOL;
    builtinStructDefCode += "  const char *file;" EOL;
    builtinStructDefCode += "  const char *name;" EOL;
    builtinStructDefCode += "  int line;" EOL;
    builtinStructDefCode += "  int col;" EOL;
    builtinStructDefCode += "} err_stack_t;" EOL;
  }

  if (this->builtins.typeErrState) {
    builtinStructDefCode += "typedef struct {" EOL;
    builtinStructDefCode += "  int id;" EOL;
    builtinStructDefCode += "  void *ctx;" EOL;
    builtinStructDefCode += "  jmp_buf buf[10];" EOL;
    builtinStructDefCode += "  int buf_idx;" EOL;
    builtinStructDefCode += "  err_stack_t stack[10];" EOL;
    builtinStructDefCode += "  int stack_idx;" EOL;
    builtinStructDefCode += "  void (*_free) (void *);" EOL;
    builtinStructDefCode += "} err_state_t;" EOL;
  }

  if (this->builtins.typeRequest) {
    builtinStructDefCode += "struct request {" EOL;
    builtinStructDefCode += "  #ifdef THE_OS_WINDOWS" EOL;
    builtinStructDefCode += "    SOCKET fd;" EOL;
    builtinStructDefCode += "  #else" EOL;
    builtinStructDefCode += "    int fd;" EOL;
    builtinStructDefCode += "  #endif" EOL;
    builtinStructDefCode += "  SSL_CTX *ctx;" EOL;
    builtinStructDefCode += "  SSL *ssl;" EOL;
    builtinStructDefCode += "};" EOL;
  }

  if (this->builtins.typeStr) {
    builtinStructDefCode += "struct str {" EOL;
    builtinStructDefCode += "  char *d;" EOL;
    builtinStructDefCode += "  size_t l;" EOL;
    builtinStructDefCode += "};" EOL;
  }

  if (this->builtins.typeThreadpool) {
    builtinStructDeclCode += "struct threadpool;" EOL;
    builtinStructDefCode += "typedef struct threadpool {" EOL;
    builtinStructDefCode += "  bool active;" EOL;
    builtinStructDefCode += "  pthread_cond_t cond1;" EOL;
    builtinStructDefCode += "  pthread_cond_t cond2;" EOL;
    builtinStructDefCode += "  struct threadpool_job *jobs;" EOL;
    builtinStructDefCode += "  pthread_mutex_t lock;" EOL;
    builtinStructDefCode += "  struct threadpool_thread *threads;" EOL;
    builtinStructDefCode += "  int working_threads;" EOL;
    builtinStructDefCode += "} threadpool_t;" EOL;
  }

  if (this->builtins.typeThreadpoolFunc) {
    builtinStructDefCode += "typedef int (*threadpool_func_t) (struct threadpool *, struct threadpool_job *, void *, void *, void *, int);" EOL;
  }

  if (this->builtins.typeThreadpoolJob) {
    builtinStructDeclCode += "struct threadpool_job;" EOL;
    builtinStructDefCode += "typedef struct threadpool_job {" EOL;
    builtinStructDefCode += "  struct threadpool_job *parent;" EOL;
    builtinStructDefCode += "  int (*func) (struct threadpool *, struct threadpool_job *, void *, void *, void *, int);" EOL;
    builtinStructDefCode += "  void *ctx;" EOL;
    builtinStructDefCode += "  void *params;" EOL;
    builtinStructDefCode += "  void *ret;" EOL;
    builtinStructDefCode += "  int step;" EOL;
    builtinStructDefCode += "  bool referenced;" EOL;
    builtinStructDefCode += "  struct threadpool_job *next;" EOL;
    builtinStructDefCode += "} threadpool_job_t;" EOL;
  }

  if (this->builtins.typeThreadpoolThread) {
    builtinStructDeclCode += "struct threadpool_thread;" EOL;
    builtinStructDefCode += "typedef struct threadpool_thread {" EOL;
    builtinStructDefCode += "  pthread_t id;" EOL;
    builtinStructDefCode += "  struct threadpool_thread *next;" EOL;
    builtinStructDefCode += "} threadpool_thread_t;" EOL;
  }

  if (this->builtins.typeWinReparseDataBuffer) {
    builtinStructDefCode += "#ifdef THE_OS_WINDOWS" EOL;
    builtinStructDefCode += "  struct win_reparse_data_buffer {" EOL;
    builtinStructDefCode += "    ULONG ReparseTag;" EOL;
    builtinStructDefCode += "    USHORT ReparseDataLength;" EOL;
    builtinStructDefCode += "    USHORT Reserved;" EOL;
    builtinStructDefCode += "    union {" EOL;
    builtinStructDefCode += "      struct {" EOL;
    builtinStructDefCode += "        USHORT SubstituteNameOffset;" EOL;
    builtinStructDefCode += "        USHORT SubstituteNameLength;" EOL;
    builtinStructDefCode += "        USHORT PrintNameOffset;" EOL;
    builtinStructDefCode += "        USHORT PrintNameLength;" EOL;
    builtinStructDefCode += "        ULONG Flags;" EOL;
    builtinStructDefCode += "        TCHAR PathBuffer[1];" EOL;
    builtinStructDefCode += "      } SymbolicLinkReparseBuffer;" EOL;
    builtinStructDefCode += "      struct {" EOL;
    builtinStructDefCode += "        USHORT SubstituteNameOffset;" EOL;
    builtinStructDefCode += "        USHORT SubstituteNameLength;" EOL;
    builtinStructDefCode += "        USHORT PrintNameOffset;" EOL;
    builtinStructDefCode += "        USHORT PrintNameLength;" EOL;
    builtinStructDefCode += "        TCHAR PathBuffer[1];" EOL;
    builtinStructDefCode += "      } MountPointReparseBuffer;" EOL;
    builtinStructDefCode += "      struct {" EOL;
    builtinStructDefCode += "        UCHAR DataBuffer[1];" EOL;
    builtinStructDefCode += "      } GenericReparseBuffer;" EOL;
    builtinStructDefCode += "      struct {" EOL;
    builtinStructDefCode += "        ULONG Version;" EOL;
    builtinStructDefCode += "        TCHAR StringList[1];" EOL;
    builtinStructDefCode += "      } AppExecLinkReparseBuffer;" EOL;
    builtinStructDefCode += "    };" EOL;
    builtinStructDefCode += "  };" EOL;
    builtinStructDefCode += "#endif" EOL;
  }

  if (this->needMainArgs) {
    builtinVarCode += "int argc = 0;" EOL;
    builtinVarCode += "char **argv = (void *) 0;" EOL;

    cMain.prepend(
      CodegenASTExprAssign::create(
        CodegenASTExprAccess::create("argc"),
        "=",
        CodegenASTExprAccess::create("_argc_")
      ).stmt()
    );

    cMain.prepend(
      CodegenASTExprAssign::create(
        CodegenASTExprAccess::create("argv"),
        "=",
        CodegenASTExprAccess::create("_argv_")
      ).stmt()
    );
  }

  if (this->builtins.varEnviron) {
    builtinExternCode += "extern char **environ;" EOL;
  }

  if (this->builtins.varErrState) {
    builtinVarCode += "err_state_t err_state = {-1, (void *) 0, {}, 0, {}, 0, (void *) 0};" EOL;
  }

  if (this->builtins.varLibOpensslInit) {
    builtinVarCode += "bool lib_openssl_init = false;" EOL;
  }

  if (this->builtins.varLibWs2Init) {
    builtinVarCode += "bool lib_ws2_init = false;" EOL;
  }

  for (const auto &[_, item] : this->api) {
    if (item.enabled) {
      builtinFnDeclCode += item.decl;
      builtinFnDefCode += item.def;
    }
  }

  fnDeclCode = builtinFnDeclCode + fnDeclCode;
  fnDefCode = builtinFnDefCode + fnDefCode;

  builtinDefineCode += builtinDefineCode.empty() ? "" : EOL;
  builtinExternCode += builtinExternCode.empty() ? "" : EOL;
  defineCode += defineCode.empty() ? "" : EOL;
  enumDeclCode += enumDeclCode.empty() ? "" : EOL;
  builtinStructDeclCode += builtinStructDeclCode.empty() ? "" : EOL;
  builtinStructDefCode += builtinStructDefCode.empty() ? "" : EOL;
  structDeclCode += structDeclCode.empty() ? "" : EOL;
  structDefCode += structDefCode.empty() ? "" : EOL;
  builtinVarCode += builtinVarCode.empty() ? "" : EOL;
  fnDeclCode += fnDeclCode.empty() ? "" : EOL;
  fnDefCode += fnDefCode.empty() ? "" : EOL;

  auto headers = std::string(this->builtins.libCtype ? "#include <ctype.h>" EOL : "");
  headers += this->builtins.libErrno ? "#include <errno.h>" EOL : "";
  headers += this->builtins.libFloat ? "#include <float.h>" EOL : "";
  headers += this->builtins.libInttypes ? "#include <inttypes.h>" EOL : "";
  headers += this->builtins.libMath ? "#include <math.h>" EOL : "";
  headers += this->builtins.libOpensslRand ? "#include <openssl/rand.h>" EOL : "";
  headers += this->builtins.libOpensslSsl ? "#include <openssl/ssl.h>" EOL : "";
  headers += this->builtins.libSetJmp ? "#include <setjmp.h>" EOL : "";
  headers += this->builtins.libStdNoReturn ? "#include <stdnoreturn.h>" EOL : "";
  headers += this->builtins.libStdarg ? "#include <stdarg.h>" EOL : "";
  headers += this->builtins.libStdbool ? "#include <stdbool.h>" EOL : "";
  headers += this->builtins.libStddef && !this->builtins.libStdlib ? "#include <stddef.h>" EOL : "";
  headers += this->builtins.libStdint && !this->builtins.libInttypes ? "#include <stdint.h>" EOL : "";
  headers += this->builtins.libStdio ? "#include <stdio.h>" EOL : "";
  headers += this->builtins.libStdlib ? "#include <stdlib.h>" EOL : "";
  headers += this->builtins.libString ? "#include <string.h>" EOL : "";

  if (
    this->builtins.libSysTypes ||
    this->builtins.libSysSocket ||
    this->builtins.libSysStat ||
    this->builtins.libSysUtsname
  ) {
    headers += this->builtins.libSysTypes ? "#include <sys/types.h>" EOL : "";
  }

  headers += this->builtins.libSysStat ? "#include <sys/stat.h>" EOL : "";
  headers += this->builtins.libTime ? "#include <time.h>" EOL : "";

  if (
    this->builtins.libWinDirect ||
    this->builtins.libWinIo ||
    this->builtins.libWinWs2tcpip ||
    this->builtins.libWindows ||
    this->builtins.libWinsock2
  ) {
    headers += "#ifdef THE_OS_WINDOWS" EOL;
    headers += this->builtins.libWinDirect ? "  #include <direct.h>" EOL : "";
    headers += this->builtins.libWinIo ? "  #include <io.h>" EOL : "";
    headers += this->builtins.libWinsock2 ? "  #include <winsock2.h>" EOL : "";
    headers += this->builtins.libWinWs2tcpip ? "  #include <ws2tcpip.h>" EOL : "";
    headers += this->builtins.libWindows ? "  #include <windows.h>" EOL : "";
    headers += "#endif" EOL;
  }

  if (
    this->builtins.libArpaInet ||
    this->builtins.libFcntl ||
    this->builtins.libDirent ||
    this->builtins.libNetdb ||
    this->builtins.libNetinetIn ||
    this->builtins.libPthread ||
    this->builtins.libPwd ||
    this->builtins.libSysSocket ||
    this->builtins.libSysUtsname ||
    this->builtins.libUnistd
  ) {
    headers += "#ifndef THE_OS_WINDOWS" EOL;
    headers += this->builtins.libArpaInet ? "  #include <arpa/inet.h>" EOL : "";
    headers += this->builtins.libFcntl ? "  #include <fcntl.h>" EOL : "";
    headers += this->builtins.libDirent ? "  #include <dirent.h>" EOL : "";
    headers += this->builtins.libNetdb ? "  #include <netdb.h>" EOL : "";
    headers += this->builtins.libNetinetIn ? "  #include <netinet/in.h>" EOL : "";
    headers += this->builtins.libPthread ? "  #include <pthread.h>" EOL : "";
    headers += this->builtins.libPwd ? "  #include <pwd.h>" EOL : "";
    headers += this->builtins.libSysSocket ? "  #include <sys/socket.h>" EOL : "";
    headers += this->builtins.libSysUtsname ? "  #include <sys/utsname.h>" EOL : "";
    headers += this->builtins.libUnistd ? "  #include <unistd.h>" EOL : "";
    headers += "#endif" EOL;
  }

  headers += headers.empty() ? "" : EOL;

  auto output = std::string();
  output += banner;
  output += builtinDefineCode;
  output += headers;
  output += builtinExternCode;

  if (!builtinWinExternCode.empty()) {
    output += "#ifdef THE_OS_WINDOWS" EOL;
    output += builtinWinExternCode;
    output += "#endif" EOL EOL;
  }

  output += defineCode;
  output += enumDeclCode;
  output += builtinStructDeclCode;
  output += builtinStructDefCode;
  output += structDeclCode;
  output += structDefCode;
  output += builtinVarCode;
  output += fnDeclCode;
  output += fnDefCode;
  output += "int main (" + std::string(this->needMainArgs ? "int _argc_, char *_argv_[]" : "") + ") ";
  output += cMain.str() + EOL;

  return std::make_tuple(output, this->flags);
}

void Codegen::_node (CodegenASTStmt *c, const ASTNode &node, CodegenPhase phase) {
  if (std::holds_alternative<ASTNodeBreak>(*node.body)) {
    return this->_nodeBreak(c, node);
  } else if (std::holds_alternative<ASTNodeContinue>(*node.body)) {
    return this->_nodeContinue(c, node);
  } else if (std::holds_alternative<ASTNodeEnumDecl>(*node.body)) {
    return this->_nodeEnumDecl(c, node, phase);
  } else if (std::holds_alternative<ASTNodeExpr>(*node.body)) {
    return this->_nodeExprDecl(c, node);
  } else if (std::holds_alternative<ASTNodeFnDecl>(*node.body)) {
    return this->_nodeFnDecl(c, node, phase);
  } else if (std::holds_alternative<ASTNodeIf>(*node.body)) {
    return this->_nodeIf(c, node);
  } else if (std::holds_alternative<ASTNodeLoop>(*node.body)) {
    return this->_nodeLoop(c, node);
  } else if (std::holds_alternative<ASTNodeMain>(*node.body)) {
    return this->_nodeMain(c, node);
  } else if (std::holds_alternative<ASTNodeObjDecl>(*node.body)) {
    return this->_nodeObjDecl(c, node, phase);
  } else if (std::holds_alternative<ASTNodeReturn>(*node.body)) {
    return this->_nodeReturn(c, node);
  } else if (std::holds_alternative<ASTNodeThrow>(*node.body)) {
    return this->_nodeThrow(c, node);
  } else if (std::holds_alternative<ASTNodeTry>(*node.body)) {
    return this->_nodeTry(c, node);
  } else if (std::holds_alternative<ASTNodeVarDecl>(*node.body)) {
    return this->_nodeVarDecl(c, node);
  }
}

void Codegen::_nodeAsync (CodegenASTStmt *c, const ASTNode &node, CodegenPhase phase) {
  if (std::holds_alternative<ASTNodeBreak>(*node.body)) {
    return this->_nodeBreakAsync(c, node);
  } else if (std::holds_alternative<ASTNodeContinue>(*node.body)) {
    return this->_nodeContinueAsync(c, node);
  } else if (std::holds_alternative<ASTNodeExpr>(*node.body)) {
    return this->_nodeExprDeclAsync(c, node);
  } else if (std::holds_alternative<ASTNodeIf>(*node.body)) {
    return this->_nodeIfAsync(c, node);
  } else if (std::holds_alternative<ASTNodeLoop>(*node.body)) {
    return this->_nodeLoopAsync(c, node);
  } else if (std::holds_alternative<ASTNodeMain>(*node.body)) {
    return this->_nodeMainAsync(c, node);
  } else if (std::holds_alternative<ASTNodeReturn>(*node.body)) {
    return this->_nodeReturnAsync(c, node);
  } else if (std::holds_alternative<ASTNodeThrow>(*node.body)) {
    return this->_nodeThrowAsync(c, node);
  } else if (std::holds_alternative<ASTNodeTry>(*node.body)) {
    return this->_nodeTryAsync(c, node);
  } else if (std::holds_alternative<ASTNodeVarDecl>(*node.body)) {
    return this->_nodeVarDeclAsync(c, node);
  } else {
    return this->_node(c, node, phase);
  }
}

CodegenASTExpr Codegen::_nodeExpr (const ASTNodeExpr &nodeExpr, Type *targetType, const ASTNode &parent, CodegenASTStmt *c, bool root, std::size_t awaitCallId) {
  if (std::holds_alternative<ASTExprAccess>(*nodeExpr.body)) {
    return this->_exprAccess(nodeExpr, targetType, parent, c, root);
  } else if (std::holds_alternative<ASTExprArray>(*nodeExpr.body)) {
    return this->_exprArray(nodeExpr, targetType, parent, c, root);
  } else if (std::holds_alternative<ASTExprAssign>(*nodeExpr.body)) {
    return this->_exprAssign(nodeExpr, targetType, parent, c, root);
  } else if (std::holds_alternative<ASTExprAwait>(*nodeExpr.body)) {
    return this->_exprAwait(nodeExpr, targetType, parent, c, root);
  } else if (std::holds_alternative<ASTExprBinary>(*nodeExpr.body)) {
    return this->_exprBinary(nodeExpr, targetType, parent, c, root);
  } else if (std::holds_alternative<ASTExprCall>(*nodeExpr.body)) {
    return this->_exprCall(nodeExpr, targetType, parent, c, root, awaitCallId);
  } else if (std::holds_alternative<ASTExprCond>(*nodeExpr.body)) {
    return this->_exprCond(nodeExpr, targetType, parent, c, root);
  } else if (std::holds_alternative<ASTExprIs>(*nodeExpr.body)) {
    return this->_exprIs(nodeExpr, targetType, parent, c, root);
  } else if (std::holds_alternative<ASTExprLit>(*nodeExpr.body)) {
    return this->_exprLit(nodeExpr, targetType, root);
  } else if (std::holds_alternative<ASTExprMap>(*nodeExpr.body)) {
    return this->_exprMap(nodeExpr, targetType, parent, c, root);
  } else if (std::holds_alternative<ASTExprObj>(*nodeExpr.body)) {
    return this->_exprObj(nodeExpr, targetType, parent, c, root);
  } else if (std::holds_alternative<ASTExprRef>(*nodeExpr.body)) {
    return this->_exprRef(nodeExpr, targetType, parent, c, root);
  } else if (std::holds_alternative<ASTExprUnary>(*nodeExpr.body)) {
    return this->_exprUnary(nodeExpr, targetType, parent, c, root);
  }

  throw Error("tried to generate code for unknown expression");
}

CodegenASTExpr Codegen::_wrapNodeExpr (const ASTNodeExpr &nodeExpr, Type *targetType, bool root, const CodegenASTExpr &expr) {
  auto realTargetType = Type::real(targetType);
  auto realNodeExprType = Type::real(nodeExpr.type);
  auto result = expr;

  if (!root && targetType->isAny() && !realNodeExprType->isAny()) {
    auto typeName = this->_typeNameAny(nodeExpr.type);
    result = CodegenASTExprCall::create(CodegenASTExprAccess::create(this->_(typeName + "_alloc")), {result});
  } else if (!root && realTargetType->isOpt() && (!realNodeExprType->isOpt() || !realTargetType->matchStrict(realNodeExprType))) {
    auto targetTypeInfo = this->_typeInfo(realTargetType);
    auto optionalType = std::get<TypeOptional>(targetTypeInfo.type->body);

    if (Type::real(optionalType.type)->isAny() && !realNodeExprType->isAny()) {
      auto typeName = this->_typeNameAny(realNodeExprType);
      result = CodegenASTExprCall::create(CodegenASTExprAccess::create(this->_(typeName + "_alloc")), {result});
    }

    result = CodegenASTExprCall::create(
      CodegenASTExprAccess::create(this->_(targetTypeInfo.typeName + "_alloc")),
      {result}
    );
  } else if (!root && realTargetType->isUnion() && (!realNodeExprType->isUnion() || !realTargetType->matchStrict(realNodeExprType))) {
    auto typeName = this->_typeNameUnion(realTargetType);
    auto defName = this->_typeDef(nodeExpr.type);

    result = CodegenASTExprCall::create(
      CodegenASTExprAccess::create(this->_(typeName + "_alloc")),
      {CodegenASTExprAccess::create(this->_(defName)), result}
    );
  }

  if (nodeExpr.parenthesized) {
    result = result.wrap();
  }

  return result;
}
