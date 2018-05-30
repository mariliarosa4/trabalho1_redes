//backoff
//threads são pacotes que tentarão acessar o meio
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

#define NUMERO_TRANSMISSORES 2
#define SLOT_TIME_ENVIO 1
#define NUM_COLISOES 0    //0 - numero de colisoes
#define POSICAO_MEIO 1    //1 - posição no meio
#define TEMPO_BACKOFF 2   //2 - tempo do backoff atual
#define POSICAO_ALVO 3    //3 - posicao alvo
#define POSICAO_MEIO_D 4  //4 - posição no meio da direita
#define POSICAO_MEIO_E 5  //5 - posição no meio da esquerda

int array[10];
int array_destino[10];
int	sinal = 1;
int emEspera[NUMERO_TRANSMISSORES];
int colisao = 0;
pthread_mutex_t lock;
pthread_mutex_t lock_meio;
int avancouTudoEsquerda[NUMERO_TRANSMISSORES];
int avancouTudoDireita[NUMERO_TRANSMISSORES];
int matriz_transmissores[NUMERO_TRANSMISSORES][6];
int slot_time = 2;

void *meio(void *j) {
	int i = *(int *)j;
	while (1) {  
	sinal = 1;
		sleep(matriz_transmissores[i][TEMPO_BACKOFF]);
		matriz_transmissores[i][TEMPO_BACKOFF] = 0;
	
		while(verificarMeio()==1){
			verificarPacotesPraMim(i);
			sleep(1);
			emEspera[i]=1;
		
		}
		emEspera[i]=0;
		escreverMeio(i);

		while(avancouTudoDireita[i] ==0 && avancouTudoEsquerda[i]==0){
			if (array[matriz_transmissores[i][POSICAO_MEIO]]==-2){
				//pthread_mutex_lock(&lock);
				printf("\n JAM recebido em %d", i);
				backoff(i);
				sleep(1);
				//pthread_mutex_unlock(&lock);
				break;
			}
		
			if (matriz_transmissores[i][POSICAO_MEIO_D]!=-1){
				andarMeioDireita(i, sinal); 
			}
			if (matriz_transmissores[i][POSICAO_MEIO_E]!=-1){
				andarMeioEsquerda(i, sinal);
			}
			
		}
		slot_time=2;
		avancouTudoDireita[i]=0;
		avancouTudoEsquerda[i]=0;
		sleep(2);
		limparMeio();
			
	}
}


void andarMeioDireita(int i){
	//direita
	
	if (matriz_transmissores[i][POSICAO_MEIO_D]<9){
	//	printf("direita");
	int j =0;
	j=matriz_transmissores[i][POSICAO_MEIO_D];
		if (array[j+1]!=0){
			emitirJam();
			sleep(2);
			
		} 
		
		array[j+1]=sinal;
		array_destino[j+1]=matriz_transmissores[i][POSICAO_ALVO];
		sleep(slot_time);
		
	matriz_transmissores[i][POSICAO_MEIO_D] = j+1;
}else{
	avancouTudoDireita[i]=1;
}
}

void andarMeioEsquerda(i){
		//esquerda
		
		if (matriz_transmissores[i][POSICAO_MEIO_E]>0){
		//printf("esquerda");
		int b=0;
		b=matriz_transmissores[i][POSICAO_MEIO_E];
		if (array[b-1]!=0){
			emitirJam();
			sleep(2);
			
		} 
	
		array[b-1]=sinal;
		array_destino[b-1]=matriz_transmissores[i][POSICAO_ALVO];
		sleep(slot_time);
		matriz_transmissores[i][POSICAO_MEIO_E] = b-1;	
		}else{
			avancouTudoEsquerda[i]=1;
		}
	}
void backoff(int i){
	printf("\n Backoff para %d", i);

	matriz_transmissores[i][NUM_COLISOES]=matriz_transmissores[i][NUM_COLISOES]+1;
	int tempo_backoff = pow(2,matriz_transmissores[i][NUM_COLISOES]) * SLOT_TIME_ENVIO;
	int aleatorio = rand() % tempo_backoff;
	matriz_transmissores[i][TEMPO_BACKOFF] = aleatorio;	
}
void *interface(){
	while(1){
			//pthread_mutex_lock(&lock);
		sleep(1);
	
		system("@cls||clear");
		mostrarMeio();
		listaDeEspera();
		//pthread_mutex_unlock(&lock);
	}
}
void listaDeEspera(){
	int k;
	int nenhum=1;
	printf("\n Transmissores em espera: ");
	for(k=0;k<NUMERO_TRANSMISSORES;k++){
		if (emEspera[k]==1){
			printf(" %d - ", k);
			nenhum=0;
		}
	}
	if (nenhum==1){
			printf("\n Não há ninguém escutando o meio \n");
	}

}
int escreverMeio(int i){
	printf("\n ---> %d", i);
	array[matriz_transmissores[i][POSICAO_MEIO]]=1;
	array_destino[matriz_transmissores[i][POSICAO_MEIO]]= matriz_transmissores[i][POSICAO_ALVO];
	if (matriz_transmissores[i][POSICAO_MEIO]==0){
		matriz_transmissores[i][POSICAO_MEIO_D]=matriz_transmissores[i][POSICAO_MEIO]+1;
		array[matriz_transmissores[i][POSICAO_MEIO_D]]=1;
		array_destino[matriz_transmissores[i][POSICAO_MEIO_D]]=matriz_transmissores[i][POSICAO_ALVO];
		matriz_transmissores[i][POSICAO_MEIO_E] = -1;
		sleep(2);
	}else{
		
	
	if (matriz_transmissores[i][POSICAO_MEIO]==9){
		matriz_transmissores[i][POSICAO_MEIO_E]=matriz_transmissores[i][POSICAO_MEIO]-1;
		array[matriz_transmissores[i][POSICAO_MEIO_E]]=1;
		array_destino[matriz_transmissores[i][POSICAO_MEIO_E]]=matriz_transmissores[i][POSICAO_ALVO];

		matriz_transmissores[i][POSICAO_MEIO_D] = -1;
		sleep(2);
	}else{
		matriz_transmissores[i][POSICAO_MEIO_E]=matriz_transmissores[i][POSICAO_MEIO]-1;
		matriz_transmissores[i][POSICAO_MEIO_D]=matriz_transmissores[i][POSICAO_MEIO]+1;
		array[matriz_transmissores[i][POSICAO_MEIO_E]]=1;
		array[matriz_transmissores[i][POSICAO_MEIO_D]]=1;
		
		array_destino[matriz_transmissores[i][POSICAO_MEIO_D]]=matriz_transmissores[i][POSICAO_ALVO];
		array_destino[matriz_transmissores[i][POSICAO_MEIO_E]]=matriz_transmissores[i][POSICAO_ALVO];

		sleep(2);
	}
}
	
}
void emitirJam(){
	printf("colisao!!");
	sinal = -2;
	slot_time = 1;
}
void limparMeio(){
	
	int i;
		for ( i=0;i<10;i++){
		array[i]=0;
	}
}
void mostrarMeio(){
	
	int i;
	printf("\n [ ");
	for (i=0;i<10;i++){
		printf(" %d", array[i]);	
	}
	printf(" ]  \n");
	
	printf("\n [ ");
	for (i=0;i<10;i++){
		printf(" %d", array_destino[i]);	
	}
	printf(" ]  \n");
	sleep(1);
}

int verificarMeio(){
	int res = 0 ;
	int i;
	int k;
	for (i=0;i<10;i++){

		if (array[i]==0){
			res = 0;
		}else{
			return 1;
		}
	
	}
	return res;
}

void verificarPacotesPraMim(int i){
	if (array_destino[matriz_transmissores[i][POSICAO_MEIO]]==matriz_transmissores[i][POSICAO_MEIO]){
		if (array[matriz_transmissores[i][POSICAO_MEIO]]==-2){
			printf("Recebi um pacote jam");
			sleep(2);
		}else{
			printf("Recebi um pacote ");
			sleep(2);
		}	
	}
}

main(){
matriz_transmissores[0][NUM_COLISOES]=0;
matriz_transmissores[0][POSICAO_MEIO]=0;
matriz_transmissores[0][TEMPO_BACKOFF]=0;
matriz_transmissores[0][POSICAO_ALVO]=9;
matriz_transmissores[1][NUM_COLISOES]=0;
matriz_transmissores[1][POSICAO_MEIO]=9;
matriz_transmissores[1][TEMPO_BACKOFF]=0;
matriz_transmissores[1][POSICAO_ALVO]=0;
avancouTudoDireita[0]=0;
avancouTudoDireita[1]=0;

	pthread_mutex_init(&lock, NULL);
	pthread_mutex_init(&lock_meio, NULL);
	
	
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
