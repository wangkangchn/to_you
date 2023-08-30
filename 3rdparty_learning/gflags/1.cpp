

struct AAA
{
    template <typename T>
    AAA(T a) {}
};

#define TYPE()  \
    1 ? AAA(int()) : AAA(float())

int main()
{
    auto a = TYPE();
}