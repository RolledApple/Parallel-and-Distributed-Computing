#include <iostream>
#include <string>
#include <random>
#include <numeric>
#include <functional>
#include <execution>
#include <cmath>
#include <mpi.h>

using namespace std;
constexpr double F = 0.00000001;

// Taylor series for sin
double mySin(double x)
{
	constexpr int TERMS = 80;

	const double mSqX = -x * x;
	double res = x, term = x;
	for (int i = 1; i < TERMS; ++i)
	{
		term *= mSqX / (2.0 * i) / (2.0 * i + 1.0);
		res += term;
	}
	return res;
}

// Generate test data into two vectors
void generateDataInto(int n, vector<double>& a, vector<double>& x)
{
	random_device rd;
	mt19937 engine(rd());
	uniform_real_distribution<double> dist(-5.0, 5.0);
	generate(begin(a), end(a), [&]() {return dist(engine); });
	for (int i = 0; i < n; ++i)
		x[i] = F * i;
}

// Calculate counts and displacement of corresponding chunk for each process
void calcCountsAndDisplacements(int n, int pCnt,
	vector<int>& counts, vector<int>& displs)
{
	const int basicCnt = n / pCnt, procCntWithExtraTerm = n % pCnt;
	int curDisp = 0;
	for (int i = 0; i < pCnt; ++i)
	{
		displs[i] = curDisp;
		counts[i] = basicCnt + (i < procCntWithExtraTerm ? 1 : 0);
		curDisp += counts[i];
	}
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cerr << "Size of array missing" << endl;
		return -1;
	}
	const int n = atoi(argv[1]);
	MPI_Init(&argc, &argv);
	int pCnt, pid;
	MPI_Comm_size(MPI_COMM_WORLD, &pCnt);
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);

	const int curTermCnt = n / pCnt + (pid < n% pCnt);
	vector<int> counts(pid == 0 ? pCnt : 0), displs(pid == 0 ? pCnt : 0);
	if (pid == 0)
	{
		calcCountsAndDisplacements(n, pCnt, counts, displs);
		cout << "Process count: " << pCnt << endl;
		cout << "Element count: " << n << endl;
	}
	const string pTitle = "P" + std::to_string(pid) + ": ";
	cout << pTitle << "processing " << curTermCnt << " elements" << endl;

	vector<double> a(pid == 0 ? n : 0), x(pid == 0 ? n : 0),
		aCur(curTermCnt), xCur(curTermCnt);
	double startT, endT, total = 0.0;
	if (pid == 0)
	{
		generateDataInto(n, a, x);
		startT = MPI_Wtime();
	}

	// Function for term evaluation using our mySin function for sin
	auto termFn = [](double a, double x) {return a * mySin(x); };
	// Function for term evaluation using standard function for sin
	auto termFnSt = [](double a, double x) {return a * sin(x); };

	MPI_Scatterv(a.data(), counts.data(), displs.data(), MPI_DOUBLE, aCur.data(),
		curTermCnt, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Scatterv(x.data(), counts.data(), displs.data(), MPI_DOUBLE, xCur.data(),
		curTermCnt, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	const double sum = transform_reduce(begin(aCur), end(aCur), begin(xCur),
		0.0, plus(), termFn);
	MPI_Barrier(MPI_COMM_WORLD); // Wait for all processes at this position
	MPI_Reduce(&sum, &total, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	if (pid == 0)
	{
		endT = MPI_Wtime();
		cout << "Execution time: " << endT - startT << "s" << endl;
		cout << "Result using MPI with mySin: " << total << endl;

		const double resSeqSt = transform_reduce(
			begin(a), end(a), begin(x), 0.0, plus(), termFnSt);
		cout << "Result using sequential code with standard sin: "
			<< resSeqSt << endl;
	}

	MPI_Finalize();
	return 0;
}