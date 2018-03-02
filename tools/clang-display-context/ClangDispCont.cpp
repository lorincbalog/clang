//===- ClangDispCont.cpp ------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===--------------------------------------------------------------------===//
//
// Clang tool which generates and outputs an issue string based on SA,
// from the location (line, column number) of the issue.
//
//===--------------------------------------------------------------------===//

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/ExprCXX.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/StaticAnalyzer/Core/IssueHash.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Signals.h"

#include <regex>
#include <string>

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

static cl::OptionCategory ClangDispContCategory("clang-disp-context options");

static cl::opt<uint> Line(
    "line",
    cl::desc("Line number of the issue's position. \n"),
    cl::cat(ClangDispContCategory));

static cl::opt<uint> Col(
    "col",
    cl::desc("Column number of the issue's position. \n"),
    cl::cat(ClangDispContCategory));

class DisplayContextConsumer : public ASTConsumer {
public:
  DisplayContextConsumer(ASTContext &Context) : Ctx(Context) {}

  ~DisplayContextConsumer() {
    // Flush results to standard output.
    outs() << IssueString;
  }

  virtual void HandleTranslationUnit(ASTContext &Ctx) {
    if (!Ctx.getDiagnostics().hasErrorOccurred())
      handleDecl(Ctx.getTranslationUnitDecl());
  }

private:
  template <typename T> 
  bool checkIfInteresting(const T *N);
  void handleDecl(const Decl *D);
  void handleStmt(const Stmt *S);
  void setEnclosingDecl(const Decl *D);
  void setIssueString(const std::string &S);
  
  ASTContext &Ctx;
  FullSourceLoc IssueLoc;
  const Decl *EnclosingDecl;
  std::string IssueString;
};

template <typename T>
bool DisplayContextConsumer::checkIfInteresting(const T *Node) {
  // Nodes are only interesting if they contain the line with the issue
  if (!Node)
    return false;

  FullSourceLoc L1 = Ctx.getFullLoc(Node->getLocStart());
  FullSourceLoc L2 = Ctx.getFullLoc(Node->getLocEnd());

  return L1.isValid() && L2.isValid() &&
         Line <= L2.getExpansionLineNumber() &&
         L1.getExpansionLineNumber() <= Line;
}

void DisplayContextConsumer::handleDecl(const Decl *D) {
  if (!D)
    return;

  if (const auto *TD = dyn_cast<TemplateDecl>(D))
    D = TD->getTemplatedDecl();
  
  if (const auto *DC = dyn_cast<DeclContext>(D)) {
    for (const Decl *D : DC->decls()) {
      if (!checkIfInteresting<Decl>(D))
        continue;
      handleDecl(D);
      return;
    }
  }
  
  setEnclosingDecl(D);
  IssueLoc = Ctx.getFullLoc(D->getLocation()).getExpansionLoc();
  
  handleStmt(D->getBody());
  setIssueString(clang::GetIssueString(Ctx.getSourceManager(), IssueLoc, "",
                                       "", EnclosingDecl, Ctx.getLangOpts()));
}

void DisplayContextConsumer::handleStmt(const Stmt *S) {
  if (!checkIfInteresting<Stmt>(S))
    return;

  IssueLoc = Ctx.getFullLoc(S->getLocStart()).getExpansionLoc();
  
  if (const LambdaExpr *LE = dyn_cast<LambdaExpr>(S))
    EnclosingDecl = LE->getCallOperator();

  for (const Stmt *Child : S->children())
    handleStmt(Child);
}

void DisplayContextConsumer::setEnclosingDecl(const Decl *D) {
  // If the issue is located in a DeclaratorDecl which is not
  // a FunctionDecl, traverse up until the enclosing context is found.
  // This function aims to solve issues especially with VarDecls.
  while (dyn_cast<DeclaratorDecl>(D) && !dyn_cast<FunctionDecl>(D)) {
    D = dyn_cast<Decl>(D->getDeclContext());
  }
  EnclosingDecl = D;
}

void DisplayContextConsumer::setIssueString(const std::string &SAIssue) {
  // Since Location is provided as a range, the IssueString from Clang SA
  // has to be modified by replacing the column number, and removing
  // the first and last delimeters (CheckerName and BugType not provided)
  IssueString = std::regex_replace(SAIssue.substr(1, SAIssue.size()-2),
                                   std::regex(R"(\$\d+\$)"),
                                   "$$" + Twine(Col).str() + "$$");
}

class DisplayContextAction : public ASTFrontendAction {
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef) {
    std::unique_ptr<ASTConsumer> PFC(
        new DisplayContextConsumer(CI.getASTContext()));
    return PFC;
  }
};

int main(int argc, const char **argv) {
  sys::PrintStackTraceOnErrorSignal(argv[0], false);
  PrettyStackTraceProgram X(argc, argv);

  const char *Overview = "\n This tool generates and prints an issue string "
                         "(similar to Clang Static Analyzer's IssueString) "
                         "from the position of the issue in the source. \n";
  CommonOptionsParser OptionsParser(argc, argv, ClangDispContCategory,
                                    cl::ZeroOrMore, Overview);

  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());
  return Tool.run(newFrontendActionFactory<DisplayContextAction>().get());
}