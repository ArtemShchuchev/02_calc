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
const int VARSIZE = sizeof SIZEV / sizeof SIZEV[0];	// кол-во вариантов размера массива

// сумму 2х векторов складывает в 1ый вектор
void summVect(std::vector<int>& v1, std::vector<int>& v2,
	int offset_begin, int num_byte,
	std::chrono::steady_clock::time_point& st)
{
	using namespace std;
	call_once(flag, [&st](){
		wcout << left << L"Количество аппаратных ядер: "
			<< thread::hardware_concurrency() << "\n\n" << setw(11) << " "
			<< setw(10) << "1000"
			<< setw(10) << "10000"
			<< setw(11) << "100000"
			<< "1000000\n";
		st = chrono::steady_clock::now(); });
	
	for (auto it2 = v2.begin() + offset_begin, it = v1.begin() + offset_begin;
		it != v1.begin() + offset_begin + num_byte; ++it, ++it2)
		*it += *it2;
}
// вывод инфо
void print(std::wstring& str, std::vector<double> workTime)
{
	using namespace std;
	wcout.setf(ios::fixed);
	wcout << setprecision(3) << left << setw(11) << str
		<< workTime.at(0) << setw(5) << "ms"
		<< workTime.at(1) << setw(5) << "ms"
		<< workTime.at(2) << setw(5) << "ms"
		<< workTime.at(3) << "ms\n";
}

int pow(int p)
{
	int res(1);
	for (int i = 1; i <= p; ++i) res *= 2;
	return res;
}

int main(int argc, char** argv)
{
	printHeader(L"Параллельные вычисления");

	const int NUMP(5);						// кол-во вариантов для потоков
	std::array<std::wstring, NUMP> potokStr{L"1 поток", L"2 потока", L"4 потока", L"8 потоков", L"16 потоков"};
	std::vector<double> workTime(VARSIZE);	// здась буду хранить время работы, для каждого размера
	std::vector<int> V1, V2;				// вектора для сложения
	
	std::vector<std::thread> thrs;			// потоки
	
	for (int p = 0; p < NUMP; ++p)
	{
		int num_potok = pow(p);				// вычисляю кол-во потоков
		thrs.resize(num_potok);				// устанавливаю кол-во потоков

		for (int i = 0; i < VARSIZE; ++i)
		{
			V1.resize(SIZEV[i]);			// устанавливаю размер данных
			V2.resize(SIZEV[i]);			// устанавливаю размер данных
			for (int j = 0; j < SIZEV[i]; ++j) // заполняю как то
			{
				V1.at(j) = j + 1;
				V2.at(j) = 10;
			}
			////////////////////////////////////
			int sizebl = SIZEV[i] / num_potok;	// размер блока данных 1го потока


			auto start = std::chrono::steady_clock::now();
			for (int k = 0; k < num_potok; ++k)
			{
				if (k != (num_potok - 1))
				{
					thrs.at(k) = (std::thread(
						summVect, std::ref(V1), std::ref(V2), sizebl * k, sizebl, std::ref(start)
					));
				}
				else
				{
					thrs.at(k) = (std::thread(
						summVect, std::ref(V1), std::ref(V2), sizebl * k, SIZEV[i] - sizebl * k, std::ref(start)
					));
				}
				thrs.at(k).join();
			}
			//for (auto& t : thrs) t.join();
			auto end = std::chrono::steady_clock::now();
			std::chrono::duration<double, std::milli> delta = end - start;
			workTime.at(i) = delta.count();
			////////////////////////////////////
		}

		print(potokStr.at(p), workTime);
	}


	
	std::wcout << "\n";

	return 0;
}
