#ifndef __GC_H__
#define __GC_H__ 1

#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

#define _DEBUG 1

#include "util/debug_new/debug_new.h"
#include "headers/gc_reference.hpp"

using namespace std;

template <class T> class GCPointer {

  static list< GCReference<T> > reference_list;
  static bool initialized;
  static int instance_count;

  static std::mutex m_lock;
  static std::condition_variable cv;
  static bool empty_references;

  typename list<GCReference<T> >::iterator findPtrInfo(T *ptr);

  // addr points to the allocated memory to which this GCPointer pointer currently points.
  T *addr;

  public:

    GCPointer(T *t=NULL) {

      m_lock.lock();

      if (!initialized) {
        initialized = true;
        atexit(shutdown);
      }

      typename list<GCReference<T> >::iterator p;
      p = findPtrInfo(t);

      if(p != reference_list.end()) {
        p->refcount++;
      } else {
        GCReference<T> gcObj(t);
        reference_list.push_front(gcObj);
      }

      if (!hasInstances()) {
        std::thread([=] { startCollectionThread(); }).detach();
      }

      instance_count++;

      addr = t;

      m_lock.unlock();

    }

    void
    startCollectionThread() {
      #ifdef _DEBUG
        cout << "Collection Thread Started" << std::endl;
      #endif
      while(hasInstances()) {
        #ifdef _DEBUG
          //cout << "Collection Thread Running" << std::endl;
        #endif
        collect();
      }
      cout << "end collection thread" << endl;
    }

    T *operator=(T *t);
    GCPointer &operator=(GCPointer &rv);

    operator T *() {
      return addr;
    }

    T &operator*() {
      return *addr;
    }

    T *operator->() {
      return addr;
    }

    ~GCPointer();

    static bool collect();

    static bool hasInstances() { return instance_count > 0; }

    static void shutdown();

    static int referenceListSize() {
      return reference_list.size();
    }
};

/*

  Destructor

*/

template <class T>
GCPointer<T>::~GCPointer() {

  m_lock.lock();

  typename list<GCReference<T> >::iterator p;
  p = findPtrInfo(addr);

  if(p->refcount) {
    p->refcount--; // decrement ref count
  }

  instance_count--;

  if (!hasInstances()) {
    cout << "No more instances" << std::endl;
    //cv.notify_one();
  }

  #ifdef _DEBUG
    cout << "GCPointer going out of scope.\n";
  #endif

  m_lock.unlock();

}

/*

  Collect

*/

template <class T>
bool GCPointer<T>::collect() {

  m_lock.lock();

  bool memfreed = false;

  typename list<GCReference<T> >::iterator p;

  do {
    // Scan gclist looking for unreferenced pointers.
    for(p = reference_list.begin(); p != reference_list.end(); p++) {
      // If in-use, skip.
      if(p->refcount > 0) {
        continue;
      }

      memfreed = true;

      // Remove unused entry from gclist.
      reference_list.remove(*p);

      // Free memory unless the GCPtr is null.
      if(p->memPtr) {
        #ifdef _DEBUG
          cout << "Deleting: " << *(T *) p->memPtr << "\n";
        #endif
        delete p->memPtr; // delete single element
      }

      // Restart the search.
      break;
    }
  } while(p != reference_list.end());

  m_lock.unlock();

  return memfreed;
}

/*

  Operators

*/

template <class T>
T * GCPointer<T>::operator=(T *t) {


  typename list<GCReference<T> >::iterator p;

  p = findPtrInfo(addr);
  p->refcount--;

  p = findPtrInfo(t);
  if(p != reference_list.end()) {
    p->refcount++;
  } else {
    GCReference<T> gcObj(t);
    reference_list.push_front(gcObj);
  }

  addr = t;

  return t;
}

template <class T>
GCPointer<T> & GCPointer<T>::operator=(GCPointer &rv) {

  typename list<GCPointer<T> >::iterator p;

  p = findPtrInfo(addr);
  p->refcount--;

  p = findPtrInfo(rv.addr);
  p->refcount++;
  addr = rv.addr;

  return rv;
}

/*

  Static Properties

*/

template <class T> list<GCReference<T> > GCPointer<T>::reference_list;
template <class T> bool GCPointer<T>::initialized = false;
template <class T> int GCPointer<T>::instance_count = 0;

template <class T> bool GCPointer<T>::empty_references = true;
template <class T> std::mutex GCPointer<T>::m_lock;
template <class T> std::condition_variable GCPointer<T>::cv;


/*

  Methods

*/

template <class T> typename list<GCReference<T> >::iterator
GCPointer<T>::findPtrInfo(T *ptr) {
  typename list<GCReference<T> >::iterator p;
  for(p = reference_list.begin(); p != reference_list.end(); p++) {
    if(p->memPtr == ptr) {
      return p;
    }
  }
  return p;
}

template <class T>
void GCPointer<T>::shutdown() {

  if(referenceListSize() == 0) {
    return;
  }

  #ifdef _DEBUG
    cout << "Before collecting for shutdown() for " << typeid(T).name() << "\n";
  #endif

  collect();

  #ifdef _DEBUG
    cout << "After collecting for shutdown() for " << typeid(T).name() << "\n";
  #endif

  typename list<GCReference<T> >::iterator p;
  for(p = reference_list.begin(); p != reference_list.end(); p++) {
    p->refcount = 0;
  }

}

#endif