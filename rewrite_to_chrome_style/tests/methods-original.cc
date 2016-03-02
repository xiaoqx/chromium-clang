// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace blink {

class MyIterator {};
using my_iterator = char*;

class Task {
 public:
  // Already style-compliant methods shouldn't change.
  void OutputDebugString() {}

  // Tests that the declarations for methods are updated.
  void doTheWork();
  // Overload to test using declarations that introduce multiple shadow
  // declarations.
  void doTheWork(int);
  virtual void reallyDoTheWork() = 0;

  // Note: this is purposely copyable and assignable, to make sure the Clang
  // tool doesn't try to emit replacements for things that aren't explicitly
  // written.

  // Overloaded operators should not be rewritten.
  Task& operator++() {
    return *this;
  }

  // Conversion functions should not be rewritten.
  explicit operator int() const {
    return 42;
  }

  // These are special functions that we don't rename so that range-based
  // for loops and STL things work.
  MyIterator begin() {}
  my_iterator end() {}
  my_iterator rbegin() {}
  MyIterator rend() {}
  // The trace() method is used by Oilpan, we shouldn't rename it.
  void trace() {}
  // These are used by std::unique_lock and std::lock_guard.
  void lock() {}
  void unlock() {}
  void try_lock() {}
};

class Other {
  // Static begin/end/trace don't count, and should be renamed.
  static MyIterator begin() {}
  static my_iterator end() {}
  static void trace() {}
  static void lock() {}
};

class NonIterators {
  // begin()/end() and friends are renamed if they don't return an iterator.
  void begin() {}
  int end() { return 0; }
  void rbegin() {}
  int rend() { return 0; }
};

// Test that the actual method definition is also updated.
void Task::doTheWork() {
  reallyDoTheWork();
}

}  // namespace blink

// Test that overrides from outside the Blink namespace are also updated.
class BovineTask : public blink::Task {
 public:
  using Task::doTheWork;
  void reallyDoTheWork() override;
};

class SuperBovineTask : public BovineTask {
 public:
  using BovineTask::reallyDoTheWork;
};

void BovineTask::reallyDoTheWork() {
  doTheWork();
  // Calls via an overridden method should also be updated.
  reallyDoTheWork();
}

// Finally, test that method pointers are also updated.
void F() {
  void (blink::Task::*p1)() = &blink::Task::doTheWork;
  void (blink::Task::*p2)() = &BovineTask::doTheWork;
  void (blink::Task::*p3)() = &blink::Task::reallyDoTheWork;
  void (BovineTask::*p4)() = &BovineTask::reallyDoTheWork;
}

namespace blink {

struct StructInBlink {
  // Structs in blink should rename their methods to capitals.
  bool function() { return true; }
};

}  // namespace blink

namespace WTF {

struct StructInWTF {
  // Structs in WTF should rename their methods to capitals.
  bool function() { return true; }
};

}  // namespace WTF

void F2() {
  blink::StructInBlink b;
  b.function();
  WTF::StructInWTF w;
  w.function();
}
