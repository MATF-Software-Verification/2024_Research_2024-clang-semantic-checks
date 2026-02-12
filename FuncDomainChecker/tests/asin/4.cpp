int asin(int x){}


void test() {
 int x = 123;
 asin(x);// expected warning: argument is definitely > 1
}
