#include<iostream>
#include<conio.h>
#include<chrono>
#include<thread>
#include <Windows.h>
using namespace std;
using namespace std::literals::chrono_literals;
//#define MIN_TANK_VOLUME 20
//#define MAX_TANK_VOLUME 80

#define Enter 13
#define Escape 27
class Tank
{
	static const int MIN_TANK_VOLUME = 20;
	static const int MAX_TANK_VOLUME = 80;
	const int VOLUME;//объем бака
	double fuel_level;//уровень топлива
public:
	int get_VOLUME()const
	{
		return this->VOLUME;
	}
	double get_fuel_level()const
	{
		return this->fuel_level;
	}
	Tank(int VOLUME) :VOLUME
	(
		VOLUME < MIN_TANK_VOLUME ? MIN_TANK_VOLUME :
		VOLUME > MAX_TANK_VOLUME ? MAX_TANK_VOLUME :
		VOLUME
	)
	{
		//this->VOLUME = VOLUME;
		this->fuel_level = 0;
		//cout << "tank constructor:\t" << this << endl;
	}
	~Tank()
	{
		//cout << "tank destructor:\t" << this << endl;
	}
	void fill(double amount)
	{
		if (amount < 0)return;
		if (fuel_level + amount > VOLUME) fuel_level = VOLUME;
		else fuel_level += amount;
	}
	double give_fuel(double amount)
	{
		fuel_level -= amount;
		if (fuel_level < 0)fuel_level = 0;
		return fuel_level;
	}
	void info()const
	{
		cout << "VOLUME:\t" << this->VOLUME << endl;
		cout << "Fuel level:\t" << this->fuel_level << endl;
	}
};
class Engine
{
	static const int MAX_ENGINE_CONSUMPTION = 3;
	static const int MIN_ENGINE_CONSUMPTION = 30;
	const double CONSUMPTION;
	double consumption_per_sec;
	bool is_started;
public:
	double get_CONSUMPTION()const
	{
		return this->CONSUMPTION;
	}
	double get_consumption_per_sec(int speed)
	{
		if (speed > 200)consumption_per_sec = CONSUMPTION * 3e-5 * 10;
		else if (speed > 140)consumption_per_sec = CONSUMPTION * 3e-5 * 8.33333;
		else if (speed > 100)consumption_per_sec = CONSUMPTION * 3e-5 * 6.66666;
		else if (speed > 60)consumption_per_sec = CONSUMPTION * 3e-5 * 4.66666;
		else if (speed > 0)consumption_per_sec = CONSUMPTION * 3e-5 * 6.66666;
		else consumption_per_sec = CONSUMPTION * 3e-5;
		return this->consumption_per_sec;
	}
	Engine(double CONSUMPTION) :CONSUMPTION
	(
		CONSUMPTION < MIN_ENGINE_CONSUMPTION ? MIN_ENGINE_CONSUMPTION :
		CONSUMPTION > MAX_ENGINE_CONSUMPTION ? MAX_ENGINE_CONSUMPTION :
		CONSUMPTION
	)
	{
		//cout << "Econstructor:\t" << this << endl;
		this->is_started = false;
		this->consumption_per_sec = CONSUMPTION * 3e-5;
	}
	~Engine()
	{
		//cout << "Edestructor:\t" << this << endl;
	}
	void start()
	{
		is_started = true;
	}
	void stop()
	{
		is_started = false;
	}
	bool started()const
	{
		return is_started;
	}
	void info()const
	{
		cout << "Consumption per 100 km: " << CONSUMPTION << endl;
		cout << "Consumption per 1 sec: " << consumption_per_sec << endl;
	}
};
class Car
{
	Engine engine;
	Tank tank;
	bool driver_inside;
	int speed;
	const int MAX_SPEED;
	static const int MAX_SPEED_LOW_LIMIT = 90;
	static const int MAX_SPEED_HIGH_LIMIT = 390;

	struct Control
	{
		std::thread panel_thread;
		std::thread engine_idle_thread;
		std::thread free_wheeling_thread;
	}threads;
public:
	Car(int volume, int consumption, int maxspeed = 250) : tank(volume), engine(consumption),
		MAX_SPEED
		(
			maxspeed < MAX_SPEED_LOW_LIMIT ? MAX_SPEED_LOW_LIMIT :
			maxspeed > MAX_SPEED_HIGH_LIMIT ? MAX_SPEED_HIGH_LIMIT :
			maxspeed
		)
	{
		driver_inside = false;
		speed = 0;
		cout << "Your car is ready. Press enter to get in\n";
	}
	~Car()
	{
		cout << "Car is over.\n";
	}
	//methods
	void get_in()
	{
		driver_inside = true;
		threads.panel_thread = std::thread(&Car::panel, this);
	}
	void get_out()
	{
		driver_inside = false;
		if (threads.panel_thread.joinable())threads.panel_thread.join();
		system("cls");
		cout << "u are out of ur car.\n";
	}
	void start_engine()
	{
		if (driver_inside && tank.get_fuel_level())
		{
			engine.start();
			threads.engine_idle_thread = std::thread(&Car::engine_idle, this);
		}
	}
	void stop_engine()
	{
		if (driver_inside)
		{
			engine.stop();
			if (threads.engine_idle_thread.joinable())threads.engine_idle_thread.join();
		}
	}
	void accelerate()
	{
		if (engine.started() && speed < MAX_SPEED) {
			if (speed > MAX_SPEED) { speed = MAX_SPEED; }
			speed += 10;
			std::this_thread::sleep_for(1s);
		}
		if (!threads.free_wheeling_thread.joinable())threads.free_wheeling_thread = std::thread(&Car::free_wheeling, this);
	}
	void slow_down()
	{
		if (speed > 0)
		{
			speed -= 10;
			if (speed < 0)speed = 0;
			std::this_thread::sleep_for(1s);
		}
	}
	void control()
	{
		char key;
		do
		{
			key = 0;
			if (_kbhit())key = _getch();
			switch (key)
			{
			case 'W':case'w': accelerate();
				break;
			case'S':case's':slow_down();
				break;
			case Enter: driver_inside ? get_out() : get_in(); break;
			case 'F':case 'f':
				if (driver_inside)cout << "выходи из машины\n";
				else {
					double amount;
					cout << "введите кол-во топлива: "; cin >> amount;
					tank.fill(amount);
				}
				break;
			case 'i':case'I':engine.started() ? stop_engine() : start_engine();
				break;
			case Escape:stop_engine(); get_out(); break;
			}
			if (tank.get_fuel_level() == 0)stop_engine();
			if (speed <= 0)speed = 0;
			if (speed == 0 && threads.free_wheeling_thread.joinable()) threads.free_wheeling_thread.join();
		} while (key != Escape);
	}
	void info()const
	{
		tank.info();
		engine.info();
	}
	void engine_idle()
	{
		while (engine.started() && tank.give_fuel(engine.get_consumption_per_sec(speed)))
		{
			std::this_thread::sleep_for(1s);
		}
	}
	void free_wheeling()
	{
		while (--speed > 0)
		{
			if (speed < 0)speed = 0;
			std::this_thread::sleep_for(1s);
		}
	}
	void panel()
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		while (driver_inside)
		{
			system("CLS");
			//cout << "Consumption per sec:" << engine.get_consumption_per_sec(speed) << " liters\n";
			cout << "Fuel level:\t" << tank.get_fuel_level() << " liters\n ";
			if (tank.get_fuel_level() < 5)
			{
				SetConsoleTextAttribute(hConsole, 0xCF);//c-красный фон,f-белые буквы
				cout << " LOW FUEL";
				SetConsoleTextAttribute(hConsole, 0x07);//вернули обратно
			}
			cout << "\nEngine:\t" << (engine.started() ? "started" : "stopped") << endl;
			cout << "Speed: " << speed << " km/h\n" << endl;
			for (size_t i = 0; i < MAX_SPEED / 3; i++)
			{
				if (i > 50)SetConsoleTextAttribute(hConsole, 0x0E);
				if (i > 66)SetConsoleTextAttribute(hConsole, 0x0C);
				cout << (i <= speed / 2 ? "|" : ".");
				SetConsoleTextAttribute(hConsole, 0x7);
			}
		}
	}
};
//#define TANK_CHECK
//#define ENGINE_CHECK
/*некоторым макросам дают только им€ и не дают значение, такие макросы используютс€ с директивами
условной компил€ции #ifdef.. #endif*/

void main()
{
	setlocale(LC_ALL, "");
#if defined TANK_CHECK
	Tank tank(40);
	tank.info();
	do
	{
		int fuel;
		cout << "¬ведите объем топлива: "; cin >> fuel;
		tank.fill(fuel);
		tank.info();
	} while (_getch() != 27);
#endif //TANK_CHECK
#if defined ENGINE_CHECK
	Engine engine(10);
	engine.info();
#endif 
	Car bmw(80, 10);
	bmw.info();
	bmw.control();


}