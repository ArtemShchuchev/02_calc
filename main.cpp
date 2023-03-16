#include <chrono>
#include <thread>
#include <random>
#include <algorithm>
#include <execution>
#include <vector>
#include <array>
#include <iomanip>	// std::setw
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
// 1000, 10 000, 100 000 и 1 000 000 элементов
const int SIZEV[]{ 1000, 10000, 100000, 1000000 };
int numS = sizeof SIZEV / sizeof SIZEV[0];

// сумму 2х векторов складывает в 1ый вектор
void summVect(std::vector<int>& v1, std::vector<int>& v2, int offset_begin, int num_byte, double& tm)
{
	std::call_once(flag, [](){std::wcout << std::left << L"Количество аппаратных ядер: "
		<< std::thread::hardware_concurrency() << "\n\n"
		<< std::setw(11) << " "
		<< std::setw(11) << "1000"
		<< std::setw(11) << "10000"
		<< std::setw(11) << "100000"
		<< "1000000\n"; });

	auto start = std::chrono::high_resolution_clock::now();
	for (auto it2 = v2.begin() + offset_begin, it = v1.begin() + offset_begin;
		it != v1.begin() + offset_begin + num_byte; ++it, ++it2)
		*it += *it2;
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> time = end - start;
	tm = time.count();
}
// вывод инфо
void print(std::wstring& str, std::vector<double> workTime)
{
	std::wcout << std::left
		<< std::setw(11) << str
		<< workTime.at(0) << std::setw(4) << "ms"
		<< workTime.at(1) << std::setw(4) << "ms"
		<< workTime.at(2) << std::setw(4) << "ms"
		<< workTime.at(3) << "ms\n";
}

int main(int argc, char** argv)
{
	printHeader(L"Параллельные вычисления");

	const int NUMP(5);
	std::array<std::wstring, NUMP> potokStr{L"1 поток", L"2 потока", L"4 потока", L"8 потоков", L"16 потоков"};
	std::vector<double> workTime(numS); // здась буду хранить время работы
	std::vector<int> V1, V2;	// вектора для сложения

	for (int p = 0; p < NUMP; ++p)
	{
		for (int i = 0; i < numS; ++i)
		{
			V1.resize(SIZEV[i]);
			V2.resize(SIZEV[i]);
			for (int j = 0; j < SIZEV[i]; ++j)
			{
				V1.at(j) = j + 1;
				V2.at(j) = 10;
			}

			if (p == 0)
			{
				double time(0.0);
				std::thread t1(summVect, std::ref(V1), std::ref(V2), 0, SIZEV[i], std::ref(time));
				t1.join();
				workTime.at(i) = time;
			}
			else if (p == 1)
			{
				double time[2]{ 0 };
				std::thread t2(summVect, std::ref(V1), std::ref(V2), 0, SIZEV[i] / 2, std::ref(time[0]));
				std::thread t3(summVect, std::ref(V1), std::ref(V2), SIZEV[i] / 2, SIZEV[i] - SIZEV[i] / 2, std::ref(time[1]));
				t3.join();
				t2.join();
				workTime.at(i) = time[0] + time[1];
			}
			else if (p == 2)
			{
				double time[4]{ 0 };
				int sizebl = SIZEV[i] / 4;
				std::thread t4(summVect, std::ref(V1), std::ref(V2), 0, sizebl, std::ref(time[0]));
				std::thread t5(summVect, std::ref(V1), std::ref(V2), sizebl, sizebl, std::ref(time[1]));
				std::thread t6(summVect, std::ref(V1), std::ref(V2), sizebl*2, sizebl, std::ref(time[2]));
				std::thread t7(summVect, std::ref(V1), std::ref(V2), sizebl*3, SIZEV[i] - sizebl * 3, std::ref(time[3]));
				t4.join();
				t5.join();
				t6.join();
				t7.join();
				workTime.at(i) = time[0] + time[1] + time[2] + time[3];
			}
			else if (p == 3)
			{
				double time[8]{ 0 };
				int sizebl = SIZEV[i] / 8;
				std::vector<std::thread> t8(8);
				for (size_t k = 0; k < 8; ++k)
				{
					if (k != 7)
					{
						t8.at(k) = (std::thread(
							summVect, std::ref(V1), std::ref(V2), sizebl * k, sizebl, std::ref(time[k]) ));
					}
					else
					{
						t8.at(k) = (std::thread(
							summVect, std::ref(V1), std::ref(V2), sizebl * k, SIZEV[i] - sizebl * 7, std::ref(time[k]) ));
					}
				}

				for (auto& t : t8) t.join();

				workTime.at(i) = 0;
				for (size_t k = 0; k < 8; ++k) workTime.at(i) += time[k];
			}
			else if (p == 4)
			{
				double time[16]{ 0 };
				int sizebl = SIZEV[i] / 16;
				std::vector<std::thread> t8(16);
				for (size_t k = 0; k < 16; ++k)
				{
					if (k != 15)
					{
						t8.at(k) = (std::thread(
							summVect, std::ref(V1), std::ref(V2), sizebl * k, sizebl, std::ref(time[k]) ));
					}
					else
					{
						t8.at(k) = (std::thread(
							summVect, std::ref(V1), std::ref(V2), sizebl * k, SIZEV[i] - sizebl * 15, std::ref(time[k]) ));
					}
				}

				for (auto& t : t8) t.join();

				workTime.at(i) = 0;
				for (size_t k = 0; k < 16; ++k) workTime.at(i) += time[k];
			}

		}

		print(potokStr.at(p), workTime);
	}


	
	std::wcout << "\n";

	return 0;
}
