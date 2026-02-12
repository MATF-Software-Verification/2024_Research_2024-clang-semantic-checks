// Command to run:
// ./build/bin/clang --analyze --analyzer-no-default-checks -Xanalyzer -analyzer-checker=security.insecureAPI.lstrcatA 2.c

char *lstrcatA(const char *, const char *);

#define SOURCE_SIZE 16
#define TARGET_SIZE 16

int main() {
    char source[SOURCE_SIZE];
    char target[TARGET_SIZE];
    lstrcatA(target, source);

    return 0;
}
