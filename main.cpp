#include <chrono>
#include <thread>
#include <random>
#include <algorithm>
#include <execution>
#include <vector>
#include "SecondaryFunction.h"

/*
Написать программу для расчета суммы двух векторов.
Распараллелить данную программу на 2, 4, 8 и 16 потоков.
Определить какое количество потоков дает самый быстрый результат.
Сравнить результаты выполнения для массивов из 1000, 10 000, 100 000 и 1 000 000 элементов.
Время выполнения для каждого варианта свести в таблицу и вывести ее в консоль.
Первый запущенный поток должен вывести на экран доступное количество аппаратных ядер.
*/

std::once_flag flag;

// сумму 2х векторов складывает в 1ый вектор
void summVect(std::vector<int>& v1, std::vector<int>& v2, int offset_begin, int num_byte)
{
	std::call_once(flag, [](){std::wcout << L"Количество аппаратных ядер: "
		<< std::thread::hardware_concurrency() << "\n\n"; });

	for (auto it2 = v2.begin() + offset_begin, it = v1.begin() + offset_begin;
		it != v1.begin() + offset_begin + num_byte; ++it, ++it2)
		*it += *it2;
}

int main(int argc, char** argv)
{
	printHeader(L"Параллельные вычисления");

	// 1000, 10 000, 100 000 и 1 000 000 элементов
	int size = 5;
	std::vector<int> V1(size), V2(size);
	int num = 0;
	for (auto& d : V1) d = ++num;
	for (auto& d : V2) d = 10;

	//auto it_start = V1.begin(), it_end = V1.end();

	auto start = std::chrono::high_resolution_clock::now();
	std::thread t1(summVect, std::ref(V1), std::ref(V2), 0, size);
	t1.join();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> time = end - start;
	std::wcout << L"1 поток = " << time.count() << "\n";



	start = std::chrono::high_resolution_clock::now();
	std::thread t2(summVect, std::ref(V1), std::ref(V2), 0, size/2);
	std::thread t3(summVect, std::ref(V1), std::ref(V2), size/2, size - size / 2);
	t3.join();
	t2.join();
	end = std::chrono::high_resolution_clock::now();
	time = end - start;
	std::wcout << L"2а потока = " << time.count() << "\n";
	





	for (auto& d : V1)
	{
		std::wcout << d << " ";
	}

	
	std::wcout << "\n";

	return 0;
}
