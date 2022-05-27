#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "Message.h"


#pragma warning(disable : 4996)

HANDLE toRead;
HANDLE toWrite;
HANDLE mutex;
int readPosition = 0;
int sizeOfQueue;


void createBinaryFile(char* filename, int queueSize) {
	std::ofstream fout(filename, std::ios::binary);
	int position = 0;
	char p[10];
	itoa(position, p, 10);
	fout.write(p, sizeof(p));
	Message* m = new Message("empty", "empty");
	for (int i = 0; i < queueSize; i++) {
		fout.write((char*)&m, sizeof(Message));
	}
	fout.close();
}

void read(char* filename) {
	WaitForSingleObject(toRead, INFINITE);
	WaitForSingleObject(mutex, INFINITE);
	std::cout << "Message read position: " << readPosition << std::endl;

	std::fstream fin(filename, std::ios::binary | std::ios::in | std::ios::out);
	if (!fin.is_open()) {
		std::cout << "error\n";
		return;
	}

	Message* m = new Message();
	char writeIter[10];
	int pos = sizeof(writeIter) + sizeof(Message) * readPosition;

	fin.seekg(pos, std::ios::beg);
	fin.read((char*)m, sizeof(Message));
	std::cout << m->name << " - " << m->text << std::endl;

	fin.seekp(pos, std::ios::beg);
	m = new Message("deleted", "deleted");
	fin.write((char*)m, sizeof(Message));

	readPosition++;
	if (readPosition == sizeOfQueue) {
		readPosition = 0;
	}

	fin.close();
	ReleaseMutex(mutex);
	ReleaseSemaphore(toWrite, 1, NULL);
}


int main() {
	std::cout << "Enter file name:\n";
	char filename[100];
	std::cin >> filename;

	std::cout << "Enter size of queue:\n";
	std::cin >> sizeOfQueue;

	std::cout << "Enter count of Sender processes:\n";
	int amountOfSenderProcesses;
	std::cin >> amountOfSenderProcesses;
	toRead = CreateSemaphore(NULL, 0, sizeOfQueue, "Queue is full");
	toWrite = CreateSemaphore(NULL, sizeOfQueue, sizeOfQueue, "Queue is empty");
	mutex = CreateMutex(NULL, FALSE, "Mutex");

	createBinaryFile(filename, sizeOfQueue);

	STARTUPINFO* si = new STARTUPINFO[amountOfSenderProcesses];
	PROCESS_INFORMATION* pi = new PROCESS_INFORMATION[amountOfSenderProcesses];
	char data[50] = "Sender.exe ";
	strcat(data, filename);
	strcat(data, " ");
	strcat(data, std::to_string(sizeOfQueue).c_str());

	for (int i = 0; i < amountOfSenderProcesses; i++) {
		ZeroMemory(&si[i], sizeof(STARTUPINFO));
		si[i].cb = sizeof(STARTUPINFO);

		if (!CreateProcess(NULL, data, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si[i], &pi[i]))
		{
			std::cout << "The new process is not created.\n";
			break;
		}
	}

	while (true) {
		std::cout << "1 - Read\n2 - Exit\n";
		int key;
		std::cin >> key;
		if (key == 1) {
			std::cout << "Read message is: \n";
			read(filename);
		}
		else if (key == 2) {
			break;
		}
	}

	CloseHandle(toRead);
	CloseHandle(toWrite);
	CloseHandle(mutex);
	return 0;
}