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

// 1000, 10 000, 100 000 и 1 000 000 элементов
const int SIZEV[]{ 1000, 10000, 100000, 1000000 };
const int VARSIZE = sizeof SIZEV / sizeof SIZEV[0];	// кол-во вариантов размера массива
const int NUMP(5);									// кол-во вариантов для потоков
std::array<std::wstring, NUMP> potokStr{
	L"1 поток", L"2 потока", L"4 потока", L"8 потоков", L"16 потоков"};

// объявления функций
static void summVect(std::vector<int>::iterator v1_it,
	std::vector<int>::iterator v2_it, const int num_byte,
	std::chrono::steady_clock::time_point& st);
static void print(const std::array <std::array<double, VARSIZE>, NUMP>& workTime);
static int pow(int p);


int main(int argc, char** argv)
{
	printHeader(L"Параллельные вычисления");

	std::array <std::array<double, VARSIZE>, NUMP> workTime{ 0 };
	std::vector<int> V1, V2;				// вектора для сложения
	
	for (int p(0); p < NUMP; ++p)
	{
		int num_potok = pow(p);				// вычисляю кол-во потоков
		std::vector<std::thread> thrs(num_potok);	// потоки

		for (size_t vs(0); vs < VARSIZE; ++vs)
		{
			V1.resize(SIZEV[vs]);			// устанавливаю размер данных
			V2.resize(SIZEV[vs]);			// устанавливаю размер данных
			for (int j(0); j < SIZEV[vs]; ++j) // заполняю как то
			{
				V1[j] = j + 1;
				V2[j] = 10;
			}
			////////////////////////////////////

			auto sizebl = SIZEV[vs] / num_potok;	// размер блока данных 1го потока
			auto v1_it = V1.begin(), v2_it = V2.begin();
			std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
			for (int k(0); k < num_potok; ++k)
			{
				int num_byte = (k != (num_potok - 1)) ? sizebl : V1.end() - v1_it;

				thrs[k] = (std::thread(
					summVect, v1_it, v2_it, num_byte, std::ref(start) ));

				v1_it += sizebl;
				v2_it += sizebl;
			}
			for (auto& t : thrs) t.join();	// ждет окончания join потоков
			auto end = std::chrono::steady_clock::now();
			std::chrono::duration<double, std::milli> delta = end - start;
			workTime[p][vs] = delta.count();
		}
	}

	print(workTime);
	
	std::wcout << "\n";

	return 0;
}


// сумму 2х векторов складывает в 1ый вектор
static void summVect(std::vector<int>::iterator v1_it,
	std::vector<int>::iterator v2_it, const int num_byte,
	std::chrono::steady_clock::time_point& st)
{
	using namespace std;

	static std::once_flag flag;
	std::call_once(flag, [&st](){
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
	
	И запись стала как то понятней и проще!
	*/
	auto end = v1_it + num_byte;
	for (; v1_it != end; ++v1_it, ++v2_it) {
		*v1_it += *v2_it;
	}
}

// вывод инфо
static void print(const std::array <std::array<double, VARSIZE>, NUMP>& workTime)
{
	using namespace std;
	wcout.setf(ios::fixed);
	
	// ищу поток с минимальным значением времени, сохраняю время
	std::array<double, VARSIZE> mintime{ 0 };
	for (size_t vs(0); vs < VARSIZE; ++vs)
	{
		mintime[vs] = workTime[0][vs];
		for (size_t p(1); p < NUMP; ++p)
		{
			if (mintime[vs] > workTime[p][vs])
				mintime[vs] = workTime[p][vs];
		}
	}

	// вывод в консоль
	for (size_t p(0); p < NUMP; ++p)
	{
		wcout << setprecision(3) << left << setw(11) << potokStr[p];
		for (size_t vs(0); vs < VARSIZE; ++vs)
		{
			bool itsMin(mintime[vs] == workTime[p][vs]);
			if (itsMin) consoleCol(col::br_cyan);
			wcout << workTime[p][vs] << setw(5) << "ms";
			if (itsMin) consoleCol(col::cancel);
		}
		wcout << "\n";
	}
	consoleCol(col::br_cyan);
	std::wcout << L"\nМинимальное время среди потоков.\n";
	consoleCol(col::cancel);
}

// возведение в степень
static int pow(const int p)
{
	int res(1);
	for (int i(1); i <= p; ++i) res *= 2;
	return res;
}
