#include <iostream>
#include "util/debug_new/debug_new.h"
#include "headers/gc_pointer.hpp"
#include <chrono>

using namespace std;

int main() {
  GCPointer<int> q = new int(44);
  for (int i = 0; i <= 10; i++) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    GCPointer<int> p = new int(i);
    GCPointer<int> x = new int(i+2);
  }
  return 0;
}