int acos(int x){}


void test(int a) {
   if(a>1)
    acos(a); //expected warning: argument is definitely > 1
}
