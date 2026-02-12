// Command to run:
// ./build/bin/clang --analyze --analyzer-no-default-checks -Xanalyzer -analyzer-checker=security.insecureAPI.lstrcpyA 3.c

char *lstrcpyA(const char *, const char *);

#define SOURCE_SIZE 16
#define TARGET_SIZE 16

int main() {
    char source[SOURCE_SIZE];
    char target[TARGET_SIZE];
    lstrcpyA(target, source);
    lstrcpyA(target, "string litral longer than 16 characters");

    return 0;
}
