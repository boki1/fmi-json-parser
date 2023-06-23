#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <gtest/gtest.h>

#include <mystd/memory.h>

std::ostringstream sstr;

///
/// This test case is directly taken from cppreference.
/// I have removed sections 3, 4 and 5 as mystd::unique_ptr
/// does not support custom deleters nor arrays.
///

// helper class for runtime polymorphism demo below
struct B {
  virtual ~B() = default;

  virtual void bar() { sstr << "B::bar\n"; }
};

struct D : B {
  D() { sstr << "D::D\n"; }
  ~D() { sstr << "D::~D\n"; }

  void bar() override { sstr << "D::bar\n"; }
};

// a function consuming a unique_ptr can take it by value or by rvalue reference
mystd::unique_ptr<D> pass_through(mystd::unique_ptr<D> p) {
  p->bar();
  return p;
}

// unique_ptr-based linked list demo
struct List {
  struct Node {
    int data;
    mystd::unique_ptr<Node> next;
  };

  mystd::unique_ptr<Node> head;

  ~List() {
    // destroy list nodes sequentially in a loop, the default destructor
    // would have invoked its `next`'s destructor recursively, which would
    // cause stack overflow for sufficiently large lists.
    while (head) {
      auto next = std::move(head->next);
      head = std::move(next);
    }
  }

  void push(int data) {
    head = mystd::unique_ptr<Node>(new Node{data, std::move(head)});
  }
};

TEST(MystdTests, UniquePtr) {
  sstr << "1) Unique ownership semantics demo\n";
  {
    // Create a (uniquely owned) resource
    mystd::unique_ptr<D> p = mystd::make_unique<D>();

    // Transfer ownership to `pass_through`,
    // which in turn transfers ownership back through the return value
    mystd::unique_ptr<D> q = pass_through(std::move(p));

    // p is now in a moved-from 'empty' state, equal to nullptr
    assert(!p);
  }

  sstr << "\n"
          "2) Runtime polymorphism demo\n";
  {
    // Create a derived resource and point to it via base type
    mystd::unique_ptr<B> p = mystd::make_unique<D>();

    // Dynamic dispatch works as expected
    p->bar();
  }

  EXPECT_EQ(sstr.str(), "1) Unique ownership semantics demo\n"
                        "D::D\n"
                        "D::bar\n"
                        "D::~D\n"
                        "\n"
                        "2) Runtime polymorphism demo\n"
                        "D::D\n"
                        "D::bar\n"
                        "D::~D\n");
}
