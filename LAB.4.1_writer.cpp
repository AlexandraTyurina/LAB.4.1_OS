#pragma comment (lib, "winmm.lib")
#include "windows.h"
#include "iostream"
#include "fstream"
#include "time.h"
//системное время в миллисекундах.Системное время - это время, истекшее с момента старта Windows
using namespace std;
int main() {
	srand(time(NULL));

	const int numOfPages = 13;
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	int sizeOfPage = sysInfo.dwPageSize;
	int sizeOfBuff = sizeOfPage * numOfPages;
	//связать объект класса с файлом
	fstream journal;
	journal.open("C:\\Users\\tyuri\\Documents\\WJournal.txt", fstream::app);
	char* message = new char[sizeOfPage];
	//Функция возвращает указатель на блок памяти(memset заполняет sizeOfPage байтов блока памяти, через указатель message)
	memset(message, 50 + rand() % 10, sizeOfPage);
	HANDLE file = CreateFile(L"C:\\Users\\tyuri\\Documents\\buffer.txt", GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//открывает именованный объект "проецируемый файл"
	HANDLE buffer = OpenFileMapping(GENERIC_WRITE, NULL, L"buffer");
	if (buffer == NULL)
		//создает или открывает именованный или безымянный объект проецируемого файла для заданного file
		buffer = CreateFileMapping(file, NULL, PAGE_READWRITE, 0, sizeOfBuff, L"buffer");
	//отображает представление проецируемого файла в адресное пространство вызывающего процесса
	LPVOID buffAddress = MapViewOfFile(buffer, FILE_MAP_WRITE, 0, 0, sizeOfBuff);
	//страницы буферной памяти(т.е проецируемого файла) заблокированы
	VirtualLock(buffAddress, sizeOfBuff);
	HANDLE* arr = new HANDLE[numOfPages];
	char name_page[] = { "strw" };
	for (int i = 0; i < numOfPages; ++i) 
	{
		name_page[2] = i + '0';
		arr[i] = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, name_page);
		if (arr[i] == NULL) 
		{
			arr[i] = CreateSemaphoreA(NULL, 1, 1, name_page);
			if (arr[i] == NULL)
				cout << "Ошибка при создании семафоров страниц: " << GetLastError()
				<< endl;
		}
	}
	HANDLE Semph = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, "Semph");
	if (Semph == NULL) 
	{
		Semph = CreateSemaphoreA(NULL, 1, 1, "Semph");
		if (Semph == NULL)
			cout << "Ошибка при создании семафора действия: " << GetLastError() << endl;
	}
	while (true) {
		WaitForSingleObject(Semph, INFINITE);
		journal << GetCurrentProcessId() << ' ' << timeGetTime() << " Ожидание " <<endl;
		ReleaseSemaphore(Semph, 1, NULL);
		int currentPage = WaitForMultipleObjects(numOfPages, arr, false,INFINITE)-WAIT_OBJECT_0;
		WaitForSingleObject(Semph, INFINITE);
		journal << GetCurrentProcessId() << ' ' << timeGetTime() << " Начало записи " << currentPage << endl;
		ReleaseSemaphore(Semph, 1, NULL);
		cout << (int)buffAddress + currentPage * sizeOfPage << " " << currentPage <<endl;
		memcpy((void*)((int)buffAddress + currentPage * sizeOfPage), message, sizeOfPage);
		Sleep(400 + rand() % 800);
		WaitForSingleObject(Semph, INFINITE);
		journal << GetCurrentProcessId() << ' ' << timeGetTime() << " Прекращение записи " << currentPage << endl;
		ReleaseSemaphore(Semph, 1, NULL);
		ReleaseSemaphore(arr[currentPage], 1, NULL);
	}
	return 0;
}