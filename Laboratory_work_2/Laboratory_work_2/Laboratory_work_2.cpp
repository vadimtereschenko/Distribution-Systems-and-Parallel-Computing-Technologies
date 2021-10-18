#include <stdio.h>
#include "mpi.h"
#include <iostream>

void initArray(int* arr, int n) {
	for (int i = 0; i < n; i++) {
		arr[i] = rand() % 10;
	}
}

void printArray(int* arr, int n) {
	for (int i = 0; i < n; i++) {
		printf("%d ", arr[i]);
	}
	std::cout << std::endl;
}

int main(int argc, char* argv[])
{
	MPI_Request request;
	MPI_Status status;
	int procRank, n, procNum, root = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
	
	int* B = NULL;
	int* C = NULL;
	int* D = NULL;
	int* sizes = NULL;
	float exp = 2.718;

	int numOfParts = procNum - 1;
	sizes = new int[numOfParts];
	if (procRank == root) {
		int step, residue;
		std::cout << "Enter array size:";
		std::cin >> n;
		fflush(stdout);
		B = new int[n];
		C = new int[n];
		D = new int[n];
		//initArray(A, n);
		initArray(B, n);
		initArray(C, n);
		initArray(D, n);
		printf("Main arrays from process #%d:\n", procRank);
		printf("Array B: ");
		printArray(B, n);
		printf("Array C: ");
		printArray(C, n);
		printf("Array D: ");
		printArray(D, n);
		step = n / numOfParts;
		residue = n % numOfParts;
		//std::cout << "\nStep: " << step << std::endl;
		//std::cout << "\nResidue: " << residue << std::endl;
		int currentProc = 1;
		
		/* Цикл по процессам */
		int temp = 0;
		for (int i = 0; i < numOfParts; i++) {
			int subArraySize = n / numOfParts;
			if(residue != 0) {
				if (i == (numOfParts - 1)) {
					subArraySize += residue;
				}
			}
			//std::cout << "\nSub array size = " << subArraySize << std::endl;
			int* subB = new int[subArraySize];
			int* subC = new int[subArraySize];
			int* subD = new int[subArraySize];
			
			for (int j = 0; j < subArraySize; j++) {
				subB[j] = B[temp];
				subC[j] = C[temp];
				subD[j] = D[temp];
				temp++;
			}
			//printArray(subB, subArraySize);
			sizes[currentProc - 1] = subArraySize;
			MPI_Isend(&subArraySize, 1, MPI_INT, currentProc, 0, MPI_COMM_WORLD, &request);
			MPI_Isend(subB, subArraySize, MPI_INT, currentProc, 0, MPI_COMM_WORLD, &request);
			MPI_Isend(subC, subArraySize, MPI_INT, currentProc, 0, MPI_COMM_WORLD, &request);
			MPI_Isend(subD, subArraySize, MPI_INT, currentProc, 0, MPI_COMM_WORLD, &request);
			currentProc++;
		}
		
		int sizeOfPortion = NULL;
		int iterator = 0;
		int currentSize = NULL;
		int* A = NULL;
		A = new int[n];
		for (int i = 1; i < procNum; i++) {
			int* test = NULL;
			currentSize = sizes[i - 1];
			//MPI_Irecv(&sizeOfPortion, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
			//MPI_Wait(&request, &status);
			test = new int[currentSize];
			//MPI_Irecv(test, sizeOfPortion, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
			//MPI_Wait(&request, &status);
			MPI_Recv(test, currentSize, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			//std::cout << "Portion from process #" << i << ": ";
			//printArray(test, sizeOfPortion);
			for (int i = 0; i < currentSize; i++) {
				A[iterator] = test[i];
				iterator++;
			}
			sizeOfPortion = 0;
			test = NULL;
		}
		printf("Processed array from process #%d:\n", procRank);
		printArray(A, n);

	} else {
		
		int subArraySize;
		int* subA = NULL;
		int* subB = NULL;
		int* subC = NULL;
		int* subD = NULL;
		MPI_Irecv(&subArraySize, 1, MPI_INT, root, 0, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, &status);
		subA = new int[subArraySize];
		subB = new int[subArraySize];
		subC = new int[subArraySize];
		subD = new int[subArraySize];

		MPI_Irecv(subB, subArraySize, MPI_INT, root, 0, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, &status);
		MPI_Irecv(subC, subArraySize, MPI_INT, root, 0, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, &status);
		MPI_Irecv(subD, subArraySize, MPI_INT, root, 0, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, &status);

		printf("Recieved part from Array B: ");
		printArray(subB, subArraySize);
		printf("Recieved part from Array C: ");
		printArray(subC, subArraySize);
		printf("Recieved part from Array D: ");
		printArray(subD, subArraySize);

		for (int i = 0; i < subArraySize; i++) {
			subA[i] = subB[i] + subC[i] - (subD[i] * exp);
		}

		printf("Calculated part from Array A:");
		printArray(subA, subArraySize);
		
		//MPI_Isend(&subArraySize, 1, MPI_INT, root, 0, MPI_COMM_WORLD, &request);
		//MPI_Send(&subArraySize, 1, MPI_INT, root, 0, MPI_COMM_WORLD);
		//MPI_Isend(subA, subArraySize, MPI_INT, root, 0, MPI_COMM_WORLD, &request);
		MPI_Send(subA, subArraySize, MPI_INT, root, 0, MPI_COMM_WORLD);
		
	}
	MPI_Finalize();
	return 0;
}