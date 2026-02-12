int atof(char s[]){}

void test() {
 char s[]="-123.123";
 atof(s); // no warning expected
}

