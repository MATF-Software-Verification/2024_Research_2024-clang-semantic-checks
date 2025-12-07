// Command to run:
// ./build/bin/clang --analyze --analyzer-no-default-checks -Xanalyzer -analyzer-checker=security.insecureAPI.strdup 1.c

char *_strdup(const char *);

#define SOURCE_SIZE 16
#define TARGET_SIZE 16

int main() {
    char source[SOURCE_SIZE];
    char target[TARGET_SIZE];
    _strdup(source);

    return 0;
}
