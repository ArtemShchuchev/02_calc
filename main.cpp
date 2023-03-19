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
const int NUMP(5);							// кол-во вариантов для потоков
std::array<std::wstring, NUMP> potokStr{L"1 поток", L"2 потока", L"4 потока", L"8 потоков", L"16 потоков"};

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
	/*
	ОБАЛДЕТЬ!!!!!!!!!!!!!
	Знаете, что самое смешное... я бы именно так и записал, т.е. расчеты
	переменных цикла вынес бы из цикла (я бы еще и итераторы it2 и it вынес),
	НО когда я стал учиться с++, я именно так и делал, на что мне сказали, - 
	"Не майся дурью, компилятор умнее тебя, он сам всё сделает"!!!
	Вот и сделал (компилятор)! ))
	Спасибо, было очень интересно узнать причину, теперь все работает!
	
	Вставил картинку со своим исполнением на ГитХаб (ну, один из вариантов,
	она всякий раз немного разная, но теперь все логично и быстро).
	*/
	auto end = v1.begin() + offset_begin + num_byte;
	for (auto it2 = v2.begin() + offset_begin, it = v1.begin() + offset_begin;
		it != end; ++it, ++it2)
		*it += *it2;
}

// вывод инфо
void print(const std::array <std::array<double, VARSIZE>, NUMP>& workTime)
{
	using namespace std;
	wcout.setf(ios::fixed);
	
	// заполняю массив минимумов значениями для 1го потока
	std::array<double, VARSIZE> mintime{ 0 };
	for (size_t vs = 0; vs < workTime[0].size(); ++vs)
	{
		mintime[vs] = workTime[0][vs];
	}

	// ищу поток с минимальным значением времени, сохраняю время
	for (size_t vs = 0; vs < workTime[0].size(); ++vs)
	{
		for (size_t p = 1; p < workTime.size(); p++)
		{
			if (mintime[vs] > workTime[p][vs]) mintime[vs] = workTime[p][vs];
		}
	}

	for (size_t p = 0; p < workTime.size(); p++)
	{
		wcout << setprecision(3) << left << setw(11) << potokStr.at(p);
		for (size_t vs = 0; vs < mintime.size(); ++vs)
		{
			if (mintime[vs] == workTime[p][vs]) consoleCol(col::br_cyan);
			wcout << workTime[p][vs] << setw(5) << "ms";
			consoleCol(col::cancel);
		}
		wcout << "\n";
	}
	consoleCol(col::br_cyan);
	std::wcout << L"\nМинимальное время среди потоков.\n";
	consoleCol(col::cancel);
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

	std::array <std::array<double, VARSIZE>, NUMP> workTime{ 0 };
	std::vector<int> V1, V2;				// вектора для сложения
	
	for (int p = 0; p < NUMP; ++p)
	{
		int num_potok = pow(p);				// вычисляю кол-во потоков
		std::vector<std::thread> thrs(num_potok);	// потоки

		for (int vs = 0; vs < VARSIZE; ++vs)
		{
			V1.resize(SIZEV[vs]);			// устанавливаю размер данных
			V2.resize(SIZEV[vs]);			// устанавливаю размер данных
			for (int j = 0; j < SIZEV[vs]; ++j) // заполняю как то
			{
				V1.at(j) = j + 1;
				V2.at(j) = 10;
			}
			////////////////////////////////////
			int sizebl = SIZEV[vs] / num_potok;	// размер блока данных 1го потока


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
						summVect, std::ref(V1), std::ref(V2), sizebl * k, SIZEV[vs] - sizebl * k, std::ref(start)
					));
				}
			}
			for (auto& t : thrs) t.join();
			auto end = std::chrono::steady_clock::now();
			std::chrono::duration<double, std::milli> delta = end - start;
			workTime.at(p).at(vs) = delta.count();
			////////////////////////////////////
		}
	}

	print(workTime);
	
	std::wcout << "\n";

	return 0;
}
