#include <iostream>
#include <iomanip>
#include <chrono>

using namespace std;

using byte = unsigned char;
using MatrixProduct = void(byte inRow[7], byte inMatrix[7][8], byte outRow[7]);
using FMatrixProduct = void(float inRow[7], float inMatrix[7][8], float outRow[7]);

// declare function from assembler code
extern "C"
{
	void multX64(byte inRow[7], byte inMatrix[7][8], byte outRow[7]);
	void multMMX(byte inRow[7], byte inMatrix[7][8], byte outRow[7]);
	void multSSE(byte inRow[7], byte inMatrix[7][8], byte outRow[7]);
	void fmultX64(float inRow[7], float inMatrix[7][8], float outRow[7]);
	void fmultSSE(float inRow[7], float inMatrix[7][8], float outRow[7]);
}
// checking results using cpp code
void multCPP(byte inRow[7], byte inMatrix[7][8], byte outRow[7])
{
	for (int i = 0; i < 7; ++i)
		outRow[i] = 0;
	for (int i = 0; i < 7; ++i)
		for (int j = 0; j < 7; ++j)
			outRow[j] += inRow[i] * inMatrix[i][j];
}
// same for float
void fmultCPP(float inRow[7], float inMatrix[7][8], float outRow[7])
{
	for (int i = 0; i < 7; ++i)
		outRow[i] = 0;
	for (int i = 0; i < 7; ++i)
		for (int j = 0; j < 7; ++j)
			outRow[j] += inRow[i] * inMatrix[i][j];
}

void testFunction(MatrixProduct function, byte inRow[7], byte inMatrix[7][8])
{
	using namespace chrono;
	constexpr int NUM_TESTS = 10000000;

	alignas(16) byte outBuf[8];

	auto startTime = high_resolution_clock::now();
	for (int i = 0; i < NUM_TESTS; ++i)
		function(inRow, inMatrix, outBuf);
	auto duration = (double)(duration_cast<nanoseconds>(
		high_resolution_clock::now() - startTime).count()) / NUM_TESTS;

	cout << "Average execution time: " << duration << "ns" << endl;
	cout << "Result: ";
	for (int i = 0; i < 7; ++i)
		cout << (int)outBuf[i] << ' ';
	cout << endl;
}

void testFloatFunction(FMatrixProduct function, float inRow[7], float inMatrix[7][8])
{
	using namespace chrono;
	constexpr int NUM_TESTS = 10000000;

	alignas(16) float outBuf[8];
	
	auto startTime = high_resolution_clock::now();
	for (int i = 0; i < NUM_TESTS; ++i)
		function(inRow, inMatrix, outBuf);
	auto duration = (double)(duration_cast<nanoseconds>(
		high_resolution_clock::now() - startTime).count()) / NUM_TESTS;

	cout << "Average execution time: " << duration << "ns" << endl;
	cout << "Result: ";
	auto oldFlags = cout.flags();
	cout << fixed << setprecision(2);
	for (int i = 0; i < 7; ++i)
		cout << outBuf[i] << ' ';
	cout.precision();
	cout.flags(oldFlags);
	cout << endl;
}

int main()
{
	alignas(16) byte row[7] = {}, matrix[7][8] = {};
	alignas(16) float frow[7] = {}, fmatrix[7][8] = {};

	int tmp;
	cout << "----- BYTE VARIANTS -----\n" << endl;
	cout << "Input row: ";
	for (int i = 0; i < 7; ++i)
	{
		cin >> tmp;
		row[i] = (byte)tmp;
	}
	cout << "Input matrix: ";
	for (int i = 0; i < 7; ++i)
		for (int j = 0; j < 7; ++j)
		{
			cin >> tmp;
			matrix[i][j] = (byte)tmp;
		}
	cout << "\n--- CPP ---" << endl;
	testFunction(multCPP, row, matrix);
	cout << "\n--- X64 without extensions ---" << endl;
	testFunction(multX64, row, matrix);
	cout << "\n--- X64 with MMX ---" << endl;
	testFunction(multMMX, row, matrix);
	cout << "\n--- X64 with SSE ---" << endl;
	testFunction(multSSE, row, matrix);

	cout << "\n----- FLOAT VARIANTS -----\n" << endl;
	cout << "Input row: ";
	for (int i = 0; i < 7; ++i)
		cin >> frow[i];
	cout << "Input matrix: ";
	for (int i = 0; i < 7; ++i)
		for (int j = 0; j < 7; ++j)
			cin >> fmatrix[i][j];
	cout << "\n--- CPP ---" << endl;
	testFloatFunction(fmultCPP, frow, fmatrix);
	cout << "\n--- X64 with SSE with scalar add/multiply ---" << endl;
	testFloatFunction(fmultX64, frow, fmatrix);
	cout << "\n--- X64 with SSE with parallel add/multiply ---" << endl;
	testFloatFunction(fmultSSE, frow, fmatrix);
	
	return 0;
}