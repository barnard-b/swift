//===--- Stmt.cpp - Swift Language Statement ASTs -------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
//  This file implements the Stmt class and subclasses.
//
//===----------------------------------------------------------------------===//

#include "swift/AST/Stmt.h"
#include "swift/AST/ASTContext.h"
#include "swift/AST/ASTVisitor.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/Support/raw_ostream.h"
using namespace swift;

//===----------------------------------------------------------------------===//
// Stmt methods.
//===----------------------------------------------------------------------===//

// Only allow allocation of Stmts using the allocator in ASTContext.
void *Stmt::operator new(size_t Bytes, ASTContext &C,
                         unsigned Alignment) throw() {
  return C.Allocate(Bytes, Alignment);
}

/// getLocStart - Return the location of the start of the expression.
/// FIXME: Need to extend this to do full source ranges like Clang.
SMLoc Stmt::getLocStart() const {
  switch (Kind) {
  case StmtKind::Semi:
    return cast<SemiStmt>(this)->Loc;
  case StmtKind::Assign:
    return cast<AssignStmt>(this)->Dest->getStartLoc();
  case StmtKind::Brace:
    return cast<BraceStmt>(this)->LBLoc;
  case StmtKind::Return:
    return cast<ReturnStmt>(this)->ReturnLoc;
  case StmtKind::If:
    return cast<IfStmt>(this)->IfLoc;
  case StmtKind::While:
    return cast<WhileStmt>(this)->WhileLoc;
  }
  
  assert(0 && "Not reachable, all cases handled");
  abort();
}

//===----------------------------------------------------------------------===//
// Printing for Stmt and all subclasses.
//===----------------------------------------------------------------------===//

namespace {
/// PrintStmt - Visitor implementation of Expr::print.
class PrintStmt : public StmtVisitor<PrintStmt> {
public:
  raw_ostream &OS;
  unsigned Indent;
  
  PrintStmt(raw_ostream &os, unsigned indent) : OS(os), Indent(indent) {
  }
  
  void printRec(Stmt *S) {
    Indent += 2;
    if (S)
      visit(S);
    else
      OS.indent(Indent) << "(**NULL STATEMENT**)";
    Indent -= 2;
  }
  
  void printRec(Decl *D) { D->print(OS, Indent+2); }
  void printRec(Expr *E) { E->print(OS, Indent+2); }
  
  void visitSemiStmt(SemiStmt *S) {
    OS.indent(Indent) << "(semi_stmt)";
  }

  void visitAssignStmt(AssignStmt *S) {
    OS.indent(Indent) << "(assign_stmt\n";
    printRec(S->Dest);
    OS << '\n';
    printRec(S->Src);
    OS << ')';
  }

  void visitBraceStmt(BraceStmt *S) {
    OS.indent(Indent) << "(brace_stmt";
    for (unsigned i = 0, e = S->NumElements; i != e; ++i) {
      OS << '\n';
      if (Expr *SubExpr = S->Elements[i].dyn_cast<Expr*>())
        printRec(SubExpr);
      else if (Stmt *SubStmt = S->Elements[i].dyn_cast<Stmt*>())
        printRec(SubStmt);
      else
        printRec(S->Elements[i].get<Decl*>());
    }
    OS << ')';
  }
  
  void visitReturnStmt(ReturnStmt *S) {
    OS.indent(Indent) << "(return_stmt\n";
    printRec(S->Result);
    OS << ')';
  }
  
  void visitIfStmt(IfStmt *S) {
    OS.indent(Indent) << "(if_stmt\n";
    printRec(S->Cond);
    OS << '\n';
    printRec(S->Then);
    if (S->Else) {
      OS << '\n';
      printRec(S->Else);
    }
    OS << ')';
  }
  void visitWhileStmt(WhileStmt *S) {
    OS.indent(Indent) << "(while_stmt\n";
    printRec(S->Cond);
    OS << '\n';
    printRec(S->Body);
    OS << ')';
  }
};

} // end anonymous namespace.

void Stmt::dump() const {
  print(llvm::errs());
  llvm::errs() << '\n';
}

void Stmt::print(raw_ostream &OS, unsigned Indent) const {
  PrintStmt(OS, Indent).visit(const_cast<Stmt*>(this));
}
