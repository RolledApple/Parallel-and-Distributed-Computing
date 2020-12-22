#include <iostream>
#include <chrono>

using namespace std;
using ModExpFn = uint64_t(uint64_t b, uint64_t n, uint64_t m);
constexpr uint32_t INTRA_REPEATS = 100000;

uint64_t binary(uint64_t b, uint64_t n, uint64_t m)
{
	uint64_t res = 1, b2, resn = res;

	for (uint64_t i = 0; i < 64; ++i)
	{
		if (n & (1ull << i))
			// for (uint64_t _ = 0; _ < INTRA_REPEATS; ++_)
			resn = res * b % m;

		// for (uint64_t _ = 0; _ < INTRA_REPEATS; ++_)
		b2 = b * b % m;

		b = b2;
		res = resn;
	}

	return res;
}

uint64_t binaryPar(uint64_t b, uint64_t n, uint64_t m)
{
	uint64_t res = 1, b2, resn = res;

	for (uint64_t i = 0; i < 64; ++i)
	{
#pragma omp parallel sections num_threads(2)
		{
#pragma omp section
			{
				if (n & (1ull << i))
					// for (uint64_t _ = 0; _ < INTRA_REPEATS; ++_)
					resn = res * b % m;
			}
#pragma omp section
			{
				// for (uint64_t _ = 0; _ < INTRA_REPEATS; ++_)
				b2 = b * b % m;
			}
		}

		b = b2;
		res = resn;
	}

	return res;
}

uint64_t montgomery(uint64_t b, uint64_t n, uint64_t m)
{
	uint64_t res1 = 1, res2 = b, res1n, res2n;

	for (uint64_t i = 63; i != (uint64_t)(-1); --i)
	{
		if (n & (1ull << i))
		{
			res1n = (res1 * res2) % m;
			res2n = (res2 * res2) % m;
		}
		else
		{
			res1n = (res1 * res1) % m;
			res2n = (res1 * res2) % m;
		}
		res1 = res1n;
		res2 = res2n;
	}

	return res1;
}

uint64_t montgomeryPar(uint64_t b, uint64_t n, uint64_t m)
{
	uint64_t res1 = 1, res2 = b, res1n, res2n;

	for (uint64_t i = 63; i != (uint64_t)(-1); --i)
	{
#pragma omp parallel sections num_threads(2)
		{
#pragma omp section
			{
				if (n & (1ull << i))
					res1n = (res1 * res2) % m;
				else
					res1n = (res1 * res1) % m;
			}
#pragma omp section
			{
				if (n & (1ull << i))
					res2n = (res2 * res2) % m;
				else
					res2n = (res1 * res2) % m;
			}
		}

		res1 = res1n;
		res2 = res2n;
	}

	return res1;
}

uint64_t leftToRightAlgorithm(uint64_t b, uint64_t n, uint64_t m)
{
	uint64_t res = 1;

	for (uint64_t i = 63; i != (uint64_t)(-1); i--)
	{
		res = res * res % m;
		if (n & (1ull << i))
			res = res * b % m;
	}

	return res;
}

void testFunction(ModExpFn function, uint64_t b, uint64_t n, uint64_t m)
{
	using namespace chrono;
	constexpr int NUM_TESTS = 100000;

	uint64_t res;
	auto startTime = high_resolution_clock::now();
	for (int i = 0; i < NUM_TESTS; ++i)
		res = function(b, n, m);
	auto duration = (double)(duration_cast<nanoseconds>(
		high_resolution_clock::now() - startTime).count()) / NUM_TESTS;

	cout << "Average execution time: " << duration << "ms" << endl;
	cout << "Result: " << res << endl;
}

int main()
{
	uint64_t b, n, m;
	cout << "Input b, n, m: ";
	cin >> b >> n >> m;

	cout << "--- Binary algorithm ---" << endl;
	testFunction(binary, b, n, m);
	cout << "--- Parallel binary algorithm ---" << endl;
	testFunction(binaryPar, b, n, m);
	cout << "--- Montgomery algorithm ---" << endl;
	testFunction(montgomery, b, n, m);
	cout << "--- Parallel montgomery algorithm ---" << endl;
	testFunction(montgomeryPar, b, n, m);
	cout << "--- Left to right algorithm ---" << endl;
	testFunction(leftToRightAlgorithm, b, n, m);

	return 0;
}