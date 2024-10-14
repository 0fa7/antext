#include <Windows.h>

void EnableUtf8Terminal()
{
    SetConsoleOutputCP( 65001 );
}