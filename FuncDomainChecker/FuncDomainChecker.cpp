#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include <iostream>
#include <utility>
#include<optional>
#include<cctype>
#include<map>
using namespace clang;
using namespace ento;

static bool isRealNumber(llvm::StringRef Str) {
  if (Str.empty()) return false;

  size_t i = 0;
  if (Str[i] == '+' || Str[i] == '-') {
    ++i;
    if (i == Str.size()) return false; 
  }

  bool hasDigit = false;
  bool hasDot = false;
  size_t length= Str.size();
  for (; i < length; ++i) {
    char c = Str[i];
    if (c >= '0' && c <= '9') {
      hasDigit = true;
    } else if (c == '.') {
      if (hasDot) return false; 
      hasDot = true;
    } else {
      return false; 
    }
  }

  return hasDigit; 
}

static bool isInteger(llvm::StringRef Str) {
  if (Str.empty()) return false;

  size_t i = 0;
  if (Str[i] == '+' || Str[i] == '-') {
    ++i;
    if (i == Str.size()) return false; 
  }

  bool hasDigit = false;

  for (; i < Str.size(); ++i) {
    char c = Str[i];
    if (c >= '0' && c <= '9') {
      hasDigit = true;
    }  else {
      return false; 
    }
  }

  return hasDigit; 
}




class FuncDomainChecker : public Checker<check::PreStmt<CallExpr>> {
  mutable std::unique_ptr<BugType> RangeBugType;
  mutable std::unique_ptr<BugType> AtoiBugType;
  mutable std::unique_ptr<BugType> AtofBugType;
  void ReportBug(CheckerContext &C, ProgramStateRef State, SourceRange SR,
                 std::unique_ptr<BugType> &BT,StringRef SpecificMessage) const;
  void checkRangeMinusOnetoOne(const CallExpr *CE, CheckerContext &C) const;
  void checkAtoiArgs(const CallExpr *CE, CheckerContext &C) const;
  void checkAtofArgs(const CallExpr *CE, CheckerContext &C) const;
public:
  FuncDomainChecker();
  void checkPreStmt(const CallExpr *CE, CheckerContext &C) const;
};

FuncDomainChecker::FuncDomainChecker() {
  RangeBugType = std::make_unique<BugType>(
      this, "Invalid asin/acos argument", "Range error");
  AtoiBugType = std::make_unique<BugType>(
      this, "Invalid atoi argument", "String domain error");
  AtofBugType = std::make_unique<BugType>(
      this, "Invalid atof argument", "String domain error");
}

void FuncDomainChecker::ReportBug(CheckerContext &C, ProgramStateRef State,
                                     SourceRange SR,
                                     std::unique_ptr<BugType> &BT,
                                     StringRef SpecificMessage) const {
  ExplodedNode *N = C.generateNonFatalErrorNode(State);
  if (!N) return;

  auto Report = std::make_unique<PathSensitiveBugReport>(*BT, SpecificMessage, N);
  Report->addRange(SR);
  C.emitReport(std::move(Report));
}
 
void FuncDomainChecker::checkPreStmt(const CallExpr *CE,
                                        CheckerContext &C) const {

  const FunctionDecl *FD = C.getCalleeDecl(CE);
  if (!FD || FD->getKind() != Decl::Function)
    return;

  StringRef FName = C.getCalleeName(FD);
  if (FName.empty())
    return;
    
  if (FName == "asin" || FName == "acos")
    checkRangeMinusOnetoOne(CE, C);
  else if (FName == "atoi")
    checkAtoiArgs(CE, C);
   else if (FName == "atof")
    checkAtofArgs(CE,C);
  else
    return;
}

void FuncDomainChecker::checkAtoiArgs(const CallExpr *CE,
                                         CheckerContext &C) const {
  // Ensure at least 1 argument exists
  if (CE->getNumArgs() < 1)
    return;

  // Get the first argument (format string)
  ProgramStateRef state = C.getState();
  const Expr *Arg = CE->getArg(0)->IgnoreParenCasts();
  const StringLiteral *StrLiteral = dyn_cast<StringLiteral>(Arg);

  //Check if StringLiteral
  if (StrLiteral)
   { 


  StringRef Str = StrLiteral->getString();
  
  	if(!isInteger(Str))
	ReportBug(C, state, Arg->getSourceRange(), AtoiBugType,
          "this is not a proper numeric string!");

    return;       
  }
 //If it's a paramater 
 const DeclRefExpr *DRE=dyn_cast<DeclRefExpr>(Arg);
  if(!DRE)
  return;
 
  const VarDecl *VD=dyn_cast<VarDecl>(DRE->getDecl());
  if(!VD)
  return;
 
  if(!VD->getType()->isArrayType())
  return;
 
  const Expr *Init=VD->getInit();
  if(!Init)
  return;
 
  Init=Init->IgnoreParenCasts();
  StrLiteral=dyn_cast<StringLiteral>(Init);
 
  if(!StrLiteral)
  return;

  StringRef StrVal=StrLiteral->getString();
    if(!isInteger(StrVal))
    ReportBug(C, state, Arg->getSourceRange(), AtoiBugType,
          "this is not a  proper numeric string!");
  
 
}


void FuncDomainChecker::checkAtofArgs(const CallExpr *CE,
                                         CheckerContext &C) const {

  // Ensure at least 1 argument exists
  if (CE->getNumArgs() < 1)
    return;

  // Get the first argument (format string)
  ProgramStateRef state = C.getState();
  const Expr *Arg = CE->getArg(0)->IgnoreParenCasts();
  const StringLiteral *StrLiteral = dyn_cast<StringLiteral>(Arg);

  //Check if StringLiteral
  if (StrLiteral)
   { 

  
  StringRef Str = StrLiteral->getString();
  if(!isRealNumber(Str)){
  ReportBug(C, state, Arg->getSourceRange(), AtofBugType,
          "this is not a proper numeric string!");}}

  //If it's a paramater
  const DeclRefExpr *DRE=dyn_cast<DeclRefExpr>(Arg);
  if(!DRE)
  return;
 
  const VarDecl *VD=dyn_cast<VarDecl>(DRE->getDecl());
  if(!VD)
  return;
 
  if(!VD->getType()->isArrayType())
  return;
 
  const Expr *Init=VD->getInit();
  if(!Init)
  return;
 
  Init=Init->IgnoreParenCasts();
  StrLiteral=dyn_cast<StringLiteral>(Init);
 
  if(!StrLiteral)
  return;
 
  StringRef StrVal=StrLiteral->getString();
    if(!isRealNumber(StrVal)){
  ReportBug(C, state, Arg->getSourceRange(), AtofBugType,
          "this is not a proper numeric string!");}
 
}


void FuncDomainChecker::checkRangeMinusOnetoOne(const CallExpr *CE,
                                         CheckerContext &C) const {
    
 ProgramStateRef state = C.getState();
 const Expr *Arg = CE->getArg(0);
  if (!Arg)
    return;

  SVal ArgVal = C.getSVal(Arg);

  // Symbolic case: build constraints
  SValBuilder &SVB = C.getSValBuilder();
  QualType ArgTy = Arg->getType();

  //if (!ArgTy->isIntegralOrEnumerationType()) {
    // std::cout << "Skipping non-integral type\n";
  //  return;

  // Create constant values -1 and 1 
  NonLoc NegOne = SVB.makeIntVal(-1, ArgTy).castAs<NonLoc>();
  NonLoc One = SVB.makeIntVal(1, ArgTy).castAs<NonLoc>();

  // Build  constraints Arg >= -1  and  Arg <= 1
  SVal GeNegOne = SVB.evalBinOp(state, BO_GE, ArgVal.castAs<NonLoc>(), NegOne, SVB.getConditionType());
  SVal LeOne   = SVB.evalBinOp(state, BO_LE, ArgVal.castAs<NonLoc>(), One, SVB.getConditionType());

  if (auto GeCond = GeNegOne.getAs<DefinedSVal>()) {
    if (auto LeCond = LeOne.getAs<DefinedSVal>()) {
      ConstraintManager &CM = C.getConstraintManager();

      // Check lower bound: x >= -1 
      ProgramStateRef StGeTrue, StGeFalse;
      std::tie(StGeTrue, StGeFalse) = CM.assumeDual(state, *GeCond);

      // Check upper bound: x <= 1 
      ProgramStateRef StLeTrue, StLeFalse;
      std::tie(StLeTrue, StLeFalse) = CM.assumeDual(state, *LeCond);
      

      // Case 1: Definitely out of range (x < -1)
      if (!StGeTrue && StGeFalse) {
      	ReportBug(C, state, Arg->getSourceRange(), RangeBugType,
          "ERROR: argument is definitely < -1");
        return;
      }

      // Case 2: Definitely out of range (x > 1)
      if (!StLeTrue && StLeFalse) {
        ReportBug(C, state, Arg->getSourceRange(), RangeBugType,
          "ERROR: argument is definitely > 1");
        return;
      }
      
      // Case 3: Could be out of range (both in-range and out-of-range paths exist)
      if ((StGeFalse || StLeFalse) && (StGeTrue || StLeTrue)) {
       ReportBug(C, state, Arg->getSourceRange(), RangeBugType,
          "WARNING: argument might be outside of range ");
        return;
      }

     
    }
  }
  
}


void ento::registerFuncDomainChecker(CheckerManager &mgr) {
  mgr.registerChecker<FuncDomainChecker>();
}

// This checker should be enabled regardless of how language options are set.
bool ento::shouldRegisterFuncDomainChecker(const CheckerManager &mgr) {
  return true;
}
