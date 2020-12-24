#pragma comment (lib, "winmm.lib")
#include "windows.h"
#include "iostream"
#include "fstream"
#include "time.h"
using namespace std;
int main() {
	srand(time(NULL));
	const int numOfPages = 13;
	char nameOfMutex[] = { "mutex" };
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	int sizeOfPage = sysInfo.dwPageSize;
	int sizeOfBuff = sizeOfPage * numOfPages;
	fstream journal;
	journal.open("C:\\Users\\tyuri\\Documents\\WJournal.txt", fstream::app);
	char* message = new char[sizeOfPage];
	memset(message, 39 + rand() % 11, sizeOfPage);
	HANDLE file = CreateFile(L"C:\\Users\\tyuri\\Documents\\buffer.txt", GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE buffer = OpenFileMapping(GENERIC_WRITE, NULL, L"buffer");
	if (buffer == NULL)
		buffer = CreateFileMapping(file, NULL, PAGE_READWRITE, 0, sizeOfBuff, L"buffer");
	LPVOID buffAddress = MapViewOfFile(buffer, FILE_MAP_WRITE, 0, 0, sizeOfBuff);
	VirtualLock(buffAddress, sizeOfBuff);
	HANDLE* arr1 = new HANDLE[numOfPages];
	char call1[] = { "Strw" };
	for (int i = 0; i < numOfPages; ++i) {
		call1[2] = i + '0';
		arr1[i] = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, call1);
		if (arr1[i] == NULL) {
			arr1[i] = CreateSemaphoreA(NULL, 1, 1, call1);
			if (arr1[i] == NULL)
				cout << " Ошибка создания семафора страницы: " << GetLastError()<< endl;
		}
	}
	HANDLE Semaph = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, "S");
	if (Semaph == NULL) {
		Semaph = CreateSemaphoreA(NULL, 1, 1, "Semaph");
		if (Semaph == NULL)
			cout <<  "Ошибка создания семафора действия: " << GetLastError() << endl;
	}
	while (true) {
		WaitForSingleObject(Semaph, INFINITE);
		journal << GetCurrentProcessId() << ' ' << timeGetTime() << " Ожидание " <<
			endl;
		ReleaseSemaphore(Semaph, 1, NULL);
		int currentPage = WaitForMultipleObjects(numOfPages, arr1, false,
			INFINITE);
		WaitForSingleObject(Semaph, INFINITE);
		journal << GetCurrentProcessId() << ' ' << timeGetTime() << " Начало записи "
			<< currentPage << endl;
		ReleaseSemaphore(Semaph, 1, NULL);
		cout << (int)buffAddress + currentPage * sizeOfPage << " " << currentPage <<
			endl;
		memcpy((void*)((int)buffAddress + currentPage * sizeOfPage), message,
			sizeOfPage);
		Sleep(500 + rand() % 1000);
		WaitForSingleObject(Semaph, INFINITE);
		journal << GetCurrentProcessId() << ' ' << timeGetTime() << " Прекращение записи " << currentPage << endl;
		ReleaseSemaphore(Semaph, 1, NULL);
	}
	return 0;
}
