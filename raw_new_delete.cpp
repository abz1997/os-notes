// // file: unique_ptr_example.cpp
// #include <iostream>
// #include <memory>

// int main() {
//     auto p = std::make_unique<int>(123);
//     std::cout << &p;
//     std::cout << p.get();
//     // std::cout << "unique_ptr value = " << *p << '\n';

//     auto arr = std::make_unique<double[]>(4);
//     // for (int i = 0; i < 4; ++i) arr[i] = i * 2.0;
//     // std::cout << "unique_ptr array:";
//     // for (int i = 0; i < 4; ++i) std::cout << ' ' << arr[i];
//     // std::cout << '\n';

//     // no explicit delete needed — RAII will free automatically
//     return 0;
// }


// file: raw_new_delete.cpp
#include <iostream>

int main() {
    // allocate single int
    int* p = new int(42);
    std::cout << *p << '\n';
    std::cout << p << '\n';
    std::cout << &p << '\n';
    std::cout << sizeof(p) << '\n';
    std::cout << sizeof(*p) << '\n';
    std::cout << sizeof(int) << '\n';
    // std::cout << &p.get() << '\n'; // This line will cause a compilation error since 'p' is a raw pointer, not a smart pointer.

    int x = 42;        // allocated on the stack
    std::cout << x << '\n';   // prints value: 42
    std::cout << &x << '\n';  // pr
    char* pa = new char[4];
    std::cin >> pa;
    std::cout << sizeof(pa) << '\n';
    std::cout << sizeof(pa) << '\n';
    std::cout << sizeof(char*) << '\n';


    return 0;
}
