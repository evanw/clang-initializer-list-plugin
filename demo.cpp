#include <iostream>
#include <memory>

namespace NS1 {
  struct Foo {
    Foo() : x() {}
    int x, y;
  };

  struct Bar : Foo {
    Bar() : w() {}
    int z, w;
  };
}

namespace NS2 {
  struct Foo {
    Foo();
    int x, y;
  };

  Foo::Foo() {}

  struct Bar : Foo {
    Bar();
    int z;
    std::auto_ptr<int> nonPOD;
    int w;
  };

  Bar::Bar() {}
}
