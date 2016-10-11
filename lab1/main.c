// Copyright 2016 Ovcharuk Oleg
#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"

// Функция для считывания вектора из файла
int* readVectorFromFile(char* filename, int* n){
    int k = 0, i;
    int s;
    FILE* file;
    file = fopen(filename, "r");
    if (file == NULL){
        printf("File not found! \n");
        exit(-47);
        return NULL;
    } else {
        while ((fscanf(file, "%d",&s)!=EOF))
        {    if(!file) break;
            k+=1;
        }
        int *c = (int*) malloc(k*sizeof(int));
        *n = k;
        rewind(file);
        for(i=0;i<k;i++)
        {
            fscanf(file, "%d",&c[i]);
        }
        printf("\n");
        fclose(file);

        return c;
    }
}

/* Суть задачи - нахождение наиболее близких
 * элементов вектора. Появляется небольшая
 * трудность в виде того, что для всех
 * процессов нужно найти и минимальную разницу
 * между элементами, и индекс элемента,
 * который, образуя пару со следующим
 * и определяет наиболее близкую пару.
 * На помощь приходит MPI_Reduce с параметром
 * MPI_MINLOC и MPI_2INT, специальная структура
 * для которого описана ниже
 **/
typedef struct {
    int minValue;
    int minIndex;
} minPair;


int main(int argc, char **argv) {
    int N;
    int* vector = (int *)malloc(sizeof(int)*100000000);
    int ROOT = 0;

    // Минимальная пара, которая будет найдена в каждом процессе
    minPair localPairVector;

    // Минимальная пара из всех мининимальных пар каждого процесса (че?)
    minPair globalPairVector;

    double startWtime = 0.0, endWtime = 0.0;

    MPI_Init(&argc, &argv);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Считываем вектор из файла только в одном (нулевом) процессе
    if (rank == ROOT) {
        char filename[] = "../smallinput.txt";
        vector = readVectorFromFile(filename, &N);
    }

    // Делимся этим вектором со всеми
    MPI_Bcast(&N, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(vector, N, MPI_INT, ROOT, MPI_COMM_WORLD);

    // Засекаем время
    startWtime = MPI_Wtime();

    // Находим ближайшую пару в поле видимости процесса
    int minDiff = 1000000000;
    int iMin;
    int diff;

    for (int i = rank; i < N - 1; i+=size) {
        diff = abs(vector[i] - vector[i + 1]);
        if (diff < minDiff) {
            minDiff = diff;
            iMin = i;
        }
    }
    localPairVector.minValue = minDiff;
    localPairVector.minIndex = iMin;

    // Находим ближайшую из всех, найденных каждым процессом
    MPI_Reduce(&localPairVector, &globalPairVector, 1, MPI_2INT, MPI_MINLOC, ROOT, MPI_COMM_WORLD);

    // Собственно выводим результат
    if (rank == ROOT){
        printf("Target elements -> [%d] [%d] \n",
               vector[globalPairVector.minIndex], vector[globalPairVector.minIndex+1]);

        endWtime = MPI_Wtime();
        printf("Time = %.15f\n", endWtime - startWtime);

    }

    MPI_Finalize();
    return 0;
}