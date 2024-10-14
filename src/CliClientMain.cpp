#include <iostream>
#include <string>
#include "LinuxPlatform.hpp"

class ClientContext
{
public:
    ClientContext(){};
    ~ClientContext(){};
    std::string username_ = "I 😍 Ω";
};

int main()
{
    ClientContext c;

    std::cout << c.username_ << std::endl;
}