#include<iostream>
int acos(int x){}

void test() {
  int a;
  std::cin>>a;
  acos(a); //expected warning: argument might be outside of range
  }
