set(LLVM_LINK_COMPONENTS
  Option
  Support
  )

add_subdirectory(Core)
add_subdirectory(Refactoring)

add_clang_library(clangTooling
  ArgumentsAdjusters.cpp
  CommonOptionsParser.cpp
  CompilationDatabase.cpp
  CrossTranslationUnit.cpp
  FileMatchTrie.cpp
  FixIt.cpp
  JSONCompilationDatabase.cpp
  Refactoring.cpp
  RefactoringCallbacks.cpp
  Tooling.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangAST
  clangASTMatchers
  clangBasic
  clangDriver
  clangFormat
  clangFrontend
  clangIndex
  clangLex
  clangRewrite
  clangToolingCore
  )
