#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

int** create2DArray(int rows, int columns);
int* createArray(int size);
void free2DArray(int** arr, int rows);
int* readNumbersFromFile(FILE* fptr, int size);
void readNumbersInto2DArray(int** arrToReadInto, int rows, int columns, int matrixSelector, int* arrToReadFrom);
void readNumbersIntoArray(int* arrToReadInto, int size, int startingPoint, int* arrToReadFrom);
int requestAcceptable(int* requestVector, int* availableVector, int size);
void requestInfoPrint(int processNo, int* requestVector, int size);
void modifyValuesWithInput(int** allocationMatrix, int** requestMatrix, int* availableVector, int columns, int* requestVector, int processNo);
int countNumberOfDigits(int number);
void printStateInfo(int** allocationMatrix, int** requestMatrix, int* availableVector, int rows, int columns);
void printArray(int* arr, int size);
void bankerAlgorithm(int** allocationMatrix, int** requestMatrix, int* availableVector, int rows, int columns, int* requestVector, int processNo);

int main()
{
	FILE* file;
	char fileLine[50];
	int numberOfProcesses;
	int numberOfResources;
	int processNo;
	int matrixSelector = 0;
	int i;

	file = fopen("state.txt", "r");
	if (file == NULL) {
		printf("File couldn't be opened");
		exit(1);
	}

	fscanf(file, "%s%d", fileLine, &numberOfProcesses);
	fscanf(file, "%s%d", fileLine, &numberOfResources);

	int* matrixNumbers = readNumbersFromFile(file, 2 * numberOfResources * numberOfProcesses + numberOfResources);
	
	int** requestMatrix = create2DArray(numberOfProcesses, numberOfResources);
	readNumbersInto2DArray(requestMatrix, numberOfProcesses, numberOfResources, matrixSelector, matrixNumbers);
	
	matrixSelector++; // Increment to read second matrix.

	int** allocationMatrix = create2DArray(numberOfProcesses, numberOfResources);
	readNumbersInto2DArray(allocationMatrix, numberOfProcesses, numberOfResources, matrixSelector, matrixNumbers);

	matrixSelector++;

	int* availableVector = createArray(numberOfResources);
	int startingPoint = matrixSelector * numberOfProcesses * numberOfResources;
	readNumbersIntoArray(availableVector, numberOfResources, startingPoint, matrixNumbers);

	printf("Enter process no: "); // Gets which resources are requested by which process.
	scanf("%d", &processNo);
	int* requestVector = createArray(numberOfResources);
	printf("Enter the request for %d resources: ", numberOfResources);
	for (i = 0; i < numberOfResources; i++)
		scanf("%d", &requestVector[i]);
	
	bankerAlgorithm(allocationMatrix, requestMatrix, availableVector, numberOfProcesses, numberOfResources, requestVector, processNo);
	free2DArray(allocationMatrix, numberOfProcesses);
	free2DArray(requestMatrix, numberOfProcesses);
	free(availableVector);
	free(requestVector);
	free(matrixNumbers);
}

int** create2DArray(int rows, int columns)
{
	int i;
	int** arr = (int**)malloc(rows * sizeof(int*));
	for (i = 0; i < rows; i++) {
		arr[i] = (int*)malloc(columns * sizeof(int));
	}

	return arr;
}

int* createArray(int size)
{
	int *arr = malloc(size * sizeof(int));
	return arr;
}

void free2DArray(int** arr, int rows)
{
	int i;
	for (i = 0; i < rows; i++)
		free(arr[i]);
	free(arr);
}

int* readNumbersFromFile(FILE* fptr, int size)
{
	int* numArray = createArray(size);
	int i = 0;
	while ((fscanf(fptr, "%d", &numArray[i])) == 1) {
		i++;
	}
	return numArray;
}

void readNumbersInto2DArray(int** arrToReadInto, int rows, int columns, int matrixSelector, int* arrToReadFrom)
{
	int numbers = matrixSelector * rows * columns; // Sets the index to the appropriate position.
	int i, j;
	for (i = 0; i < rows; i++) {
		for (j = 0; j < columns; j++) {
			arrToReadInto[i][j] = arrToReadFrom[numbers];
			numbers++;
		}
	}
}

void readNumbersIntoArray(int* arrToReadInto, int size, int startingPoint, int* arrToReadFrom)
{
	int i;
	for (i = 0; i < size; i++) {
		arrToReadInto[i] = arrToReadFrom[startingPoint];
		startingPoint++;
	}
}

int requestAcceptable(int* requestVector, int* availableVector, int size)
{
	int i;
	for (i = 0; i < size; i++) { // Tests whether the request is acceptable or not.
		if (requestVector[i] > availableVector[i])
			return 0;
	}
	return 1;
}

void requestInfoPrint(int processNo, int* requestVector, int size)
{
	printf("--------------------------------------------------------\n"); // Prints the info about the process and the resources it requested.
	printf("If P%d gets ", processNo);
	int i;
	for (i = 0; i < size; i++) {
		if (requestVector[i] > 0)
			printf("%dxR%d ", requestVector[i], i + 1);
	}
	printf("then new state:\n");
}

void modifyValuesWithInput(int** allocationMatrix, int** requestMatrix, int* availableVector, int columns, int* requestVector, int processNo)
{
	int j;
	for (j = 0; j < columns; j++) // Modify matrices with appropriate values.
		allocationMatrix[processNo - 1][j] += requestVector[j];
	
	for (j = 0; j < columns; j++)
		requestMatrix[processNo - 1][j] -= requestVector[j];

	for (j = 0; j < columns; j++)
		availableVector[j] -= requestVector[j];
}

int countNumberOfDigits(int number)
{
	int counter = 0;
	while (number > 0) {
		number /= 10;
		counter++;
	}
	return counter;
}

void printStateInfo(int** allocationMatrix, int** requestMatrix, int* availableVector, int rows, int columns)
{
	int i, j;
	/* 2 * columns - 1 is for the integers and the spaces between them. + 8 is for the spaces between matrices
	* - 10 is to subtract the length of "Allocation" from that. Then we add 7 to this in order to add the space
	* required by the "Request" string. The other one works the same way. */
	int requestSpaceFinder = (2 * columns - 1 + 8) - 10; 
	int requestSpace = requestSpaceFinder + 7;
	int availableSpaceFinder = (2 * columns - 1 + 8) - 7;
	int availableSpace = availableSpaceFinder + 9;
	
	int digitsOfRequestSpace = countNumberOfDigits(requestSpace); // We count the number of digits because we will need this to determine the length of the format string.
	int digitsOfAvailableSpace = countNumberOfDigits(availableSpace);
	
	char* requestSpaceString = malloc(digitsOfRequestSpace + 1); // We create the strings that will serve as formatters.
	char* availableSpaceString = malloc(digitsOfAvailableSpace + 1);
	sprintf(requestSpaceString, "%d", requestSpace);
	sprintf(availableSpaceString, "%d", availableSpace);
	
	char* format = malloc(strlen("Allocation") + strlen("Request") + strlen("Available") + digitsOfRequestSpace + digitsOfAvailableSpace + 1);
	strcpy(format, "Allocation%"); // Concatanating the string to finally get the requested string.
	strcat(format, requestSpaceString);
	strcat(format, "s%");
	strcat(format, availableSpaceString);
	strcat(format, "s\n");
	printf(format, "Request", "Available");

	free(requestSpaceString);
	free(availableSpaceString);
	free(format);
	
	for (i = 0; i < rows; i++) {
		for (j = 0; j < columns; j++) {
			printf("%d ", allocationMatrix[i][j]);
		}

		for (j = 0; j < columns; j++) {
			if (j == 0)
				printf("%8d ", requestMatrix[i][j]);
			else
				printf("%d ", requestMatrix[i][j]);
		}
		
		if (i == 0) {
			for (j = 0; j < columns; j++) {
				if (j == 0)
					printf("%8d ", availableVector[j]);
				else
					printf("%d ", availableVector[j]);
			}
		}

		printf("\n");
	}
	printf("--------------------------------------------------------\n");
}

void printArray(int* arr, int size)
{
	int i;
	for (i = 0; i < size; i++)
		printf("%d ", arr[i]);
}

void bankerAlgorithm(int** allocationMatrix, int** requestMatrix, int* availableVector, int rows, int columns, int* requestVector, int processNo)
{
	if (!requestAcceptable(requestVector, availableVector, columns)) {
		printf("\nERROR: The system does not have enough resources to comply with this request!\n");
		return;
	}

	requestInfoPrint(processNo, requestVector, columns);
	modifyValuesWithInput(allocationMatrix, requestMatrix, availableVector, columns, requestVector, processNo);
	printStateInfo(allocationMatrix, requestMatrix, availableVector, rows, columns);

	int i, j, k;
	int cannotRun, processCompleted;
	int* completedProcesses = createArray(rows);
	for (i = 0; i < rows; i++)
		completedProcesses[i] = 0;

	printf("Trying to find a sequence of processes!\n");
	for (i = 0; i < rows; i++) {
		processCompleted = 0;
		
		for (j = 0; j < rows; j++) {
			
			if (completedProcesses[j] == 1)
				continue; // If this process is complete, go to the next one.

			cannotRun = 0;
			
			for (k = 0; k < columns; k++) { // If there aren't enough resources available, this process cannot run.
				if (requestMatrix[j][k] > availableVector[k]) {
					cannotRun = 1;
					break;
				}
			}
			if (cannotRun) {
				printf("* P%d cannot run\n", j + 1);
				continue; // If this process cannot run, go to the next one.
			}

			printf("* P%d runs into completion > New available = ", j + 1);
			completedProcesses[j] = 1;
			processCompleted = 1;

			for (k = 0; k < columns; k++) {
				availableVector[k] += allocationMatrix[j][k];
			}
			printArray(availableVector, columns);
			printf("\n--------------------------------------------------------\n");
			break; // After completing a process, start again from process one.
		}
		if (processCompleted == 0) { // If no process has been completed after looping through all processes, the system is unsafe.
			free(completedProcesses);
			printf("--------------------------------------------------------\n");
			printf("> System is UNSAFE");
			printf("\n--------------------------------------------------------\n\n");
			printf("RESULT: OS must DENY the request!\n");
			return;
		}
	}

	free(completedProcesses);
	printf("! All processes could run into completion !");
	printf("\n> System is SAFE");
	printf("\n--------------------------------------------------------\n\n");
	printf("RESULT: OS can safely GRANT the request!\n");
}



