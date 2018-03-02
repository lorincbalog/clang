set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  asmparser
  support
  mc
  )

add_clang_executable(clang-display-context
  ClangDispCont.cpp
  )

target_link_libraries(clang-display-context
  clangTooling
  clangAST
  clangBasic
  clangFrontend
  clangStaticAnalyzerCore
  )

install(TARGETS clang-display-context
  RUNTIME DESTINATION bin)