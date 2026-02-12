int atof(char s[]){}

void test() {
 char s[]="12asd3";
 atof(s); // expected warning: Not a proper numeric string!
}

