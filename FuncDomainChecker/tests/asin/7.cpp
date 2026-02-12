#include<iostream>
int asin(int x){}

void test() {
  int a;
  std::cin>>a;
  asin(a); //expected warning: argument might be outside of range
  }
