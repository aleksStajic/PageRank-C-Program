#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "engine.h"

#define  BUFSIZE 256

int main(void) {
    Engine* ep = NULL;
    mxArray* Connectivity = NULL;
    mxArray* result = NULL;
    FILE* fp;

    int i = -1;
    int j = 0;
    int k = 0;
    int count = 0;
    char temp;
    int size;

    fp = fopen("web.txt", "r");

    /**
     * This while loop will return the total number of elements in the
     * web matrix
     * We can take the square root of this to find the number of rows/columns (square matrix)
     */
    while (fscanf(fp, "%c", &temp) == 1) {
        if (temp != ' ' && temp != '\n') {
            count++;
        }
    }

    /* Dynamically allocate a 2-D connectivity matrix with the correct sizing */
    size = sqrt(count);

    char* linebuffer = (char*)malloc(size * 3 * sizeof(char));

    double** ConnectivityMatrix = (double**)malloc(size * sizeof(double*)); //initialize pointers to each row

    for (i = 0; i < size; i++) {
        *(ConnectivityMatrix + i) = (double*)malloc(size * sizeof(double));
    }

    i = -1;
    fclose(fp);
    fp = fopen("web.txt", "r");

    while (fgets(linebuffer, size * 3, fp) != NULL) { //maybe figure out how to fix size later
        i++;
        j = 0;
        k = 0;
        while (linebuffer[j] != '\0') {
            ConnectivityMatrix[k][i] = linebuffer[j] - '0';
            j = j + 2;
            k++;
        }
    }

    double* temp2 = (double*)malloc(count * sizeof(double));

    j = 0;
    k = 0;
    for (int i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            temp2[k] = ConnectivityMatrix[i][j];
            k++;

        }
    }

    /* Starts a MATLAB process */
    if (!(ep = engOpen(NULL))) {
        fprintf(stderr, "\nCan't start MATLAB engine\n");
        system("pause");
        return 1;
    }

    Connectivity = mxCreateDoubleMatrix(size, size, mxREAL);
    memcpy((void*)mxGetPr(Connectivity), (void*)(temp2), count * sizeof(double));

    //Use if statements to check if there was an error in any Matlab connection sequence or variable creation
    if (engPutVariable(ep, "ConnectivityMatrix", Connectivity)) {
        fprintf(stderr, "\nCannot write test array to MATLAB \n");
        system("pause");
        exit(1); // Same as return 1;
    }

    if (engEvalString(ep, "[rows, columns] = size(ConnectivityMatrix)")) {
        fprintf(stderr, "\nError  \n");
        system("pause");
        exit(1);
    }

    if (engEvalString(ep, "dimension = size(ConnectivityMatrix,1)")) {
        fprintf(stderr, "\nError  \n");
        system("pause");
        exit(1);
    }

    if (engEvalString(ep, "columnsums = sum(ConnectivityMatrix,1)")) {
        fprintf(stderr, "\nError  \n");
        system("pause");
        exit(1);
    }

    if (engEvalString(ep, "p = 0.85")) {
        fprintf(stderr, "\nError  \n");
        system("pause");
        exit(1);
    }

    if (engEvalString(ep, "zerocolumns = find(columnsums~=0)")) {
        fprintf(stderr, "\nError  \n");
        system("pause");
        exit(1);
    }

    if (engEvalString(ep, "D = sparse(zerocolumns,zerocolumns,1./columnsums(zerocolumns),dimension,dimension)")) {
        fprintf(stderr, "\nError  \n");
        system("pause");
        exit(1);
    }

    if (engEvalString(ep, "StochasticMatrix = ConnectivityMatrix * D")) {
        fprintf(stderr, "\nError  \n");
        system("pause");
        exit(1);
    }

    if (engEvalString(ep, "[row, column] = find(columnsums==0)")) {
        fprintf(stderr, "\nError  \n");
        system("pause");
        exit(1);
    }

    if (engEvalString(ep, "StochasticMatrix(:,column) = 1./dimension")) {
        fprintf(stderr, "\nError  \n");
        system("pause");
        exit(1);
    }


    if (engEvalString(ep, "Q = ones(dimension, dimension)")) {
        fprintf(stderr, "\nError  \n");
        system("pause");
        exit(1);
    }

    if (engEvalString(ep, "TransitionMatrix = p * StochasticMatrix + (1-p) * (Q/dimension)")) {
        fprintf(stderr, "\nError  \n");
        system("pause");
        exit(1);
    }

    if (engEvalString(ep, "PageRank = ones(dimension,1)")) {
        fprintf(stderr, "\nError  \n");
        system("pause");
        exit(1);
    }
   
    if (engEvalString(ep, "for i = 1:100 PageRank = TransitionMatrix * PageRank; end")) {
        fprintf(stderr, "\nError  \n");
        system("pause");
        exit(1);
    }

    if (engEvalString(ep, "PageRank = PageRank / sum(PageRank)")) {
        fprintf(stderr, "\nError  \n");
        system("pause");
        exit(1);
    }

    //Print the result

    result = engGetVariable(ep, "PageRank");
    printf("NODE    RANK\n");
    printf("---     ----\n");
    
    if (result == NULL) {
        fprintf(stderr, "\nFailed to retrieve PageRank\n");
        system("pause");
        exit(1);
    }

    for (i = 0; i < size; i++) {
        printf("%d    %f \n" ,i+1 , *(mxGetPr(result) + i));
    }


    //destroy Matlab variables
    mxDestroyArray(Connectivity);
    mxDestroyArray(result);

    //set used pointers to NULL
    Connectivity = NULL;
    result = NULL;

    //close the Matlab engine instance
    if (engClose(ep)) {
        fprintf(stderr, "\nFailed to close MATLAB engine\n");
    }

    system("pause"); // So the terminal window remains open long enough for you to read it
    return 0; // Because main returns 0 for successful completion
}
