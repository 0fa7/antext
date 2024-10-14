#include <iostream>
#include <string>
#include "WindowsPlatform.hpp"

class ClientContext
{
public:
    ClientContext(){};
    ~ClientContext(){};
    std::string username_ = "test-user";
};

int main()
{
    EnableUtf8Terminal();
    ClientContext c;
    
    std::cout << c.username_ << std::endl;
}