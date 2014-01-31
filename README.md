# Clang Initializer List Plugin

This is a plugin that emits a warning for every member variable that isn't initialized in its constructor's initializer list. Failing to initialize a member causes it to contain garbage and is the source of many subtle bugs that usually reveal themselves far away from where the issue actually lies. I will never understand why automatic initialization isn't the default in C++, but this plugin fixes the problem. Given a C++ file like this:

    struct Foo {
      Foo() : x() {}
      int x, y;
    };

The plugin finds all uninitialized members:

    demo.cpp:6:5: warning: constructor for Foo is missing initializer for member y
        Foo() : x() {}
        ^

It is careful to avoid producing warnings in system headers and for members with non-POD types (a non-POD type is a C++-style type, usually with a user-specified constructor and/or destructor).
