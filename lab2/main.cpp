#include <iostream>
#include <fstream>
#include "mpi.h"
#define ARR_SIZE 50000

using namespace std;

// Init array with random values and
// write it into the "input.txt" file
void initArray(int *array, int N) {
    srand(uint(MPI_Wtime()));
    ofstream file;

    file.open("../input.txt");
    for (int i = 0; i<N; i++){
        array[i] = rand() % 15000;
        file << array[i] << " ";
    }

    file.close();
}
// Saves array into "output.txt" file
void saveArray(int *array, int N) {
    ofstream file;
    file.open("../output.txt");
    for (int i = 0; i<N; i++){
        file << array[i] << " ";
    }
    file.close();
}

void mergeArrays (int *array1, int size1, int *array2, int size2, int *result) {
    int i = 0, j = 0, k = 0;
    while (i < size1 && j < size2)
        if (array1[i] < array2[j])
            result[k++] = array1[i++];
        else
            result[k++] = array2[j++];
    if (i == size1)
        while (j < size2)
            result[k++] = array2[j++];
    if(j == size2)
        while (i < size1)
            result[k++] = array1[i++];
}

void swap (int* array, int i, int j){
    int t;
    t = array[i];
    array[i] = array[j];
    array[j] = t;
}

void sort (int* array, int n){
    for (int i = n - 2; i >= 0; i--)
        for (int j = 0; j <= i; j++)
            if (array[j] > array[j + 1])
                swap (array, j, j + 1);
}


int main(int argc, char ** argv){
    int *array; int N;
    int *resultArray;

    int s, r;
    int move;
    int *sub;
    int m;

    double startTime, stopTime;
    int size, rank;
    int ROOT = 0;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == ROOT) {
        N = ARR_SIZE;
        s = N / size;
        r = N % size;
        array = new int[N + s - r];
        initArray(array, N);
        if (r != 0) {
            for (int i = N; i < N + s - r; i++) {
                array[i] = 0;
            }
            s++;
        }
    }

    startTime = MPI_Wtime();
    MPI_Bcast(&s, 1, MPI_INT, 0, MPI_COMM_WORLD);
    resultArray = new int[s];
    MPI_Scatter(array, s, MPI_INT, resultArray, s, MPI_INT, ROOT, MPI_COMM_WORLD);
    sort(resultArray, s);

    move = 1;
    while (move<size){
        if (rank % (2*move) == 0){
            if (rank+move < size){
                MPI_Recv(&m, 1, MPI_INT, rank + move, ROOT, MPI_COMM_WORLD, &status);
                sub = new int[m];
                MPI_Recv(sub, m, MPI_INT, rank + move, ROOT, MPI_COMM_WORLD, &status);
                int *tmp = new int[s+m];
                mergeArrays(resultArray, s, sub, m, tmp);
                resultArray = tmp;
                s += m;
            }
        } else {
            int near = rank - move;
            MPI_Send (&s, 1, MPI_INT, near, ROOT, MPI_COMM_WORLD);
            MPI_Send (resultArray, s, MPI_INT, near, ROOT, MPI_COMM_WORLD);
            break;
        }
        move *= 2;
    }

    if (rank == ROOT){
        stopTime = MPI_Wtime();
        double t1 = stopTime- startTime;
        printf("Array size: %d\n", ARR_SIZE);
        startTime = MPI_Wtime();
        sort(array, ARR_SIZE);
        stopTime = MPI_Wtime();
        double t2 = stopTime - startTime;
        printf("Time on 1: %lf\n", t2);
        printf("Time on %d: %lf\n", size, t1);
        printf("PROFIT???: %f\n", t2/t1);
        saveArray(resultArray, ARR_SIZE);
    }

    MPI_Finalize();
    return 0;
}