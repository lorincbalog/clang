// This test is line- and column-sensitive, so test commands are at the bottom.
// Tests are identical with Analysis/bug_hash_test.cpp

constexpr int clang_analyzer_hashDump(int) { return 5; }

void function(int) {
  clang_analyzer_hashDump(5);
}

namespace {
void variadicParam(int, ...) {
  clang_analyzer_hashDump(5);
}
} // namespace

constexpr int f() {
  return clang_analyzer_hashDump(5);
}

namespace AA {
class X {
  X() {
    clang_analyzer_hashDump(5);
  }

  static void static_method() {
    clang_analyzer_hashDump(5);
    variadicParam(5);
  }

  void method() && {
    struct Y {
      inline void method() const & {
        clang_analyzer_hashDump(5);
      }
    };

    Y y;
    y.method();

    clang_analyzer_hashDump(5);
  }

  void OutOfLine();

  X &operator=(int) {
    clang_analyzer_hashDump(5);
    return *this;
  }

  operator int() {
    clang_analyzer_hashDump(5);
    return 0;
  }

  explicit operator float() {
    clang_analyzer_hashDump(5);
    return 0;
  }
};
} // namespace AA

void AA::X::OutOfLine() {
  clang_analyzer_hashDump(5);
}

void testLambda() {
  []() {
    clang_analyzer_hashDump(5);
  }();
}

template <typename T>
void f(T) {
  clang_analyzer_hashDump(5);
}

template <typename T>
struct TX {
  void f(T) {
    clang_analyzer_hashDump(5);
  }
};

template <>
void f<long>(long) {
  clang_analyzer_hashDump(5);
}

template <>
struct TX<long> {
  void f(long) {
    clang_analyzer_hashDump(5);
  }
};

template <typename T>
struct TTX {
  template<typename S>
  void f(T, S) {
    clang_analyzer_hashDump(5);
  }
};

// RUN: mkdir -p %T
// RUN: cp %s %T/test.cpp
// RUN: sed 's|test_dir|%/T|g' %S/template.json > %T/compile_commands.json
// RUN: clang-display-context -line=7 -col=27 %T/test.cpp | FileCheck -check-prefix=CHECK1 %s
// RUN: clang-display-context -line=12 -col=27 %T/test.cpp | FileCheck -check-prefix=CHECK2 %s
// RUN: clang-display-context -line=17 -col=34 %T/test.cpp | FileCheck -check-prefix=CHECK3 %s
// RUN: clang-display-context -line=23 -col=29 %T/test.cpp | FileCheck -check-prefix=CHECK4 %s
// RUN: clang-display-context -line=27 -col=29 %T/test.cpp | FileCheck -check-prefix=CHECK5 %s
// RUN: clang-display-context -line=34 -col=33 %T/test.cpp | FileCheck -check-prefix=CHECK6 %s
// RUN: clang-display-context -line=41 -col=29 %T/test.cpp | FileCheck -check-prefix=CHECK7 %s
// RUN: clang-display-context -line=47 -col=29 %T/test.cpp | FileCheck -check-prefix=CHECK8 %s
// RUN: clang-display-context -line=52 -col=29 %T/test.cpp | FileCheck -check-prefix=CHECK9 %s
// RUN: clang-display-context -line=57 -col=29 %T/test.cpp | FileCheck -check-prefix=CHECK10 %s
// RUN: clang-display-context -line=64 -col=27 %T/test.cpp | FileCheck -check-prefix=CHECK11 %s
// RUN: clang-display-context -line=69 -col=29 %T/test.cpp | FileCheck -check-prefix=CHECK12 %s
// RUN: clang-display-context -line=75 -col=27 %T/test.cpp | FileCheck -check-prefix=CHECK13 %s
// RUN: clang-display-context -line=81 -col=29 %T/test.cpp | FileCheck -check-prefix=CHECK14 %s
// RUN: clang-display-context -line=87 -col=27 %T/test.cpp | FileCheck -check-prefix=CHECK15 %s
// RUN: clang-display-context -line=93 -col=29 %T/test.cpp | FileCheck -check-prefix=CHECK16 %s
// RUN: clang-display-context -line=101 -col=29 %T/test.cpp | FileCheck -check-prefix=CHECK17 %s

// CHECK1: void function(int)$27$clang_analyzer_hashDump(5);
// CHECK2: void (anonymous namespace)::variadicParam(int, ...)$27$clang_analyzer_hashDump(5);
// CHECK3: int f()$34$returnclang_analyzer_hashDump(5);
// CHECK4: AA::X::X()$29$clang_analyzer_hashDump(5);
// CHECK5: void AA::X::static_method()$29$clang_analyzer_hashDump(5);
// CHECK6: void AA::X::method()::Y::method() const &$33$clang_analyzer_hashDump(5);
// CHECK7: void AA::X::method() &&$29$clang_analyzer_hashDump(5);
// CHECK8: class AA::X & AA::X::operator=(int)$29$clang_analyzer_hashDump(5);
// CHECK9: AA::X::operator int()$29$clang_analyzer_hashDump(5);
// CHECK10: AA::X::operator float()$29$clang_analyzer_hashDump(5);
// CHECK11: void AA::X::OutOfLine()$27$clang_analyzer_hashDump(5);
// CHECK12: void testLambda()::(anonymous class)::operator()() const$29$clang_analyzer_hashDump(5);
// CHECK13: void f(T)$27$clang_analyzer_hashDump(5);
// CHECK14: void TX::f(T)$29$clang_analyzer_hashDump(5);
// CHECK15: void f(long)$27$clang_analyzer_hashDump(5);
// CHECK16: void TX<long>::f(long)$29$clang_analyzer_hashDump(5);
// CHECK17: void TTX::f(T, S)$29$clang_analyzer_hashDump(5);
