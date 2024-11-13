#include <iostream>

#define LOG(x) std::cout << #x << ": " << (x) << "\n"
#define LOGI(x) std::cout << #x << "\n"

int main()
{
	LOGI(Hello);
}
