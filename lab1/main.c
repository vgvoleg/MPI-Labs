#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"

int* readVectorFromFile(char* filename, int* n){
    int k = 0, i;
    int s;
    FILE* file;
    file = fopen(filename, "r");
    if (file == NULL){
        printf("File not found! \n");
        return 0;
    } else {
        while ((fscanf(file, "%d",&s)!=EOF))
        {    if(!file) break;    //чтобы не делал лишнего
            k+=1;
        }
        int *c = (int*) malloc(k*sizeof(int));  //должен быть динамическим
        *n = k;
        rewind(file);    //перематываем файл для повторного чтения
        for(i=0;i<k;i++)
        {
            fscanf(file, "%d",&c[i]);
            printf("c[%d]=%d  ",i,c[i]);
        }
        printf("\n");
        fclose(file);

        return c;
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char filename[] = "../input.txt";
    int vectorSize;
    int *vector = readVectorFromFile(filename, &vectorSize);

    int minDiff = 1000000;
    int iMin = -1;

    int i;
    for (i = 0; i< (vectorSize-1); i++){
        int diff = abs(vector[i] - vector[i+1]);
        if (diff < minDiff){
            minDiff = diff;
            iMin = i;
        }
    }

    printf("Min: %i %i", vector[iMin], vector[iMin + 1]);
    return 0;
}