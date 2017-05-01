#ifndef GC_REFERENCE
#define GC_REFERENCE

template <class T> class GCReference {
  public:
    unsigned refcount;    // current reference count
    T *memPtr;            // pointer to allocated memory

    GCReference(T *mPtr) {
      refcount = 1;
      memPtr = mPtr;
    }
};

template <class T>
bool operator==(const GCReference<T> &ob1, const GCReference<T> &ob2) {
  return (ob1.memPtr == ob2.memPtr);
}

#endif