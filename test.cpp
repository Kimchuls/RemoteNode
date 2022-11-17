// 01_hello_thread.cpp

#include <iostream>
#include <thread> 
#include<vector>

using namespace std; 

struct node{
  int x;
  int y;
};

int main() {
  vector<node> vi;
  for (int i=0;i<6;i++)vi.emplace_back((node){i+6,i+1});
  auto it=vi.begin();
  cout<<(it+4)->x<<endl;

  return 0;
}