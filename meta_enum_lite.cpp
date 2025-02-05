#include <windows.h>
#include <string>
#include "meta_enum.hpp"

#include <vector>
#include <string>

namespace vg::core
{
    using string = std::string;
    template <typename T> using vector = std::vector<T>;
    using u8 = unsigned char;
    using uint = unsigned int;
}
#include "Enum.h"

using namespace vg::core;

void debugPrint(const char * _format, ...)
{
    const int bufferSize = 4096;
    char tempBuffer[bufferSize];
    tempBuffer[0] = 0;
    va_list args;
    va_start(args, _format);
    vsnprintf_s(tempBuffer, bufferSize - 1, _format, args);
    OutputDebugStringA(tempBuffer);
    va_end(args);
}

// Declare enum class MyEnumABC in namespace test
namespace test
{
    vg_enum_class(test, MyEnumABC,
        vg::core::u8,
        A = 65,
        B = 66,
        C = 67
    );
}

// Declare enum class MyEnumUVW in namespace test2
namespace toto::test2
{
    vg_enum_class(toto::test2, MyEnumUVW,
        int,
        U = 10,
        V = 11,
        W = 12
    );
}

// Declare global enum class GlobalEnum
vg_enum_class_global(GlobalEnum,
    int,
    X = 0,
    Y,
    Z
);

using namespace test;

int main()
{
    debugPrint("*** meta_enum_lite tests begin ***\n");
    {
        const auto count = enumCount<MyEnumABC>();
        const auto & members = getEnumMembers<MyEnumABC>();
        for (auto i = 0; i < members.size(); ++i)
            debugPrint("#%u %.*s = %u\n", i, members[i].name.size(), members[i].name.data(), members[i].value);
        
        debugPrint("getEnumString(MyEnumABC::A) = \"%s\"\n", getEnumString(MyEnumABC::A).c_str());

        MyEnumABC enumABC = MyEnumABC::C;
        string tempString = getEnumString(enumABC);
        
        debugPrint("getEnumString(getEnumValue<MyEnumABC>(0)) = \"%s\" (%u)\n", getEnumString(getEnumValue<MyEnumABC>(0)).c_str(), getEnumValue<MyEnumABC>(0));
    
        debugPrint("getEnumString(toto::test2::MyEnumUVW::U) = \"%s\"\n", getEnumString(toto::test2::MyEnumUVW::U).c_str());

        debugPrint("getEnumString(GlobalEnum::X) = \"%s\"\n", getEnumString(GlobalEnum::X).c_str());
    }
    debugPrint("*** meta_enum_lite tests end ***\n");
}
