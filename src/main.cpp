#include <iostream>

#define LOG(x) std::cout << #x << ": " << (x) << "\n"
#define LOGI(x) std::cout << #x << "\n"

// Forward declare templated class C1
template <typename T>
class C1;

class C2
{
public:
	C2(int a = 0) : a(a) {}

private:
	template <typename T>
	friend class C1<T>;

	int a;
};

template <typename T>
class C1
{
public:
	C1(T a) : a(a) {}

	void PrintC2(C2 c2)
	{
		LOG(c2.a);
	}
private:
    T a;
};

int main()
{
	C1<int> c1(5);
	C2 c2(10);
	c1.PrintC2(c2);
}
