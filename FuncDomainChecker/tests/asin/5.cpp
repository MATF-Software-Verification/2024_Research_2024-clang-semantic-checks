int asin(int x){}


void test(int a) {
   if(a>1)
    asin(a); //expected warning: argument is definitely > 1
}
