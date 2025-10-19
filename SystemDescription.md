# Developing new semantic checks within *Clang*

Authors (alphabetical):
- Mina Dograjić
- Stefan Milenković
- Dimitrije Radjenović

The project comprises three separate subprojects.
Two of them implement new *Clang* checkers for mathematical functions, and the third addresses an open *GitHub* issue for *Clang Static Analyzer*.

## Subproject A: Checker for examining the parameters of functions `asin` and `acos`

TODO: Fill in.

## Subproject B: Checker for examining the parameters of functions `atoi` and `atof`

TODO: Fill in.

## Subproject C: Extending the list of unsafe functions in *Clang*

The third part of the project is to extend the existing list of checkers for unsafe function calls in `security.insecureAPI` checker family.
More details can be found in the [corresponding *GitHub* issue][issue-103038], and the [first][pr-mark-insecure] and [second][pr-add-option] opened pull requests resolving the issue.

Currently, the security.insecureAPI checkers issue a warning whenever they encounter a call to an unsafe function (such as strcat). Most of the commonly known unsafe functions are already covered for Unix-based operating systems. However, their Windows-specific counterparts are not included.

To address this limitation, we extend the checker in two ways. First, we add inspections for Windows functions that are considered insecure, specifically _strdup, lstrcatA, and lstrcpyA. Second, we introduce an option that allows users to manually specify additional function names that should be treated as potentially dangerous via a command-line argument at the checker’s invocation.

### Checks for `_strdup`, `lstrcatA`, and `lstrcpyA`

#### Explanation

The checker issues a warning when a function call to a function with one of the following declaration is encountered:
```
char *_strdup(const char *);
char *lstrcatA(const char *, const char *);
char *lstrcpyA(const char *, const char *);
```
We do this by checking the function return type, and the types of the function arguments.
In the case of `_strdup`, it is done in the following way.

First, we extract the type of the argument from `FunctionDecl *` object.
```cpp
const FunctionProtoType *FPT = FD->getType()->getAs<FunctionProtoType>();
if (!FPT)
  return;

// Verify the function takes one argument
int numArgs = FPT->getNumParams();
if (numArgs != 1)
  return;

// Verify that the argument is a pointer
const PointerType *PT = FPT->getParamType(0)->getAs<PointerType>();
if (!PT)
  return;
```
Then check that the type is appropriate:
```cpp
// Verify that the argument is a 'const char*'.
if (PT->getPointeeType().getUnqualifiedType() != BR.getContext().CharTy)
  return;
if (!PT->getPointeeType().isConstQualified())
  return;
```
If the function at no point returns, it means that we have found a function with the same declaration and name as `_strdup` and we issue a warning.
```cpp
PathDiagnosticLocation CELoc =
  PathDiagnosticLocation::createBegin(CE, BR.getSourceManager(), AC);
BR.EmitBasicReport(AC->getDecl(), filter.checkName_strdup,
                   "Potential unfreed memory from call of '_strdup'",
                   "Security",
                   "Call to function '_strdup' allocates memory on heap "
                   "and requires to be freed manually.",
                   CELoc, CE->getCallee()->getSourceRange());
```

The other two functions are checked in a similar manner, with `lstrcpyA` requiring an additional check on the source argument.
Specifically, if the second argument is a string literal and the first (target) argument is an array with sufficient space to hold that literal, no warnings are issued.
```cpp
const auto *Target = CE->getArg(0)->IgnoreImpCasts(),
           *Source = CE->getArg(1)->IgnoreImpCasts();

if (const auto *Array = dyn_cast<ConstantArrayType>(Target->getType())) {
  uint64_t ArraySize = BR.getContext().getTypeSize(Array) / 8;
  if (const auto *String = dyn_cast<StringLiteral>(Source)) {
    if (ArraySize >= String->getLength() + 1)
      return;
  }
}
```

#### Running the checker

The checker can be tested by running the following command:
```sh
$ ./build/bin/clang --analyze --analyzer-no-default-checks -Xanalyzer -analyzer-checker=security.insecureAPI.strdup test.c
```

*Note*: These checkers issue warnings only on *Windows*.

### Command line argument for user-defined dangerous functions

#### Explanation

Another component of this subproject is the ability for users to specify functions that should trigger a warning when invoked. To support this functionality, the `SecuritySyntaxChecker` provides a command-line argument `Warn` allowing users to supply a space-separated list of such functions.

That functionality is achieved by storing the list of functions in an internal variable of type `StringSet<>`.
Then the checker issues a warning when it explores a call node with the name of the callee that belongs to the set of function names.
```cpp
IdentifierInfo *II = FD->getIdentifier();
if (!II)   // if no identifier, not a simple C function
  return;
StringRef Name = II->getName();
Name.consume_front("__builtin_");

if (!(this->WarnFunctions.contains(Name)))
  return;
```

#### Running the checker

To try the checker, run the following command and replace `a b c` with the appropriate list of functions
```sh
$ ./build/bin/clang --analyze --analyzer-no-default-checks -Xanalyzer -analyzer-checker=security.insecureAPI.SecuritySyntaxChecker -Xclang -analyzer-config -Xclang security.insecureAPI.SecuritySyntaxChecker:Warn="a b c" test.c
```
In this example, `a`, `b`, and `c` denote function names for which warnings will be issued.

[issue-103038]: https://github.com/llvm/llvm-project/issues/103038
[pr-mark-insecure]: https://github.com/llvm/llvm-project/pull/164183
[pr-add-option]: https://github.com/llvm/llvm-project/pull/164184
