#include <iostream>

struct Hello
{
    int helloworld() { return 0; }
};

struct Generic {};    

// SFINAE test
template <typename T>
class has_helloworld
{
    typedef char one;
    struct two { char x[2]; };

    template <typename C> static one test( decltype(&C::helloworld) ) ;
    template <typename C> static two test(...);    

public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};
    
int main(int argc, char *argv[])
{
    std::cout << has_helloworld<Hello>::value << std::endl;
    std::cout << has_helloworld<Generic>::value << std::endl;
    return 0;
}