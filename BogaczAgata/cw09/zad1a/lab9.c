#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <semaphore.h>

//semafory + faworyzowanie czytelnikow

#define N 50
//#define readerTurns 1
//#define writerTurns 2
int showStartEnd = 0;

int threadsCreated = 0;
int array[N];
int iVersion;		//wersja opisowa == 1, nie == 0

sem_t readers;
sem_t writers;
int activeReaders = 0;


typedef struct guideStruct {
	int divider;
} args;

long gettid(){
	return syscall(SYS_gettid);
}

void printErr(char* string){
	printf("Error: %s, %s\n", string, strerror(errno));
	exit(1);
}

void *writer(void *taisetsuJiaNai){
	while(threadsCreated != 1);

	int numOfModifications;
	int newNumber;
	int oldNumber;
	int randPosition;
	
	//usleep(rand() % 500);

	//for (int i = 0; i < writerTurns; i++){
		if (sem_wait(&writers) != 0)
			printErr("sem_wait error");

		//zapis--------------------------------------------
		//usleep(rand() % 5000);
		if (showStartEnd == 1)
			printf("-> pisarz %ld zaczyna zapis\n", gettid());
		numOfModifications = rand() % N;	
		for (int i = 0; i < numOfModifications; i++){
			randPosition = rand() % N;
			newNumber = rand();
			oldNumber = array[randPosition];
			array[randPosition] = newNumber;
			if (iVersion == 1)
				printf("(pisarz %ld) array[%d] == %d zamieniono na == %d\n", gettid(), randPosition, oldNumber,  newNumber);
		}	

		usleep(rand() % 5000);
		if (iVersion == 0) 
			printf("(pisarz %ld) zmodyfikowano %d wartosci w tablicy\n", gettid(), numOfModifications);
		
		//--------------------------------------------------
		if (showStartEnd == 1)
			printf("-> pisarz %ld konczy zapis\n", gettid());

		if (sem_post(&writers) != 0)
			printErr("sem_post error");


	//}
	return NULL;
}

void *reader(void *arg){
	while(threadsCreated != 1);

	args *tmp = (args*) arg;
	int div = tmp->divider;
	int foundNumCounter;

	//usleep(rand() % 500);
	
	//for (int i = 0; i < readerTurns; i++){
		if (sem_wait(&readers) != 0)
			printErr("sem_wait error");
		activeReaders++;

		if (activeReaders == 1)
			sem_wait(&writers);
		if (sem_post(&readers)  != 0)
			printErr("sem_post error");

		//czytanie------
		//usleep(rand() % 5000);
		if (showStartEnd == 1)
			printf("-> czytelnik %ld zaczyna czytac\n", gettid());

		foundNumCounter = 0;
		for (int i = 0; i < N; i++){
			if (array[i] % div == 0){
				foundNumCounter++;
				if (iVersion == 1)
					printf("(czytelnik %ld) Liczba %d pod indeksem %d podzielna przez %d\n",gettid(), array[i], i, div);
			}
		}		
		if (iVersion == 0)
			printf("(czytelnik %ld) Znaleziono %d liczb podzielnych przez %d\n", gettid(), foundNumCounter, div);


		usleep(rand() % 5000);
		if (showStartEnd == 1)
			printf("-> czytelnik %ld konczy czytac\n", gettid());

		//---------------

		if (sem_wait(&readers) != 0)
			printErr("sem_wait error");
		activeReaders--;
		if (activeReaders == 0)
			sem_post(&writers);
		if (sem_post(&readers)  != 0)
			printErr("sem_post error");

	//}
	return NULL;
}


int main(int argc, char *argv[]) {
	pthread_t *writerThreads;
	pthread_t *readerThreads;

	int writerNum, readerNum;
	args readerArg;
	//char iV[2] = "-i";
	
	if (argc < 4 || argc > 5 || atoi(argv[1]) < 0 || atoi(argv[2]) < 0 || atoi(argv[3]) < 0){
		printf("Nalezy podac dzielnik, liczbe pisarzy i liczbe czytelnikow.\nWersja opisowa - ostatni arg to -i\n");
		exit(1);
	}
	readerArg.divider = atoi(argv[1]);
	writerNum = atoi(argv[2]);
	readerNum = atoi(argv[3]);

	if (argc == 5)			// && strcmp(argv[4], iV) == 0
		iVersion = 1;
	
	else
		iVersion = 0;
	
		
	srand(time(NULL));
	for (int i = 0; i < N; i++)
		array[i] = rand();
	
	sem_init(&readers, 0, 1);
	sem_init(&writers, 0, 1);
	

	writerThreads = calloc(writerNum, sizeof(pthread_t));
	readerThreads = calloc(readerNum, sizeof(pthread_t));
	if (writerThreads == NULL || readerThreads == NULL)
		printErr("calloc error");
	
	
	for (int i = 0; i < writerNum; i++)
		if (pthread_create(&writerThreads[i], NULL, writer, NULL) != 0)
			printf("pthread_create error\n");

	for (int i = 0; i < readerNum; i++)
		if (pthread_create(&readerThreads[i], NULL, reader, (void*)&readerArg) != 0)
			printf("pthread_create error\n");
	/*

	for (int i = 0; i < readerNum; i++)
		if (pthread_create(&readerThreads[i], NULL, reader, (void*)&readerArg) != 0)
			printf("pthread_create error\n");

	for (int i = 0; i < writerNum; i++)
		if (pthread_create(&writerThreads[i], NULL, writer, NULL) != 0)
			printf("pthread_create error\n");
	
	*/
	threadsCreated = 1;

	for (int i = 0; i < readerNum; i++)
		if (pthread_join(readerThreads[i], NULL) != 0)
			printErr("pthread_join error");	
	for (int i = 0; i < writerNum; i++)
		if (pthread_join(writerThreads[i], NULL) != 0)
			printErr("pthread_join error");	

	free(writerThreads);
	free(readerThreads);
	sem_destroy(&readers);
	sem_destroy(&writers);

	return 0;
}
