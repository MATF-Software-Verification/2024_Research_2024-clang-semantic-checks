# Semantic checks within *Clang*

Authors (alphabetical):
- [Mina Dograjić][mina-dograjic]
- [Stefan Milenković][stemil01]
- [Dimitrije Radjenović][domeGIT]

The project comprises three separate subprojects.
Two of them implement new *Clang* checkers for mathematical functions, and the third addresses an open *GitHub* issue for *Clang Static Analyzer*.

Description on how each checker works can be found in the file `SystemDescription.md`.

## Subproject A: Checker for examining the parameters of functions `asin` and `acos`

Brief explanation what the checker does.

### Usage

To try out the checker do ...

## Subproject B: Checker for examining the parameters of functions `atoi` and `atof`

Brief explanation what the checker does.

### Usage

To try out the checker do ...

## Subproject C: Extending the list of unsafe functions in *Clang*

The third part of the project is to extend the existing list of checkers for unsafe function calls in `security.insecureAPI` checker family.
More details can be found in the [corresponding *GitHub* issue][issue-103038], and the [first][pr-mark-insecure] and [second][pr-add-option] opened pull requests resolving the issue.

In short, we extended the family of security.insecureAPI checkers with three new checkers, each of which issues a warning when it encounters `_strdup`, `lstrcatA`, or `lstrcpyA`.
Moreover, we added a command line argument by which a user can provide a list of functions the checker should issue a warning about.

### Usage

To run the checker follow the steps:
1. Checkers need to be registered in file `clang/include/clang/StaticAnalyzer/Checkers/Checkers.td`.
To do so, add the following checker declarations in the said file in `InsecureAPI` section:
    ```
    def SecuritySyntaxChecker : Checker<"SecuritySyntaxChecker">,
      HelpText<"Base of various security function related checkers">,
      CheckerOptions<[
        CmdLineOption<String,
                      "Warn",
                      "List of space-separated function name to be warned about. "
                      "Defaults to an empty list.",
                      "",
                      InAlpha>
      ]>,
      Documentation<NotDocumented>,
      Hidden;

    def strdup : Checker<"strdup">,
      HelpText<"Warn on uses of the '_strdup' function">,
      Dependencies<[SecuritySyntaxChecker]>,
      Documentation<HasDocumentation>;

    def lstrcatA : Checker<"lstrcatA">,
      HelpText<"Warn on uses of the 'lstrcatA' function">,
      Dependencies<[SecuritySyntaxChecker]>,
      Documentation<HasDocumentation>;

    def lstrcpyA : Checker<"lstrcpyA">,
      HelpText<"Warn on uses of the 'lstrcpyA' function">,
      Dependencies<[SecuritySyntaxChecker]>,
      Documentation<HasDocumentation>;
    ```

2. Then copy the file `SubprojectC/CheckSecuritySyntaxOnly.cpp` to `clang/lib/StaticAnalyzer/Checkers`

3. Recompile the project (consult the official [LLVM documentation][building-llvm-docs] on how to do so)

The checker can be tested by running the following command:
```sh
$ ./build/bin/clang --analyze --analyzer-no-default-checks -Xanalyzer -analyzer-checker=security.insecureAPI.strdup test.c
```
Replace the word `strdup` with `lstrcatA` or `lstrcpyA` to try out the other two checkers.

Another component of this subproject is the ability for users to specify functions that should trigger a warning when invoked. To support this functionality, the SecuritySyntaxChecker provides a command-line interface of the following form:
```sh
$ ./build/bin/clang --analyze --analyzer-no-default-checks -Xanalyzer -analyzer-checker=security.insecureAPI.SecuritySyntaxChecker -Xclang -analyzer-config -Xclang security.insecureAPI.SecuritySyntaxChecker:Warn="a b c" test.c
```
In this example, `a`, `b`, and `c` denote function names for which warnings will be issued.

[mina-dograjic]: https://github.com/mina-dograjic
[stemil01]: https://github.com/stemil01
[domeGIT]: https://github.com/domeGIT
[issue-103038]: https://github.com/llvm/llvm-project/issues/103038
[building-llvm-docs]: https://llvm.org/docs/CMake.html
[pr-mark-insecure]: https://github.com/llvm/llvm-project/pull/164183
[pr-add-option]: https://github.com/llvm/llvm-project/pull/164184
