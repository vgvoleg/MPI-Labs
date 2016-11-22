#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <mpi.h>

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
        if (v==elem)
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
        cycle.push_back(endV);
        string s = to_string(cycle[cycle.size() - 1]);
        for (int i = cycle.size() - 2; i >= 0; i--) {
            s += "-" + to_string(cycle[i]);
        }
        if (!hasElement(catalogCycles, s)) {
            s = to_string(cycle[0]);
            for (int i = 1; i < cycle.size(); i++) {
                s += "-" + to_string(cycle[i]);
            }
            catalogCycles.push_back(s);
        }
        return;
    }
    for (int i = 0; i < size; i++) {
        if (i == unavailable) {
            continue;
        }
        if ((matrix[getIndex(current, i, size)]==1) && (visited[i] == 0)) {
            DFS(catalogCycles, i, endV, matrix, size, visited, current, cycle);
            visited[i] = 0;
        }
    }
}

int main(int argc, char** argv) {
    int *matrix;
    int sizeMtx;

    int ROOT = 0;

    MPI_Init(&argc, &argv);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == ROOT){
        readGraph("../input/smallgraph.txt", matrix, sizeMtx);
    }

    MPI_Bcast(&sizeMtx, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    if (rank != ROOT){
        matrix = new int[sizeMtx];
    }
    MPI_Bcast(matrix, sizeMtx, MPI_INT, ROOT, MPI_COMM_WORLD);



    int *visited = new int[sizeMtx];
    vector<string> cycles;

    for (int i = rank; i<sizeMtx; i+=size) {
        printf("Process %d searching for cycles from vertex %d\n", rank, i);
        resetVisited(visited, sizeMtx);
        vector<int> cycle = vector<int>();
        cycle.push_back(i);
        DFS(cycles, i, i, matrix, sizeMtx, visited, -1, cycle);
    }

//    for (string c: cycles){
//        cout<<c<<endl;
//    }
    cout <<"Cycles found in "<<rank<<" process: "<<cycles.size()<<endl;
    MPI_Finalize();
    return 0;
}