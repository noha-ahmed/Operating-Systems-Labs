#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#define MAX_LIMIT 100

int mat1_row;
int mat1_col;
int mat2_row;
int mat2_col;
int mat1[MAX_LIMIT][MAX_LIMIT];
int mat2[MAX_LIMIT][MAX_LIMIT];
int matOut[MAX_LIMIT][MAX_LIMIT];
char buff[MAX_LIMIT];


//A Function to read matrix from file
void readFile( const char* fileName, int matNum )
{
    int *row, *col, *mat;
    if( matNum == 1 )
    {
        row = &mat1_row;
        col = &mat1_col;
        mat = mat1;
    }
    else
    {
        row = &mat2_row;
        col = &mat2_col;
        mat = mat2;
    }


    FILE *fptr = fopen( fileName, "r" );
    //checking if the file exist
    if (fptr == NULL)
    {
        printf("Error while opening the file.\n");
        exit(1);
    }

    //read and validate first line
    readFirstLine(fptr,row,col);
    //read and validate matrix
    readMat(fptr,mat, *row, *col );

    fclose(fptr);

}

//A function to read and validate First Line

void readFirstLine( FILE *fptr, int* row, int *col )
{
    //reading first Line ( dimensions line )
    fscanf(fptr, "%[^\n]", buff);
    printf("Data from the file: %s\n", buff);
    if(  sscanf(buff, "row=%d col=%d", row,col) != 2 )
    {
        printf("Invalid first line format");
        exit(1);
    }

}

//A function to validate and read Matrix from file
void readMat(FILE *fptr, int mat[MAX_LIMIT][MAX_LIMIT], int row, int col )
{
    for(int i =0 ; i < row ; i++)
        for(int j = 0 ; j < col ; j++ )
            if(fscanf(fptr, "%d", &mat[i][j]) != 1)
            {
                printf("Invalid Matrix input\n");
                exit(-1);
            }
}

//A function to save output matrix in text file
void saveMatOut( char* fileName ){
   FILE *fptr;
   fptr = fopen(fileName, "w");
   fprintf(fptr,"row=%d col=%d\n",mat1_row,mat2_col);
   for( int i = 0 ; i < mat1_row ; i++ ){
      for( int j = 0 ; j < mat2_col ; j++ ){
        fprintf(fptr,"%d ",matOut[i][j]);
      }
      fprintf(fptr,"\n");
   }

}

//A function to validate dimensions ( make sure if mat1_col = mat2_row )
void validateDimensions()
{
    if( mat1_col != mat2_row )
    {
        printf("Dimensions of matrices is not valid\n");
        exit(3);
    }
}

//A function to print Matrix
void printMat(int mat[MAX_LIMIT][MAX_LIMIT], int row, int col )
{
    for(int i =0 ; i < row ; i++)
    {
        for(int j = 0 ; j < col ; j++ )
            printf("%d ",mat[i][j]);
        printf("\n");
    }
}




/////////////////////////////////////////////////////////
//Methods of multiplication

//Method 1 ( 1 thread per Matrix )
void matrixMult_1()
{
    for( int i = 0 ; i < mat1_row ; i++ )
    {
        for( int j = 0 ; j < mat2_col ; j++ )
        {
            for( int k = 0 ; k < mat1_col ; k++ )
            {
                matOut[i][j] += mat1[i][k]*mat2[k][j];
            }
        }
    }
}

//Method 2 ( 1 thread per row )

// procedure of 1 thread ( one row multiplied by columns )
void *matrixMult_2_thread( void* rowNum )
{
    int i = (int) rowNum;
    for( int j = 0 ; j < mat2_col ; j++ )
    {
        matOut[i][j] = 0;
        for( int k = 0 ; k < mat1_col ; k++ )
        {
            matOut[i][j] += mat1[i][k]*mat2[k][j];
        }
    }
    pthread_exit(NULL);
}

//The main method that create threads ( thread per row )
void matrixMult_2()
{
    int rc;
    pthread_t threads[mat1_row];
    for( int i = 0 ; i < mat1_row ; i++ )
    {
        //creating thread per row and passing index of row to the thread
        rc = pthread_create(&threads[i],NULL,matrixMult_2_thread,(void*)i);
        if (rc)
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }

    }
    for(int t=0; t < mat1_row ; t++)
    {
        pthread_join(threads[t], NULL);
    }
}





//Method 3 ( 1 thread per element )

// Data to be passed to a thread
struct thread_data
{
    int rowNum;
    int colNum;
};


// procedure of 1 thread ( one row multiplied by one column )
void *matrixMult_3_thread( void* threadarg )
{
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    int i = my_data->rowNum;
    int j = my_data->colNum;
    matOut[i][j] = 0;
    for( int k = 0 ; k < mat1_col ; k++ )
    {
        matOut[i][j] += mat1[i][k]*mat2[k][j];
    }
    pthread_exit(NULL);

}

//The main method that create threads ( thread per element
void matrixMult_3()
{
    int rc;
    pthread_t threads[mat1_row*mat2_col];
    int t= 0 ;
    for( int i = 0 ; i < mat1_row ; i++ )
    {
        for( int j = 0 ; j < mat2_col ; j++ )
        {
            struct thread_data *tdata = malloc(sizeof(struct thread_data));
            tdata->rowNum = i;
            tdata->colNum = j;
            //creating thread per element and passing index of row and col to the thread
            rc = pthread_create(&threads[t++],NULL,matrixMult_3_thread,(void*)tdata);
            if (rc)
            {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
        }
    }

    for(int t=0; t < mat1_row*mat2_col ; t++)
    {
        pthread_join(threads[t], NULL);
    }
}

//////////////////////////////////////////////////////////////////////////
//Methods to run the three methods and get number of threads and execution time
void runmethod_1(){
    struct timeval stop, start;

    gettimeofday(&start, NULL); //start checking time

    //executing method
    matrixMult_1();
    printf("output matrix is: \n");
    printMat(matOut,mat1_row,mat2_col);

    gettimeofday(&stop, NULL); //end checking time

    printf(">> Method 1 ( A Thread per Matrix ) \n");
    printf("-------------------------------------\n");
    printf("Number of Threads used: 1\n");
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    printf("\n\n");

}

void runmethod_2(){
    struct timeval stop, start;

    gettimeofday(&start, NULL); //start checking time

    //executing method
    matrixMult_2();
    printf("output matrix is: \n");
    printMat(matOut,mat1_row,mat2_col);

    gettimeofday(&stop, NULL); //end checking time

    //printing info
    printf(">> Method 2 ( A Thread per Row ) \n");
    printf("-------------------------------------\n");
    printf("Number of Threads used: %d\n",mat1_row);
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    printf("\n\n");

}

void runmethod_3(){
    struct timeval stop, start;

    gettimeofday(&start, NULL); //start checking time

    //executing method
    matrixMult_3();
    printf("output matrix is: \n");
    printMat(matOut,mat1_row,mat2_col);

    gettimeofday(&stop, NULL); //end checking time


    //printing info
    printf(">> Method 3 ( A Thread per Element ) \n");
    printf("-------------------------------------\n");
    printf("Number of Threads used: %d\n",mat1_row*mat2_col);
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    printf("\n\n");

}



int main(  int argc, char *argv[] )
{
    //Taking the file names of the matrices and setting their default value if not entered
    char* mat1_fileName;
    char* mat2_fileName;
    char* matOut_fileName;
    printf("%d\n",argc);
    if( argc == 1 ){
        mat1_fileName = "a.txt";
        mat2_fileName = "b.txt";
        matOut_fileName = "c.txt";
    }
    else if( argc == 3 ){
        mat1_fileName =  strcat(argv[1],".txt");
        mat2_fileName = strcat(argv[2],".txt");
        matOut_fileName = "c.txt";
    }
    else if ( argc == 4 ){
        mat1_fileName = strcat(argv[1],".txt");
        mat2_fileName = strcat(argv[2],".txt");
        matOut_fileName = strcat(argv[3],".txt");
    }
    else{
        printf("Invalid Arguments");
        exit(-1);
    }


    //Reading matrix 1 from file
    readFile(mat1_fileName, 1);
    printf("%d,%d \n",mat1_row,mat1_col);
    printMat(mat1,mat1_row,mat1_col);
    //Reading matrix 2 from file
    readFile(mat2_fileName, 2);
    printf("%d,%d \n",mat2_row,mat2_col);
    printMat(mat2,mat2_row,mat2_col);
    validateDimensions();

    //running the three methods and printing number of threads and execution time for its execution
    runmethod_1();
    runmethod_2();
    runmethod_3();

    //Printing output matrix and saving it to text file
    printf(" << Output Matrix >> \n");
    printMat(matOut,mat1_row,mat2_col);
    saveMatOut(matOut_fileName);





    return 0;
}



///////////////////////////////////////////////////////////////////////

//Trash functions

int validateFirstLine2( char* line, int* row, int *col)
{
    char* dimsLine[MAX_LIMIT];
    //split "row=x col=y" by space
    dimsLine[0] = strtok(line," ");
    int i = 0;
    while( dimsLine[i] != NULL )
    {
        dimsLine[++i] = strtok(NULL," ");
    }
    //check if "row=x" and "col=y" exists
    if( i != 2  )
    {
        printf("ERROR in split 1 \n");
        return -1;
    }


    //validate "row=" part
    if( strlen(dimsLine[0]) <= 4 || dimsLine[0][3] != '=' )
    {
        printf("ERROR in row=x format \n");
        return -1;
    }

    //validate "col=" part
    if( strlen(dimsLine[1]) <= 4 || dimsLine[1][3] != '=' )
    {
        printf("ERROR in col=y format \n");
        return -1;
    }

    //split "row=x" by "="
    char* temp[MAX_LIMIT];
    temp[0] = strtok(dimsLine[0],"=");
    i = 0;
    while( temp[i] != NULL )
    {
        temp[++i] = strtok(NULL,"=");
    }
    if( i != 2 || strcmp(temp[0],"row") )
    {
        printf("ERROR in split 1-1 \n");
        return -1;
    }

    //validating x

    if( ( *row = parseToInteger(temp[1]) ) == -1 )
    {
        printf("ERROR in parsing x \n");
        return -1;
    }



    //split "col=y" by "="
    temp[0] = strtok(dimsLine[1],"=");
    i = 0;
    while( temp[i] != NULL )
    {
        temp[++i] = strtok(NULL,"=");
    }
    if( i != 2 || strcmp(temp[0],"col") )
    {
        printf("ERROR in split 1-2 \n");
        return -1;
    }

    //validating y
    if( ( *col = parseToInteger(temp[1]) ) == -1 )
    {
        printf("ERROR in parsing y \n");
        return -1;
    }

    return 0;

}





//returns numerical value of a string and -1 if it is not valid string
int parseToInteger( char* strNum )
{
    for( int i = 0 ; i < strlen(strNum) ; i++ )
    {
        if( !isdigit(strNum[i]) ) return -1;
    }
    return atoi(strNum);

}
///////////////////////////////////////////////////////////////////////


