#include <iostream>
#include "util/debug_new/debug_new.h"
#include "headers/gc_pointer.hpp"

using namespace std;

int main() {
  GCPointer<int> p = new int(1);
  GCPointer<int> c = new int(2);
  return 0;
}