// Copyright (c) 2015, The Regents of the University of California (Regents)
// See LICENSE.txt for license details

#ifndef PVECTOR_H_
#define PVECTOR_H_

#include <algorithm>
//#include "path to arrays.h"
/*
GAP Benchmark Suite
Class:  pvector
Author: Scott Beamer

Vector class with ability to not initialize or do initialize in parallel
 - std::vector (when resizing) will always initialize, and does it serially
 - When pvector is resized, new elements are uninitialized
 - Resizing is not thread-safe
*/


template <typename T_>
class pvector {
 public:
  typedef ArrayIter<T_> iterator;

 pvector() : arr_(nullptr), elements_(0), capacity_(0) {}
  
  explicit pvector(size_t num_elements) {
    arr_ = new Array<T_>(num_elements);
    elements_ = num_elements;
    capacity_ = num_elements;
  }

  pvector(size_t num_elements, T_ init_val) : pvector(num_elements) {
    fill(init_val);
  }

  pvector(iterator copy_begin, iterator copy_end)
      : pvector(copy_end - copy_begin) {
    #pragma omp parallel for
    for (size_t i=0; i < capacity(); i++)
      arr_[i] = copy_begin[i];
  }

  // don't want this to be copied, too much data to move
  pvector(const pvector &other) = delete;

  // prefer move because too much data to copy
  pvector(pvector &&other)
    : arr_(other.arr_), elements_(other.elements_), capacity_(other.capacity_) {
    other.arr_ = nullptr;
    other.elements_ = 0;
    other.capacity_ = 0;
  }

  // want move assignment
  pvector& operator= (pvector &&other) {
    arr_ = other.arr_;
    elements_ = other.elements_;
    capacity_ = other.capacity_;
    other.arr_ = nullptr;
    other.elements_ = 0;
    other.capacity_ = 0;
    return *this;
  }

  ~pvector() {
    if (arr_ != nullptr)
      delete arr_;
  }

  // not thread-safe
  void reserve(size_t num_elements) {
    if (num_elements > capacity()) {
      Array<T_>* new_range = new Array<T_>(num_elements);
      #pragma omp parallel for
      for (size_t i=0; i < size(); i++)
        new_range[i] = arr_[i];
      delete arr_;
      arr_ = new_range;
      // elements_ does not change, just adding space
      capacity_ = num_elements;
    }
  }

  bool empty() {
    return elements_ = 0;
  }

  void clear() {
    elements_ = 0;
  }

  void resize(size_t num_elements) {
    reserve(num_elements);
  }

  T_& operator[](size_t n) {
    return (*arr_)[n];
  }

  const T_& operator[](size_t n) const {
    return (*arr_)[n];
  }

  void push_back(T_ val) {
    if (size() == capacity()) {
      size_t new_size = capacity() == 0 ? 1 : capacity() * growth_factor;
      reserve(new_size);
    }
    (*arr_)[elements_] = val;
    elements_++;
  }

  void fill(T_ init_val) {
    #pragma omp parallel for
    for (size_t i=0; i < elements_; i++)
      (*arr_)[i] = init_val;
  }

  size_t capacity() const {
    return capacity_;
  }

  size_t size() const {
    return elements_;
  }

  iterator begin() const {
    return iterator(arr_);
  }

  iterator end() const {
    iterator tmp = iterator(arr_);
    tmp += elements_;
    return tmp;
  }

  //TODO eliminate
  T_* data() const {
    return &((*arr_)[0]);
  }

  void swap(pvector &other) {
    std::swap(arr_, other.arr_);
    std::swap(elements_, other.elements_);
    std::swap(capacity_, other.capacity_);
  }


 private:
  Array<T_>* arr_;
  size_t elements_;
  size_t capacity_;
  static const size_t growth_factor = 2;
};

#endif  // PVECTOR_H_
