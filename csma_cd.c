//backoff
//threads são pacotes que tentarão acessar o meio
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>


#define NUMERO_TRANSMISSORES 2
#define SLOT_TIME_ENVIO 1
#define NUM_COLISOES 0    //0 - numero de colisoes
#define POSICAO_MEIO 1    //1 - posição no meio
#define TEMPO_BACKOFF 2   //2 - tempo do backoff atual
#define POSICAO_ALVO 3    //3 - posicao alvo
#define POSICAO_MEIO_D 4  //4 - posição no meio da direita
#define POSICAO_MEIO_E 5  //5 - posição no meio da esquerda
sem_t mutex;

int tempo_backoff[NUMERO_TRANSMISSORES];
int aleatorio[NUMERO_TRANSMISSORES];
int enviarJam[NUMERO_TRANSMISSORES];
int array[10];
int array_destino[10];
int arrayEmTransmissao[NUMERO_TRANSMISSORES];
int	sinal[NUMERO_TRANSMISSORES];
int emEspera[NUMERO_TRANSMISSORES];
int colisao = 0;
pthread_mutex_t lock;
pthread_mutex_t lock_meio;
pthread_mutex_t lock_rand;
int avancouTudoEsquerda[NUMERO_TRANSMISSORES];
int avancouTudoDireita[NUMERO_TRANSMISSORES];
int matriz_transmissores[NUMERO_TRANSMISSORES][6];
int slot_time = 2;

void *sensing(void *j) {
    int i = *(int *)j;
    while(1) {
    	
        verificarPacotesPraMim(i); //caso quem nao esteja transmitindo receba pacotes de jam
        // ou pacote de dados e ver se é pra ele mesmo
        if (verificarMeio()==1) {
            emEspera[i]=1;
            sleep(2);
        }
      
    }
}
void *meio(void *j) {
    int i = *(int *)j;
    while (1) {
  		if (matriz_transmissores[i][TEMPO_BACKOFF]!=0){
        		printf("vou dormir\n \n \n \n ======");
        		sleep(1);
        		sleep(matriz_transmissores[i][TEMPO_BACKOFF]);
        		matriz_transmissores[i][TEMPO_BACKOFF]=0;
			}
        if (verificarMeio()==0 && matriz_transmissores[i][TEMPO_BACKOFF]==0) {
        	
            
            matriz_transmissores[i][TEMPO_BACKOFF] = 0;


            emEspera[i]=0;
           // pthread_mutex_lock(&lock);
            escreverMeio(i);
           // pthread_mutex_unlock(&lock);
            avancarNoMeio(i);
            
            if (enviarJam[i]==1){
            	tacaJam(i);
			}
            avancouTudoDireita[i]=0;
          	avancouTudoEsquerda[i]=0;
            //pthread_mutex_lock(&lock_acontecendo);
            while (verificarSeAcabou()==0) {
                sleep(1);
                printf("verificando se acabou");
            }
            limparMeio();
            limparMeioDestino();
            slot_time=2;
            arrayEmTransmissao[i]=0;
            sinal[i] = 1;
            sleep(2);


//	}
        }
    }
}
void avancarNoMeio(int i) {
    while(avancouTudoDireita[i] ==0 && avancouTudoEsquerda[i]==0) {


        if (matriz_transmissores[i][POSICAO_MEIO_D]!=-1) {
            if (matriz_transmissores[i][POSICAO_MEIO_D]<9) {
                andarMeioDireita(i);
            } else {
                avancouTudoDireita[i]=1;
            }
        }
        if (matriz_transmissores[i][POSICAO_MEIO_E]!=-1) {
            if (matriz_transmissores[i][POSICAO_MEIO_E]>0) {
                andarMeioEsquerda(i);
            } else {
                avancouTudoEsquerda[i]=1;
                 
            }
        }

    }
    if (sinal[i]==1){
                arrayEmTransmissao[i]=0;	
				}else{
					arrayEmTransmissao[i]=1;
					avancouTudoEsquerda[i]=0;
					avancouTudoDireita[i]=0;
					printf("ainda nao terminei");
				}
}
int verificarSeAcabou() {
    int k;
    int retorno=0;
    for (k=0; k<NUMERO_TRANSMISSORES; k++) {
        if (arrayEmTransmissao[k]==0) {
            retorno =1;
        } else {
            return 0;
        }
    }
    return retorno;
}
void tacaJam(i) {
    sinal[i] = -3;

    sleep(1);
    //limparMeio();
    limparMeioDestino();
    //sleep(5);
    avancouTudoDireita[i]=0;
    avancouTudoEsquerda[i]=0;
//	sleep(1);
    //pthread_mutex_lock(&lock);
    escreverMeio(i);
   // pthread_mutex_unlock(&lock);
    
   
    sleep(3);
    avancarNoMeio(i);
    arrayEmTransmissao[i]=0;

}
void andarMeioDireita(int i) {
    int j =0;
    j=matriz_transmissores[i][POSICAO_MEIO_D];
   // printf("%d", j);
  //  sem_wait(&mutex);
    if (array[j+1]==1) {
        emitirJam(i);
        sleep(1);

    }
 pthread_mutex_lock(&lock);
    array[j+1]=sinal[i];
     pthread_mutex_unlock(&lock);
    array_destino[j+1]=matriz_transmissores[i][POSICAO_ALVO];
    //sem_post(&mutex);
    sleep(slot_time);

    matriz_transmissores[i][POSICAO_MEIO_D] = j+1;

}

void andarMeioEsquerda(i) {
    int b=0;
    b=matriz_transmissores[i][POSICAO_MEIO_E];
    if (array[b-1]==1) {
        emitirJam(i);
        sleep(1);

    }
    pthread_mutex_lock(&lock);
    array[b-1]=sinal[i];
    pthread_mutex_unlock(&lock);
    array_destino[b-1]=matriz_transmissores[i][POSICAO_ALVO];
    sleep(slot_time);
    matriz_transmissores[i][POSICAO_MEIO_E] = b-1;

}
void backoff(i) {

    matriz_transmissores[i][NUM_COLISOES]=matriz_transmissores[i][NUM_COLISOES]+1;
    tempo_backoff[i] = pow(2,matriz_transmissores[i][NUM_COLISOES]) * SLOT_TIME_ENVIO;
    pthread_mutex_lock(&lock_rand);
    srand(time(NULL));
	sleep(1);
    aleatorio[i] = rand() % (tempo_backoff[i]);
    pthread_mutex_unlock(&lock_rand);
    sleep(1);
    matriz_transmissores[i][TEMPO_BACKOFF] = aleatorio[i];

    printf("\n Backoff para %d de tempo %d \n", i, matriz_transmissores[i][TEMPO_BACKOFF]);
   sleep(1);
    // sem_wait(&mutex);


    // sem_post(&mutex);

}
void *interface() {
    while(1) {
        pthread_mutex_lock(&lock_meio);

        system("@cls||clear");
        emTransmissao();
        mostrarMeio();
        listaDeEspera();
        pthread_mutex_unlock(&lock_meio);
        sleep(1);
    }
}
void listaDeEspera() {
    int k;
    int nenhum=1;
    printf("\n Transmissores em espera: ");
    for(k=0; k<NUMERO_TRANSMISSORES; k++) {
        if (emEspera[k]==1) {
            printf(" %d - ", k);
            nenhum=0;
        }
    }
    if (nenhum==1) {
        printf("\n Não há ninguém escutando o meio \n");
    }

}

void emTransmissao() {
    int i;
    printf("\n Em transmissao: ");
    for(i=0; i<NUMERO_TRANSMISSORES; i++) {
        if (arrayEmTransmissao[i]==1) {
            printf("\n ---> %d", i);
        }

    }
    printf(" \n");
    sleep(1);
}
int escreverMeio(int i) {
   pthread_mutex_lock(&lock);
    arrayEmTransmissao[i]=1;
    
    array[matriz_transmissores[i][POSICAO_MEIO]]=sinal[i];
    
    array_destino[matriz_transmissores[i][POSICAO_MEIO]]= matriz_transmissores[i][POSICAO_ALVO];
    if (matriz_transmissores[i][POSICAO_MEIO]==0) {
        matriz_transmissores[i][POSICAO_MEIO_D]=matriz_transmissores[i][POSICAO_MEIO]+1;
        array[matriz_transmissores[i][POSICAO_MEIO_D]]=sinal[i];
        array_destino[matriz_transmissores[i][POSICAO_MEIO_D]]=matriz_transmissores[i][POSICAO_ALVO];
        matriz_transmissores[i][POSICAO_MEIO_E] = -1;
        	sleep(2);
    } else {


        if (matriz_transmissores[i][POSICAO_MEIO]==9) {
            matriz_transmissores[i][POSICAO_MEIO_E]=matriz_transmissores[i][POSICAO_MEIO]-1;
            array[matriz_transmissores[i][POSICAO_MEIO_E]]=sinal[i];
            array_destino[matriz_transmissores[i][POSICAO_MEIO_E]]=matriz_transmissores[i][POSICAO_ALVO];

            matriz_transmissores[i][POSICAO_MEIO_D] = -1;
            	sleep(2);
        } else {
            matriz_transmissores[i][POSICAO_MEIO_E]=matriz_transmissores[i][POSICAO_MEIO]-1;
            matriz_transmissores[i][POSICAO_MEIO_D]=matriz_transmissores[i][POSICAO_MEIO]+1;
            array[matriz_transmissores[i][POSICAO_MEIO_E]]=sinal[i];
            array[matriz_transmissores[i][POSICAO_MEIO_D]]=sinal[i];
            array_destino[matriz_transmissores[i][POSICAO_MEIO_D]]=matriz_transmissores[i][POSICAO_ALVO];
            array_destino[matriz_transmissores[i][POSICAO_MEIO_E]]=matriz_transmissores[i][POSICAO_ALVO];

            	sleep(2);
        }
    }
    pthread_mutex_unlock(&lock);

}
void emitirJam(i) {
    printf("colisao!!");
    sinal[i] = -2;
//	slot_time = 1;
}
void limparMeio() {

    int i;
    pthread_mutex_lock(&lock);
    for ( i=0; i<10; i++) {
        array[i]=0;
    }
    pthread_mutex_unlock(&lock);
}

void imparMeioDestino() {

    int i;
    for ( i=0; i<10; i++) {
        array_destino[i]=-9;
    }
}
void mostrarMeio() {

    int i;
    printf("\n [ ");
    for (i=0; i<10; i++) {
        printf(" %d", array[i]);
    }
    printf(" ]  \n");

    printf("\n [ ");
    for (i=0; i<10; i++) {
        printf(" %d", array_destino[i]);
    }
    printf(" ]  \n");
    sleep(1);
}

int verificarMeio() {
//	printf("verificando");
    int res = 0 ;
    int i;
    int k;
    for (i=2; i<8; i++) {

        if (array[i]==0) {
            res = 0;
        } else {
            return 1;
        }

    }
    return res;
}

void verificarPacotesPraMim(i) {
//printf("verificando pacotes pra mim************* \n");
    if (array_destino[matriz_transmissores[i][POSICAO_MEIO]]==matriz_transmissores[i][POSICAO_MEIO]) {
        if (array[matriz_transmissores[i][POSICAO_MEIO]]==-2 ) {
            printf("Chegou colisao \n");
          //  tacaJam(i);
          enviarJam[i]=1;
            sleep(2);
        } else if (array[matriz_transmissores[i][POSICAO_MEIO]]==1) {
            printf("Eu, %d Recebi um pacote \n", i);
            sleep(2);
        }
    }
    if (array[matriz_transmissores[i][POSICAO_MEIO]]==-3 && matriz_transmissores[i][TEMPO_BACKOFF]==0) {
        printf("Eu, %d Recebi um pacote jam \n", i);
        //sleep(2);
      //  pthread_mutex_lock(&lock);
      //  array[matriz_transmissores[i][POSICAO_MEIO]]=0;
      //  pthread_mutex_lock(&lock);
        backoff(i);
        //sleep(1);
    }
}
void preencherArrayDestino() {
    int i;
    for ( i=0; i<10; i++) {
        array_destino[i]=-9;
    }
}
void limparMeioDestino() {


    int i;
    for ( i=0; i<10; i++) {
        array_destino[i]=-9;
    }

}
main() {
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
    avancouTudoEsquerda[0]=0;
    avancouTudoEsquerda[1]=0;
    enviarJam[0]=0;
    enviarJam[1]=0;
    sinal[0]=1;
    sinal[1]=1;

    preencherArrayDestino();
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&lock_meio, NULL);
    pthread_mutex_init(&lock_rand, NULL);
    sem_init(&mutex, 0, 1);

    void *thread_result;
    pthread_t transmissor[NUMERO_TRANSMISSORES];
    pthread_t threadInterface;
    pthread_t threadSensing[NUMERO_TRANSMISSORES];
    void *threadInterface_result;
    void *threadSensing_result;
    int i=0;
    pthread_create(&threadInterface, NULL, interface, NULL);


    for(i=0; i<NUMERO_TRANSMISSORES; i++) {
        pthread_create(&transmissor[i], NULL, meio, &i);
        pthread_create(&threadSensing[i],NULL, sensing, &i);
    }

    for(i=0; i<NUMERO_TRANSMISSORES; i++) {
        pthread_join(transmissor[i],&thread_result);
        pthread_join(threadSensing[i], &threadSensing_result);


    }
    pthread_join(threadInterface, &threadInterface_result);
}
