#include <iostream>
#include "windows.h"
#include <string>
#include <thread>

#pragma warning(disable: 4996)

using namespace std;

struct Parameter {
	int size;
	double* coefficients;
	double x;
	double result = 0;
};

class PolynomParams {
public:
	PolynomParams(int size, double* coef, double x) {
		this->size = size;
		this->x = x;
		this->coefs = coef;
		this->result = 0;
	}
	int getSize() {
		return this->size;
	}
	double getCoef(int i) {
		return this->coefs[i];
	}
	double getX() {
		return this->x;
	}
	void setResult(double result) {
		this->result = result;
	}
	double getResult() {
		return this->result;
	}
private:
	int size;
	double* coefs;
	double x;
	double result;
};


DWORD WINAPI ThreadProc(LPVOID lpParameters) {
	PolynomParams* parameter = (PolynomParams*)lpParameters;

	double result = 0;
	for (int i = 0; i < parameter->getSize(); i++) {
		double addend = parameter->getCoef(i);
		for (int j = 0; j < parameter->getSize() - i; j++) {
			addend *= parameter->getX();
		}
		result += addend;
		Sleep(15);
	}

	parameter->setResult(result);
	cout << "\n" << "p(" << parameter->getX() << ") = " << result << "\n";
	return 0;
}


HANDLE startPolynomThread(PolynomParams &parameter) {
	DWORD lpThreadId;
	HANDLE handle;

	handle = CreateThread(NULL, 0, ThreadProc, (void*)&parameter, 0, &lpThreadId);
	if (!handle) {
		cout << "Поток не был создан." << "\n";
	}

	return handle;
}

int main() {
	setlocale(LC_ALL, ".1251");
	cout << "Введите степень полинома в числителе:" << "\n";
	int PolNumDegree;
	cin >> PolNumDegree;

	cout << "\n" << "Введите коэффициенты полинома в числителе:" << "\n";
	double* PolNumCoef = new double[PolNumDegree];
	for (int i = 0; i < PolNumDegree; i++) {
		cin >> PolNumCoef[i];
	}

	cout << "\n" << "Введите степень полинома в знаменателе:" << "\n";
	int PolDenumDegree;
	cin >> PolDenumDegree;

	cout << "\n" << "Введите коэффициенты полинома в знаменателе:" << "\n";
	double* PolDenumCoef = new double[PolDenumDegree];
	for (int i = 0; i < PolDenumDegree; i++) {
		cin >> PolDenumCoef[i];
	}

	cout << "\n" << "Введите x:" << "\n";
	double x;
	cin >> x;

	PolynomParams param1(PolNumDegree, PolNumCoef, x);
	PolynomParams param2(PolDenumDegree, PolDenumCoef, x);

	HANDLE* handles = new HANDLE[2];

	handles[0] = startPolynomThread(param1);
	handles[1] = startPolynomThread(param2);


	WaitForMultipleObjects(2, handles, TRUE, INFINITE);

	CloseHandle(handles[0]);
	CloseHandle(handles[1]);

	cout << "f(" << x << ") = " << param1.getResult() / param2.getResult();

	return 0;
}