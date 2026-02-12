// Command to run:
// ./build/bin/clang --analyze --analyzer-no-default-checks -Xanalyzer -analyzer-checker=security.insecureAPI.SecuritySyntaxChecker -Xclang -analyzer-config -Xclang security.insecureAPI.SecuritySyntaxChecker:Warn="a b c" 4.c
#include <stdio.h>

void a(int);
double b();
char *c(double, int *);

int main() {
    int x = 2;
    a(x);
    printf("%lf\n", b());
    printf("%s\n", c(3.14, &x));

    return 0;
}
