#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int acc = 0;
pthread_mutex_t mtx;

void *TaskCode(void *argument) {
	int tid;
	tid = *((int*) argument);
	int partial_acc = 0;
	
	for (int i = 0; i < 1000000; i++) {
		acc = acc + 1;
	}

	/* data race 해결 */
	pthread_mutex_lock(&mtx);
	acc += partial_acc;
	pthread_mutex_unlock(&mtx);

	return NULL;
}

int main(int argc, char *argv[]) {
	pthread_t threads[4];
	int args[4];
	int i;

	pthread_mutex_init(&mtx, NULL);

	/* create all threads */
	for (i = 0; i < 4; i++) {
		args[i] = i;
		pthread_create(&threads[i], NULL, TaskCode, (void *)&args[i]);
	}

	/* wait for all threads to complete */
	for (i = 0; i < 4; i++)
		pthread_join(threads[i], NULL);

	printf("%d\n", acc);

	return 0;
}

