
#include <iostream>
#include <vector>
#include <memory>
#include <string>
// #include <fmt/format.h>
#include <type_traits>

namespace name
{
    
} // end namespace name
 
class A
{
public:
    constexpr void member() {}
};
 
class B
{
public:
    void member1() {}
};
 
 template<typename T>
void PrintMessage()
{
    if constexpr(std::is_member_function_pointer_v<void(&T::member)>)
    {
        std::cout << "T::member" << std::endl;
    }
    else
    {
        std::cout << "NO T::member" << std::endl;
    }
}

// int main()
// {
//     // fails at compile time if A::member is a data member and not a function
//     static_assert(std::is_member_function_pointer<decltype(&A::member)>::value,
//                   "A::member is not a member function."); 
// }

int main(int argc, char const *argv[])
{
    PrintMessage<A>();
    PrintMessage<B>();

    return 0;
}
