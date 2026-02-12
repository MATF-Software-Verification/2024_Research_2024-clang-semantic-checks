int acos(int x){}

int foo(int& a) {
  a-=5;
}

void test() {
  int a = 0;
  foo(a);
  acos(a); //expected warning: argument is definitely < -1
  }
