# FuncDomainChecker
Author: 
  - Dimitrije Radjenovic
## Problem Description
Many functions, including a large amount of C standard library functions, have strict domain requirements that, when violated, lead to undefined behaviour 
or incorrect results. These violations are typically detected only at runtime, often leading to subtle bugs. Existing compilers provide minimal compile-time 
validation for these cases. Our solution to this problem is to create a path sensitive clang checker which uses symbolic execution. The checker only
works for functions: atoi,atof,asin and acos , but it can be extended to include many more functions.
## Design 
Our checker is implemented using `check::PreStmt<CallExpr>` . It fires every time the analyzer engine is about to  
analyze a statement of class CallExpr. First we check if the given `CallExpr` object coresponds to function call using `FunctionDecl *` object, 
and afterwords we check the name of the function using `CheckerContext`. 
```cpp
const FunctionDecl *FD = C.getCalleeDecl(CE);
  if (!FD || FD->getKind() != Decl::Function)
    return;

  StringRef FName = C.getCalleeName(FD);
  if (FName.empty())
    return;

```
After that we perform simple string comparisons to check if we find our functions, and if so we use our helper functions 
to handle specific cases.
```cpp
 if (FName == "asin" || FName == "acos")
    checkRangeMinusOnetoOne(CE, C);
  else if (FName == "atoi")
    checkAtoiArgs(CE, C);
   else if (FName == "atof")
    checkAtofArgs(CE,C);
  else
    return;
```
## checkAtofArgs and checkAtoiArgs
After we get the function argument we check if it's a compile-time known string value,a direct string literal (e.g. "123") or an
initialized character array variable. We do that by creating
a `StringLiteral *` object:
```cpp
const StringLiteral *StrLiteral = dyn_cast<StringLiteral>(Arg);
```
1.) if `StrLiteral` is not `null`, we know that it is a direct string literal and simply send it to our custom function which checks
if argument is a proper numeric string (`isRealNumber` or `isInteger`). If not, we send a warning (use the ReportBug function).

2.) if `StrLiteral` is `null`, we know that it is a variable and use a series of casts to get the initialization:
```cpp
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
```
After we get the Initialization, we extract the StringLiteral and basically perform the final steps of 1.) .
## checkRangeMinusOnetoOne
After retrieving the argument, we retrieve its symbolic value. In order to create proper constraints, because
we are using symbolic values, we need to use `SValBuilder`.
First we create symbolic constants -1 and 1 :
```cpp
NonLoc NegOne = SVB.makeIntVal(-1, ArgTy).castAs<NonLoc>();
NonLoc One = SVB.makeIntVal(1, ArgTy).castAs<NonLoc>();
```
And after that we create proper symbolic constraints (x>=-1 and x<=1):
```cpp
SVal GeNegOne = SVB.evalBinOp(state, BO_GE, ArgVal.castAs<NonLoc>(), NegOne, SVB.getConditionType());
SVal LeOne   = SVB.evalBinOp(state, BO_LE, ArgVal.castAs<NonLoc>(), One, SVB.getConditionType());
```
After making sure that we are not dealing with undefined values , we use the `ConstraintManager`
to check the constraints. To do this we use the `assumeDual` method. The method returns two states,
one which represent a path where the constraint holds and one where it doesn't. If it returns a 
`nullptr` for one of them it means that a path is infeasible , given the current constraints.

```cpp
ConstraintManager &CM = C.getConstraintManager();

ProgramStateRef StGeTrue, StGeFalse;
std::tie(StGeTrue, StGeFalse) = CM.assumeDual(state, *GeCond);

ProgramStateRef StLeTrue, StLeFalse;
std::tie(StLeTrue, StLeFalse) = CM.assumeDual(state, *LeCond);
```
Now we differentiate cases:
```cpp
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
```
## ReportBug
The ReportBug method emits a path-sensitive diagnostic when a domain violation is detected.
First it atempts to create a new ExplodedNode in the ExplodedGraph, and with the `generateNonFatalErrorNode`
metod, we can can do it without terminating symbolic execution. If it fails it exits silently.
```cpp
  ExplodedNode *N = C.generateNonFatalErrorNode(State);
  if (!N) return;
```
Next it constructs a `PathSensitiveBugReport` anchored to the generated node, which preserves the full execution 
path leading to the error, and by adding the `SourceRange` we can get precise visual higlighting.
In the end we emit the report.
```cpp
  auto Report = std::make_unique<PathSensitiveBugReport>(*BT, SpecificMessage, N);
  Report->addRange(SR);
  C.emitReport(std::move(Report));
```


