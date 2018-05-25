//backoff
//threads são pacotes que tentarão acessar o meio
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

#define NUMERO_TRANSMISSORES 2
#define SLOT_TIME 1
int array[10];
pthread_mutex_t lock;
int matriz_transmissores[NUMERO_TRANSMISSORES][3];
#define NUM_COLISOES 0//0 - numero de colisoes
#define POSICAO_MEIO 1//1 - posição no meio
//2 - posicao alvo


void *meio(void *j) {
	int i = *(int *)j;
	while (1) {  /* repete eternamente */

		while(verificarMeio()==1){
			sleep(4);
			printf("to esperando %d", i);
		}
			pthread_mutex_lock(&lock);
			escreverMeio(i);
			pthread_mutex_unlock(&lock);
			andarMeio(i);
	}
}
void andarMeio(int i){
	if (i==0){
		int j;
		for (j=matriz_transmissores[i][POSICAO_MEIO];j<9;j++){
			if (array[j+1]!=0){
				emitirJam();
				sleep(1);
				break;
			}
			int pos = array[j];
			array[j+1]=pos;
			array[j]=0;
			sleep(1);
		}
			limparMeio();
	}else{
		int j;
		for (j=matriz_transmissores[i][POSICAO_MEIO];j>0;j--){
			if (array[j-1]!=0){
				emitirJam();
				sleep(1);
				break;
			}
			int	pos = array[j];
			array[j-1]=pos;
			array[j]=0;
			
			sleep(1);
			
		}
			limparMeio();
	}
}
void backoff(int i){
	printf("Backoff para %d", i);
	matriz_transmissores[i][NUM_COLISOES]=matriz_transmissores[i][NUM_COLISOES]+1;
	int tempo_backoff = pow(2,matriz_transmissores[i][NUM_COLISOES]) * SLOT_TIME;
	sleep(tempo_backoff);
	
}
void *interface(){
	while(1){
		sleep(1);
		 system("@cls||clear");
		mostrarMeio();
		
	}
}

int escreverMeio(int i){
	printf(" %d", i);
	if (i==0){
		array[0]=1;
	}else{
		array[9]=1;		
	}

}
void emitirJam(){
	printf("colisao!!!!!");
	limparMeio();
	backoff(0);
	backoff(1);
}
void limparMeio(){
	int i;
		for ( i=0;i<10;i++){
		array[i]=0;
	}
	sleep(1);
}
void mostrarMeio(){
	printf("\n [ ");
	int i;
	for (i=0;i<10;i++){
		printf(" %d", array[i]);
	}
	printf(" ]  \n");
}

int verificarMeio(){
	int res = 0 ;
	int i;
	for (i=0;i<10;i++){
		if (array[i]==NULL){
			res = 0;
		}else{
			return 1;
		}
	}
	return res;
}

main(){
matriz_transmissores[0][NUM_COLISOES]=0;
matriz_transmissores[0][POSICAO_MEIO]=0;
matriz_transmissores[1][NUM_COLISOES]=0;
matriz_transmissores[1][POSICAO_MEIO]=9;

	pthread_mutex_init(&lock, NULL);
	
	void *thread_result;
	pthread_t pacote[NUMERO_TRANSMISSORES];
	pthread_t threadInterface;
	void *threadInterface_result;
	int i=0;
	pthread_create(&threadInterface, NULL, interface, NULL);
	
	for(i=0;i<NUMERO_TRANSMISSORES;i++){
		pthread_create(&pacote[i],NULL, meio, &i);
	}	

	for(i=0;i<NUMERO_TRANSMISSORES;i++){
		pthread_join(pacote[i],&thread_result);
	
	}	
	 pthread_join(threadInterface, &threadInterface_result);
}
