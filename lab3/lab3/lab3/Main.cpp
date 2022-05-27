#include <iostream>
#include <Windows.h>

using namespace std;

HANDLE start;

class SyncQueue {
private:
	int* array;
	int size;
	int addIter;
	int removeIter;
	HANDLE availableToDelete;
	HANDLE availableToAdd;
	CRITICAL_SECTION critQueue;
public:
	SyncQueue(int size) {
		this->size = size;
		this->array = new int[size];
		this->addIter = 0;
		this->removeIter = 0;
		availableToDelete = CreateSemaphore(NULL, 0, size, NULL);
		availableToAdd = CreateSemaphore(NULL, size, size, NULL);
		InitializeCriticalSection(&critQueue);
	}

	~SyncQueue() {
		DeleteCriticalSection(&critQueue);
		CloseHandle(availableToAdd);
		CloseHandle(availableToDelete);
		delete[] array;
	}

	void Insert(int element) {
		WaitForSingleObject(availableToAdd, INFINITE);

		EnterCriticalSection(&critQueue);
		array[addIter++] = element;
		if (addIter == size)
		{
			addIter = 0;
		}
		LeaveCriticalSection(&critQueue);

		ReleaseSemaphore(availableToDelete, 1, NULL);
	}

	int Remove() {
		WaitForSingleObject(availableToDelete, INFINITE);

		EnterCriticalSection(&critQueue);
		int element = array[removeIter++];
		if (removeIter == size)
		{
			removeIter = 0;
		}
		LeaveCriticalSection(&critQueue);

		ReleaseSemaphore(availableToAdd, 1, NULL);

		return element;
	}
};


class Producer {
public:
	Producer() {

	}
	Producer(SyncQueue *&myQue, int ProdId, int ProdCount) {
		this->myQue = myQue;
		this->ProdCount = ProdCount;
		this->ProdId = ProdId;
	}
	int getProdId() {
		return this->ProdId;
	}
	int getProdCount() {
		return this->ProdCount;
	}
	void ProdQueInsert(int val) {
		this->myQue->Insert(val);
	}

private:
	SyncQueue* myQue;
	int ProdId;
	int ProdCount;
};


class Consumer {
public:
	Consumer() {

	}
	Consumer(SyncQueue *&myQue, int ConsCount){
		this->myQue = myQue;
		this->ConsCount = ConsCount;
	}
	int getConsCount() {
		return this->ConsCount;
	}
	int getConsQueElem() {
		return this->myQue->Remove();
	}

private:
	SyncQueue* myQue;
	int ConsCount;
};

DWORD WINAPI TreadProcProducer(LPVOID lpParametrs) {
	WaitForSingleObject(start, INFINITE);

	Producer* currentProd = (Producer*)lpParametrs;
	
	for (int i = 0; i < currentProd->getProdCount(); i++) {
		currentProd->ProdQueInsert(currentProd->getProdId()+i);
		cout << "Produce number: " << currentProd->getProdId()+i << "\n";
		Sleep(700);
	}

	return 0;
}

DWORD WINAPI ThreadProcConsumer(LPVOID lpParametrs) {
	WaitForSingleObject(start, INFINITE);

	Consumer* currentCons = (Consumer*)lpParametrs;

	for (int i = 0; i < currentCons->getConsCount(); i++) {
		cout << "Consumer number: " << currentCons->getConsQueElem() << "\n";
		Sleep(700);
	}

	return 0;
}

int main() {

	setlocale(LC_ALL, ".1251");

	int queueSize;
	int prodCount;
	start = CreateEvent(NULL, TRUE, FALSE, NULL);

	cout << "Enter queue size:\n";
	cin >> queueSize;

	SyncQueue* queue = new SyncQueue(queueSize);

	cout << "Enter Producer count:\n";
	cin >> prodCount;

	Producer* prodList = new Producer[prodCount];
	for (int i = 0; i < prodCount; i++) {
		cout << "Enter count to produce in " << i + 1 << " thread:\n";
		int tmp;
		cin >> tmp;
		prodList[i] = Producer(*&queue, i + 1, tmp);
	}

	HANDLE* producerHandles = new HANDLE[prodCount];
	DWORD* producerHandleIds = new DWORD[prodCount];

	for (int i = 0; i < prodCount; i++) {
		producerHandles[i] = CreateThread(NULL, 0, TreadProcProducer, (void*)&prodList[i], 0, &producerHandleIds[i]);
	}


	cout << "Enter Consumer count:\n";
	int consumerCount;
	cin >> consumerCount;

	Consumer* consList = new Consumer[consumerCount];

	for (int i = 0; i < consumerCount; i++) {
		cout << "Enter count of numbers to remove in " << i + 1 << " thread:\n";
		int tmp;
		cin >> tmp;
		consList[i] = Consumer(*&queue, tmp);
	}

	HANDLE* consumerHandles = new HANDLE[consumerCount];
	DWORD* consumerHandleIds = new DWORD[consumerCount];

	for (int i = 0; i < consumerCount; i++) {
		consumerHandles[i] = CreateThread(NULL, 0, ThreadProcConsumer, (void*)&consList[i], 0, &consumerHandleIds[i]);
	}

	HANDLE* allHandles = new HANDLE[prodCount + consumerCount];

	for (int i = 0; i < prodCount; i++) {
		allHandles[i] = producerHandles[i];
	}

	for (int i = 0; i < consumerCount; i++) {
		allHandles[i + prodCount] = consumerHandles[i];
	}

	SetEvent(start);

	WaitForMultipleObjects(prodCount + consumerCount, allHandles, TRUE, INFINITE);

	CloseHandle(start);

	for (int i = 0; i < prodCount; i++) {
		CloseHandle(producerHandles[i]);
	}

	for (int i = 0; i < consumerCount; i++) {
		CloseHandle(consumerHandles[i]);
	}

	return 0;
}