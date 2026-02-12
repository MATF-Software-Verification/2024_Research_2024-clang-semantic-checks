int acos(int x){}


void test() {
 int x = 123;
 acos(x);// expected warning: argument is definitely > 1
}
