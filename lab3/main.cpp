#include <iostream>
#include <fstream>
#include <vector>
#include <mpi.h>
#include <conio.h>

using namespace std;

// Reading adjacency matrix from file
void readGraph(string filename, int *&matrix, int &size) {
    ifstream file;
    file.open(filename);
    if (file.is_open()) {
        file >> size;
        matrix = new int[size * size];
        for (int i = 0; i < size * size; i++) {
            file >> matrix[i];
        }
    }
}

// Getting index for 1d array, using 2d notation
int getIndex(int i, int j, int &size) {
    return size * i + j;
}


void resetVisited(int *&visited, int &size) {
    for (int i = 0; i < size; i++) {
        visited[i] = 0;
    }
}

bool hasElement(vector<string> &vec, string elem) {
    for (string v: vec) {
        if (v == elem)
            return true;
    }
    return false;
}

void DFS(vector<string> &catalogCycles, int current, int endV, int *&matrix, int &size,
         int *&visited, int unavailable, vector<int> cycle) {
    if (current != endV) {
        visited[current] = 1;
        cycle.push_back(current);
    } else if (cycle.size() >= 2) {
        char* buf = new char[10];
        cycle.push_back(endV);
        string s = string(itoa(cycle[cycle.size() - 1], buf, 10));
        for (int i = cycle.size() - 2; i >= 0; i--) {
            s += "-" + string(itoa(cycle[i], buf, 10));
        }
        if (!hasElement(catalogCycles, s)) {
            s = string(itoa(cycle[0], buf, 10));
            for (int i = 1; i < cycle.size(); i++) {
                s += "-" + string(itoa(cycle[i], buf, 10));
            }
            catalogCycles.push_back(s);
        }
        return;
    }
    for (int i = 0; i < size; i++) {
        if (i == unavailable) {
            continue;
        }
        if ((matrix[getIndex(current, i, size)] == 1) && (visited[i] == 0)) {
            DFS(catalogCycles, i, endV, matrix, size, visited, current, cycle);
            visited[i] = 0;
        }
    }
}

void vecToChar(vector<string>& strvec, char** cstr, int& cSize){
    cSize = strvec.size();
    int size = 0;
    *cstr = new char[cSize*80];

    for (unsigned long i=0; i<cSize; i++){
        strcpy(*cstr + size , strvec[i].c_str());
        size+=80;
    }
}

int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);

    int size, rank;
    double startTime, stopTime;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int *matrix;
    int sizeMtx;
    int ROOT = 0;
    // считывание графа из файла нулевым процессом
    if (rank == ROOT) {
        readGraph("input/graph.txt", matrix, sizeMtx);
    }
    // начало записи времени
    startTime = MPI_Wtime();
    // отправка считанного графа всем остальным процессам
    MPI_Bcast(&sizeMtx, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    if (rank != ROOT) {
        matrix = new int[sizeMtx*sizeMtx];
    }
    MPI_Bcast(matrix, sizeMtx*sizeMtx, MPI_INT, ROOT, MPI_COMM_WORLD);


    int *visited = new int[sizeMtx];
    vector<string> cycles;
    // вызов процедуры поиска в глубину для вершин, доступных каждому процессу
    for (int i = rank; i < sizeMtx; i += size) {
        resetVisited(visited, sizeMtx);
        vector<int> cycle = vector<int>();
        cycle.push_back(i);
        DFS(cycles, i, i, matrix, sizeMtx, visited, -1, cycle);
    }

    int vecSize; char *foundCycles;
    vecToChar(cycles, &foundCycles, vecSize);

    int resultVecSize; char* resultVec;
    // вычисление суммы длин всех найденных массивов для выделения памяти под сбор 
    MPI_Reduce(&vecSize, &resultVecSize, 1, MPI_INT, MPI_SUM, ROOT, MPI_COMM_WORLD);

    int* mass_count_r = new int[size];
    int* mass_disp_r = new int[size];
    resultVec = new char[vecSize * 80];
    // собираем длины всех найденных массивов в один массив нулевого процесса для использования MPI_Gatherv
    MPI_Gather(&vecSize, 1, MPI_INT, mass_count_r, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    // подготовка нулевого процесса, в частности инициализация и заполнение массива смещений
    if (rank == ROOT){
        resultVec = new char[resultVecSize*80];
        mass_count_r[0]*=80;
        mass_disp_r[0]=0;
        for (int i = 1; i<size; i++){
            mass_disp_r[i]= mass_disp_r[i-1] + mass_count_r[i-1];   
            mass_count_r[i]*=80;
        }
    }
    // сбор найденных массивов в один
    MPI_Gatherv(foundCycles, vecSize*80, MPI_CHAR, resultVec, mass_count_r, mass_disp_r, MPI_CHAR, ROOT, MPI_COMM_WORLD);
    // запись в файл и вывод времени работы программы
    if (rank == ROOT){
        stopTime = MPI_Wtime();
        printf("Total cycles found = %d\n", resultVecSize);
        double time = stopTime - startTime;
        printf("Time: %lf\n", time);
        ofstream file;
        file.open("output/output.txt");
        for (int i=0; i<resultVecSize; i++){
            file<<&resultVec[i*80]<<"\n";
        }
        file.close();
        printf("List of all cycles is available on output/\n");
    }

    MPI_Finalize();
    return 0;
}