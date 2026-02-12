# FuncDomainChecker
FuncDomainChecker is a Clang Static Analyzer checker that checks the validity of inputs od certain functions, more
specifically it checks if the input arguments of functions are in the proper function domain. As of right now it checks
functions: atoi,atof,asin and acos. Furhter information is provided in SystemDescription.md. 

Author: Dimitrije Radjenovic 1084/2024

## Setup
- Build [LLVM](https://github.com/llvm/llvm-project) as described in README
- Copy `FuncDomainChecker.cpp` to the `llvm-project/clang/lib/StaticAnalyzer/Checkers` folder
- Extend `llvm-project/clang/lib/StaticAnalyzer/Checkers/CMakeLists.txt` with `FuncDomainChecker.cpp`
- Add following block of code into `Checkers.td` file contained in `llvm-project/clang/include/clang/StaticAnalyzer/Checkers`: 
```
let ParentPackage = UnixAlpha in {
    ...
    def FuncDomainChecker : Checker<"FuncDomainCheckerr">,  
      HelpText<"Check functions arguments">,  
      Documentation<NotDocumented>;
    ...
} // end "alpha.unix" 
```
- Rebuild project

## Usage
For applications written in C, run: ` clang --analyze -Xanalyzer -analyzer-checker=unix.alpha.FuncDomainChecker example.c `

For applications written in C++, run: ` clang++ --analyze -Xanalyzer -analyzer-checker=unix.alpha.FuncDomainChecker example.cpp `

Both `clang` and `clang++` can be found in `llvm-project/build/bin`.
