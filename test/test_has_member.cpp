
// usage example: check for the existence of a "sayHi" member //////////////////////////////////////

// clang++ -std=c++11 -pedantic -Wall -Wextra test_has_member.cpp -o test_has_member && ./test_has_member
// g++     -std=c++11 -pedantic -Wall -Wextra test_has_member.cpp -o test_has_member && ./test_has_member

#include <iostream> // cout, endl
#include <iomanip>  // std::boolalpha

#include "has_member.hpp"

struct A  // has a "sayHi" member
{
    void sayHi() { std::cout << "Hi there!" << std::endl;  }
};

struct B  // doesn't have a "sayHi" member
{
    void sayBye() { std::cout << "Bye bye!" << std::endl; }
};

// define a "sayHi" "member checker" class
define_has_member(sayHi);

int main()
{
    using std::cout;
    using std::endl;
    cout << std::boolalpha;  // display "true" or "false" for booleans

    A a;
    B b;

    // check the existence of "sayHi"
    cout << "has_member(A, sayHi) " << has_member(A, sayHi) << endl;
    cout << "has_member(B, sayHi) " << has_member(B, sayHi) << endl;

    cout << endl;

    // same thing, using decltype on instances
    cout << "has_member(decltype(a), sayHi) " << has_member(decltype(a), sayHi) << endl;
    cout << "has_member(decltype(b), sayHi) " << has_member(decltype(b), sayHi) << endl;

    return 0;
}
