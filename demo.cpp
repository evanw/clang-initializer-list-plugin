#include <iostream>
#include <memory>

namespace NS1 {
  struct Foo {
    Foo() : x() {} // missing x
    int x, y;
  };

  struct Bar : Foo {
    Bar() : w() {} // missing w
    int z, w;
  };
}

namespace NS2 {
  struct Foo {
    Foo();
    int x, y;
  };

  Foo::Foo() {} // missing x and y

  struct Bar : Foo {
    Bar();
    int z;
    std::auto_ptr<int> nonPOD;
    int w;
  };

  Bar::Bar() {} // missing z and w
}

namespace NS3 {
  struct Foo {
    Foo() : x() { y = x; }
    int x, y;
  };
}
