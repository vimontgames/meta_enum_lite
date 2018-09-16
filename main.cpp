#include <array>
#include <string_view>
#include <iostream>

template <typename EnumType>
struct MetaEnumMember
{
    EnumType value = {};
    std::string_view name;
    std::string_view string;
    size_t index = {};
};

template <typename EnumType, size_t size>
struct EnumMeta
{
    std::string_view string;
    std::array<MetaEnumMember<EnumType>, size> members;
};

constexpr size_t nextEnumCommaOrEnd(size_t start, std::string_view enumString)
{
    size_t brackets = 0; //()[]{}
    bool quote = false; //""
    char lastChar = '\0';
    char nextChar = '\0';

    auto isNested = [&brackets, &quote] ()
    {
        return brackets != 0 || quote;
    };

    auto feedCounters = [&brackets, &quote, &lastChar, &nextChar] (char c)
    {
        if(quote)
        {
            if(lastChar != '\\' && c == '"') //ignore " if they are backslashed
                quote = false;
            return;
        }

        switch(c)
        {
            case '"':
                if(lastChar != '\\') //ignore " if they are backslashed
                    quote = true;
                break;
            case '(':
            case '<':
                if(lastChar == '<' || nextChar == '<')
                    break;
            case '{':
                ++brackets;
                break;
            case ')':
            case '>':
                if(lastChar == '>' || nextChar == '>')
                    break;
            case '}':
                --brackets;
                break;
            default:
                break;
        }
    };

    size_t current = start;
    for(; current < enumString.size() && (isNested() || (enumString[current] != ',')); ++current)
    {
        feedCounters(enumString[current]);
        lastChar = enumString[current];
        nextChar = current + 2 < enumString.size() ? enumString[current + 2] : '\0';
    }

    return current;
}

constexpr std::string_view parseEnumMemberName(std::string_view memberString)
{
    std::string_view memberName = memberString;

    //TODO: parse name

    return memberName;
}

template <typename EnumType, size_t size>
constexpr EnumMeta<EnumType, size> parseEnumMeta(std::string_view in, const std::array<EnumType, size>& values)
{
    EnumMeta<EnumType, size> result;
    result.string = in;

    std::array<std::string_view, size> memberStrings;
    size_t amountFilled = 0;

    size_t currentStringStart = 0;

    while(amountFilled < size)
    {
        size_t currentStringEnd = nextEnumCommaOrEnd(currentStringStart + 1, in);
        size_t currentStringSize = currentStringEnd - currentStringStart;

        if(currentStringStart != 0)
        {
            ++currentStringStart;
            --currentStringSize;
        }

        memberStrings[amountFilled] = std::string_view(in.begin() + currentStringStart, currentStringSize);
        ++amountFilled;
        currentStringStart = currentStringEnd;
    }

    for(size_t i = 0; i < memberStrings.size(); ++i)
    {
        result.members[i].name = parseEnumMemberName(memberStrings[i]);
        result.members[i].string = memberStrings[i];
        result.members[i].value = values[i];
        result.members[i].index = i;
    }

    return result;
}

template <typename EnumUnderlyingType>
struct IntWrapper
{
    constexpr IntWrapper(): value(0), empty(true)
    {
    }
    constexpr IntWrapper(EnumUnderlyingType in): value(in), empty(false)
    {
    }
    constexpr IntWrapper operator=(EnumUnderlyingType in)
    {
        value = in;
        empty = false;
        return *this;
    }
    EnumUnderlyingType value;
    bool empty;
};

template <typename EnumType, typename EnumUnderlyingType, size_t size>
constexpr std::array<EnumType, size> resolveEnumValuesArray(const std::initializer_list<IntWrapper<EnumUnderlyingType>>& in)
{
    std::array<EnumType, size> result{};

    EnumUnderlyingType nextValue = 0;
    for(size_t i = 0; i < size; ++i)
    {
        auto wrapper = *(in.begin() + i);
        EnumUnderlyingType newValue = wrapper.empty ? nextValue : wrapper.value;
        nextValue = newValue + 1;
        result[i] = static_cast<EnumType>(newValue);
    }

    return result;
}

#define meta_enum(Type, UnderlyingType, ...)\
    enum Type : UnderlyingType { __VA_ARGS__};\
    constexpr static size_t Type##_internal_size = ([] ()\
    {\
        using IntWrapperType = IntWrapper<UnderlyingType>;\
        IntWrapperType __VA_ARGS__;\
        auto init_list = {__VA_ARGS__};\
        return init_list.size();\
    }());\
    constexpr static auto Type##_meta = parseEnumMeta<Type, Type##_internal_size>(#__VA_ARGS__, []() {\
        using IntWrapperType = IntWrapper<UnderlyingType>;\
        IntWrapperType __VA_ARGS__;\
        return resolveEnumValuesArray<Type, UnderlyingType, Type##_internal_size>({__VA_ARGS__});\
    }());

#define meta_enum_class(Type, UnderlyingType, ...)\
    enum class Type : UnderlyingType { __VA_ARGS__};\
    constexpr static size_t Type##_internal_size = ([] ()\
    {\
        using IntWrapperType = IntWrapper<UnderlyingType>;\
        IntWrapperType __VA_ARGS__;\
        auto init_list = {__VA_ARGS__};\
        return init_list.size();\
    }());\
    constexpr static auto Type##_meta = parseEnumMeta<Type, Type##_internal_size>(#__VA_ARGS__, []() {\
        using IntWrapperType = IntWrapper<UnderlyingType>;\
        IntWrapperType __VA_ARGS__;\
        return resolveEnumValuesArray<Type, UnderlyingType, Type##_internal_size>({__VA_ARGS__});\
    }());

////USAGE:

constexpr int getDivFourOfLast(int a, int b, int c)
{
    return c;
}

meta_enum(Hahas, int32_t, Hi, Ho= getDivFourOfLast(1, {(2, ")h(),,\"ej", 1)}, 4 >> 2), Hu =     4,
He);

//declares enum as Hahas like usual.
//Provides meta object accessible with Hahas_meta

int main()
{
    static_assert(Hahas_meta.members.size() == 4);
    static_assert(Hahas_meta.members[0].string == "Hi");
    static_assert(Hahas_meta.members[1].string == " Ho= getDivFourOfLast(1, {(2, \")h(),,\\\"ej\", 1)}, 4 >> 2)");
    static_assert(Hahas_meta.members[2].string == " Hu = 4");
    static_assert(Hahas_meta.members[3].string == " He");

    //TODO:
    //static_assert(Hahas_meta.members[0].name == "Hi");
    //static_assert(Hahas_meta.members[1].name == "Ho");
    //static_assert(Hahas_meta.members[2].name == "Hu");
    //static_assert(Hahas_meta.members[3].name == "He");

    static_assert(Hahas_meta.members[0].value == 0);
    static_assert(Hahas_meta.members[1].value == 1);
    static_assert(Hahas_meta.members[2].value == 4);
    static_assert(Hahas_meta.members[3].value == 5);

    static_assert(Hahas_meta.members[0].index == 0);
    static_assert(Hahas_meta.members[1].index == 1);
    static_assert(Hahas_meta.members[2].index == 2);
    static_assert(Hahas_meta.members[3].index == 3);

    std::cout << "declared enum: " << Hahas_meta.string << "\n";
    std::cout << "member strings: \n";
    for(const auto& enumMember : Hahas_meta.members)
    {
        std::cout << enumMember.string << "\n";
    }
    std::cout << "member values: \n";
    for(const auto& enumMember : Hahas_meta.members)
    {
        std::cout << enumMember.value << "\n";
    }
    std::cout << "member names: \n";
    for(const auto& enumMember : Hahas_meta.members)
    {
        std::cout << enumMember.name << "\n";
    }

    meta_enum_class(Nested, uint8_t, One,Two,Three);
}

struct T1
{
    meta_enum_class(Nested, size_t, One, Two
    =
    0b11010101, Three
    );
};

// TODO

//parse enum member name

//errors on clang :'(

