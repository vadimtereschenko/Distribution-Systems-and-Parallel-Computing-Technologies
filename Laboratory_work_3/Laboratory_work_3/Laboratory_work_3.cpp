#include "mpi.h"
#include <iostream>

void printArray(int* arr, int n) {
	for (int i = 0; i < n; i++) {
		std::cout << arr[i] << " ";
	}
	std::cout << std::endl;
}

void initArray(int* arr, int n) {
	for (int i = 0; i < n; i++) {
		arr[i] = rand() % 10;
	}
}

int main(int argc, char** argv) {
	int procRank,
		procNum,
		residue,
		n;

	int sizes[2];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

	int* mainArr = NULL;
	int* sendcounts = NULL;
	int* displs = NULL;

	double result;

	if (procRank == 0) {
		std::cout << "\n\nEnter the size of the array (divisible by 100 and more than " << procNum*100 << "):";
		fflush(stdout);
		std::cin >> n;
		fflush(stdout);
		if (n % 100 != 0) {
			std::cout << "Error! Array size is not a divisible by 100." << std::endl;
			fflush(stdout);
			MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
		}
		if (n < (procNum * 100)) {
			std::cout << "Error! Array size is less than " << procNum*100 << std::endl;
			fflush(stdout);
			MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
		}

		mainArr = new int[n];
		initArray(mainArr, n);
		//std::cout << "\n\nMain array:" << std::endl;
		//printArray(mainArr, n);

		residue = n - (procNum * 100);
		//std::cout << "\n\nResidue: " << residue << std::endl;

		sendcounts = new int[procNum];
		displs = new int[procNum];
		for (int i = 0; i < procNum; i++) {
			sendcounts[i] = 100;
			sizes[0] = sendcounts[i];
			if (i == 0)
				displs[i] = 0;
			if (i != 0)
				displs[i] = displs[i - 1] + sendcounts[i - 1];
			if ((residue != 0) && (i == procNum - 1))
				sendcounts[i] += residue;
				sizes[1] = sendcounts[i];
		}
		//printArray(sendcounts, procNum);
		//printArray(displs, procNum);
		//printArray(sizes, 2);
	}

	MPI_Bcast(&sizes, 2, MPI_INT, 0, MPI_COMM_WORLD);

	int myBlockSize;
	myBlockSize = (procRank == (procNum - 1)) ? sizes[1] : sizes[0];
	int* res = new int[myBlockSize];
	MPI_Scatterv(mainArr, sendcounts, displs, MPI_INT, res, myBlockSize, MPI_INT, 0, MPI_COMM_WORLD);
	
	std::cout << "\n\nArray:" << std::endl;
	printArray(res, myBlockSize);

	double MatOj = 0;
	double Disp = 0;

	for (int i = 0; i < myBlockSize; i++)
		MatOj = res[i] + MatOj;
	MatOj = MatOj / myBlockSize; // математическое ожидание

	for (int i = 0; i < myBlockSize; i++)
		Disp = pow((res[i] - MatOj), 2) + Disp;
	Disp = Disp / (myBlockSize-1); // дисперсия

	std::cout << "Local dispersion:" << Disp << std::endl;

	MPI_Reduce(&Disp, &result, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);

	if (procRank == 0) {
		std::cout << "Result:" << result << std::endl;
		fflush(stdout);
	}

	MPI_Finalize();
	return 0;
}