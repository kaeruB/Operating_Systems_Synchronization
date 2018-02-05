#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <semaphore.h>

#define N 50		//dlugosc tablicy 

int threadsNumber;
int showStartEnd = 0;		//dla 1 pokazuje jesli zaczyna/konczy czytac zapisywac
int threadsCreated = 0;
int array[N];
int iVersion;		//wersja opisowa == 1, nie == 0

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int r, w;
int qIndex = 0;
int *queue;



typedef struct guideStruct {
	int divider;
	int id;
} args;

long gettid(){
	return syscall(SYS_gettid);
}

void printErr(char* string){
	printf("Error: %s, %s\n", string, strerror(errno));
	exit(1);
}

void *writer(void *arg){
	while(threadsCreated != 1);

	args *tmp = (args*) arg;
	int writerThreadId = tmp->id;
	int numOfModifications;
	int newNumber;
	int oldNumber;
	int randPosition;
	
	
	for (int i = 0; i < threadsNumber; i++){
		if (queue[i] == -1){
			queue[i] = writerThreadId;
			break;							
		}
	}

	if (pthread_mutex_lock(&mutex) != 0)
		printErr("pthread_mutex_lock error");
									//jesli wskazanie w kolejce nie na ten watek lub
	while (r > 0 || w == 1 || queue[qIndex] != writerThreadId)	//r > 0 -> reader active lub w == 1 writer active
		if (pthread_cond_wait(&cond, &mutex) != 0)		//czekaj
			printErr("pthread_cond_wait error");
	qIndex++;
	w = 1;
	if (pthread_mutex_unlock(&mutex) != 0)				
		printErr("pthread_mutex_unlock error");

	//zapis--------------------------------------------------------------------
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
	if (iVersion == 0) 
		printf("(pisarz %ld) zmodyfikowano %d wartosci w tablicy\n", gettid(), numOfModifications);


	usleep(rand() % 5000);
	if (showStartEnd == 1)
		printf("-> pisarz %ld konczy zapis\n", gettid());

	//---------------------------------------------------------------------------


	if (pthread_mutex_lock(&mutex) != 0)
		printErr("pthread_mutex_lock error");
	w = 0;
	if (pthread_cond_broadcast(&cond) != 0)
		printErr("pthread_cond_broadcast error");
	if (pthread_mutex_unlock(&mutex) != 0)
		printErr("pthread_mutex_unlock error");

	return NULL;
}

void *reader(void *arg){
	while(threadsCreated != 1);

	args *tmp = (args*) arg;
	int div = tmp->divider;
	int readerThreadId = tmp->id;
	int foundNumCounter;
	
	for (int i = 0; i < threadsNumber; i++){
		if (queue[i] == -1){
			queue[i] = readerThreadId;
			break;							
		}
	}

	if (pthread_mutex_lock(&mutex) != 0)
		printErr("pthread_mutex_lock error");

	while (w > 0 || queue[qIndex] != readerThreadId)	//czekaj dopoki w > 0 (pisarz aktywny)		
		if (pthread_cond_wait(&cond, &mutex) != 0)
			printErr("pthread_cond_wait error");

	qIndex++;
	r++;
	if (pthread_mutex_unlock(&mutex) != 0)
		printErr("pthread_mutex_unlock error");

	//czytanie--------------------------------------------------------------
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

	//-----------------------------------------------------------------------
	if (pthread_mutex_lock(&mutex) != 0)
		printErr("pthread_mutex_lock error");
	r--;
	if (r == 0)						//jesli wszyscy czytelnicy skonczyli to powiadom pisarzy
		if (pthread_cond_broadcast(&cond) != 0)
			printErr("pthread_cond_broadcast error");
	if (pthread_mutex_unlock(&mutex) != 0)
		printErr("pthread_mutex_unlock error");
		
	
	return NULL;
}


int main(int argc, char *argv[]) {
	pthread_t *writerThreads;
	pthread_t *readerThreads;

	int writerNum, readerNum;
	int div;
	args *readersArgsTab;
	args *writersArgsTab;

	//char iV[2] = "-i";		
	
	if (argc < 4 || argc > 5 || atoi(argv[1]) < 0 || atoi(argv[2]) < 0 || atoi(argv[3]) < 0){
		printf("Nalezy podac dzielnik, liczbe pisarzy i liczbe czytelnikow.\nWersja opisowa - ostatni arg to -i\n");
		exit(1);
	}
	div = atoi(argv[1]);
	writerNum = atoi(argv[2]);
	readerNum = atoi(argv[3]);
	threadsNumber = writerNum + readerNum;

	readersArgsTab = calloc(readerNum, sizeof(args));
	if (readersArgsTab == NULL)
		printErr("calloc error");
	for (int i = 0; i < readerNum; i++)
		readersArgsTab[i].divider = div;	
		

	writersArgsTab = calloc(writerNum, sizeof(args));
	if (writersArgsTab == NULL)
		printErr("calloc error");
	

	queue = calloc(writerNum + readerNum, sizeof(int));
	if (queue == NULL)
		printErr("calloc error");
	for (int i = 0; i < writerNum + readerNum; i++)
		queue[i] = -1;


	if (argc == 5)			// && strcmp(argv[4], iV) == 0
		iVersion = 1;
	
	else
		iVersion = 0;
	
		
	srand(time(NULL));
	for (int i = 0; i < N; i++)
		array[i] = rand();

	if (pthread_mutex_init(&mutex, NULL) != 0)
		printErr("pthread_mutex_init error");
	

	writerThreads = calloc(writerNum, sizeof(pthread_t));
	readerThreads = calloc(readerNum, sizeof(pthread_t));
	if (writerThreads == NULL || readerThreads == NULL)
		printErr("calloc error");

	
	
	for (int i = 0; i < readerNum; i++){
		readersArgsTab[i].id = i;
		if (pthread_create(&readerThreads[i], NULL, reader, (void*)&readersArgsTab[i]) != 0)		
			printf("pthread_create error\n");
	}

	for (int i = 0; i < writerNum; i++){
		writersArgsTab[i].id = i + readerNum;
		if (pthread_create(&writerThreads[i], NULL, writer, (void*)&writersArgsTab[i]) != 0)
			printf("pthread_create error\n");
	}

	threadsCreated = 1;

	for (int i = 0; i < readerNum; i++)
		if (pthread_join(readerThreads[i], NULL) != 0)
			printErr("pthread_join error");	
	for (int i = 0; i < writerNum; i++)
		if (pthread_join(writerThreads[i], NULL) != 0)
			printErr("pthread_join error");	

	free(writerThreads);
	free(readerThreads);
	free(queue);
	free(readersArgsTab);
	free(writersArgsTab);
	if (pthread_mutex_destroy(&mutex) != 0)
		printErr("pthread_mutex_destroy error");
	if (pthread_cond_destroy(&cond) != 0)
		printErr("pthread_cond_destroy error");

	return 0;
}
