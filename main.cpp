#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <string>
#include <ctime>
#include <unistd.h>

using namespace std;

sem_t rooms; // Семафор, отображающий свободные места в отеле
pthread_mutex_t print; // Мьютекс для печати в консоль
pthread_mutex_t personMutex; // Мьютекс для работы с id клиента
time_t startTime; // Начальное время (время старта программы)
int dayLength; // Длина дня (число секунд в дне)
int maxTimeInteral; // Верхняя граница задержки между появлениями клиентов (не включительно)
int hourLength; // Число секунд в часе
int clientID = 1; // Свобоный id клиента
int clientNum; // Число клиентов
/// <summary>
/// Возвращает текущее время в гостинице
/// </summary>
/// <returns>Текущее время в гостинице</returns>
int getTime() {
	return ((time(0) - startTime + dayLength / 2) % dayLength) / (hourLength);
}
/// <summary>
/// Ночевка клиента в гостинице
/// </summary>
void* rent(void* param) {
	pthread_mutex_lock(&personMutex);

	int num = clientID;
	clientID++;

	pthread_mutex_lock(&print); // Выводим сообщение о клиенте, который зашел в гостиницу
	cout << "Client " << num << " went to the hotel at " << getTime() << ":00" << endl;
	pthread_mutex_unlock(&print);

	pthread_mutex_unlock(&personMutex);

	srand((unsigned)time(0));

	sem_wait(&rooms); // Клиент запрашивает свободный номер

	pthread_mutex_lock(&print); // Выводим сообщение о брони клиентом номера
	cout << "Client " << num << " booked a room at " << getTime() << ":00" << endl;
	pthread_mutex_unlock(&print);

	auto currentTime = (time(0) - startTime) % dayLength; // Вычисляем сколько времени прошло с момента открытия гостиницы
	int sleepTime; // Время, которое клиент проведет в гостинице
	if (currentTime >= 18 * hourLength) {
		sleepTime = dayLength - currentTime + 18 * hourLength;
	}
	else {
		sleepTime = 18 * hourLength - currentTime;
	}
	int d = rand() % 4; // Получаем время, которое клиент проведет в гостинице после 6 часов утра
	sleepTime += d * hourLength;
	sleep(sleepTime); // Клиент остается в своем номере

	pthread_mutex_lock(&print); // Выводим сообщение о клиенте, который покинул номер
	cout << "Client " << num << " vacated the room at " << getTime() << ":00" << endl;
	pthread_mutex_unlock(&print);

	sem_post(&rooms); // Клиент покинул свой номер
	return 0;
}

int main(int argc, char** argv) {
	if (argc != 4) { // Проверяем число параметров
		std::cout << "Wrong number of console arguments" << std::endl;
		return 1;
	}
	if (std::stol(argv[1]) <= 0) { // Проверка длины часа
		std::cout << "Wrong hour length" << std::endl;
		return 1;
	}
	if (std::stol(argv[2]) <= 0) { // Проверка верхней границы задержки
		std::cout << "Wrong time interval" << std::endl;
		return 1;
	}
	if (std::stol(argv[3]) < 0) { // Проверка числа клиентов
		std::cout << "Wrong number of clients" << std::endl;
		return 1;
	}
	hourLength = std::stoul(argv[1]);
	maxTimeInteral = std::stol(argv[2]) + 1;
	dayLength = hourLength * 24;
	clientNum = std::stoul(argv[3]);


	srand((unsigned)time(NULL));
	pthread_mutex_init(&print, NULL);
	pthread_mutex_init(&personMutex, NULL);
	sem_init(&rooms, 0, 30); // Инициализируем семафор гостиницы с 30 местами

	startTime = time(0);

	pthread_mutex_lock(&print); // Выводим сообщение об открытии гостиницы
	cout << "Hotel opened at 12:00" << endl;
	pthread_mutex_unlock(&print);

	for (int i = 0; i < clientNum; i++)
	{
		// Создаем поток-клиента
		pthread_t person;
		pthread_create(&person, NULL, rent, NULL);

		if (i != clientNum - 1) {
			// Генерируем задержку между клиентами
			int sleepTime = (rand() % maxTimeInteral) * hourLength;
			sleep(sleepTime);
		}
	}
	int freeRooms = 0;
	while (freeRooms != 30) { // Ждем пока освободятся все номера
		sem_getvalue(&rooms, &freeRooms);
	}
	return 0;
}

