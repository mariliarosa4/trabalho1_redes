//backoff
//threads s�o pacotes que tentar�o acessar o meio
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>


#define NUMERO_TRANSMISSORES 2
#define SLOT_TIME_ENVIO 1
#define NUM_COLISOES 0    //0 - numero de colisoes
#define POSICAO_MEIO 1    //1 - posi��o no meio
#define TEMPO_BACKOFF 2   //2 - tempo do backoff atual
#define POSICAO_ALVO 3    //3 - posicao alvo
#define POSICAO_MEIO_D 4  //4 - posi��o no meio da direita
#define POSICAO_MEIO_E 5  //5 - posi��o no meio da esquerda
#define TAMANHO_MEIO 16

int tempo_backoff[NUMERO_TRANSMISSORES];
int aleatorio[NUMERO_TRANSMISSORES];
int enviarJam[NUMERO_TRANSMISSORES];
int arrayEmTransmissao[NUMERO_TRANSMISSORES];
int	sinal[NUMERO_TRANSMISSORES];
int emEspera[NUMERO_TRANSMISSORES];
int avancouTudoEsquerda[NUMERO_TRANSMISSORES];
int avancouTudoDireita[NUMERO_TRANSMISSORES];
int matriz_transmissores[NUMERO_TRANSMISSORES][6];

int array[TAMANHO_MEIO];
int array_destino[TAMANHO_MEIO];
int colisao = 0;
pthread_mutex_t lock;
pthread_mutex_t lock_meio;
pthread_mutex_t lock_rand;

int slot_time = 1;

void *thread_result;
void *threadInterface_result;
void *threadSensing_result;
pthread_t transmissor[NUMERO_TRANSMISSORES];
pthread_t threadInterface;
pthread_t threadSensing[NUMERO_TRANSMISSORES];


void *sensing(void *j) {
    int i = *(int *)j;
    while(1) {

        verificarPacotesPraMim(i); //caso quem nao esteja transmitindo receba pacotes de jam
        // ou pacote de dados e ver se � pra ele mesmo
        if (verificarMeio()==1) {
             emEspera[i]=1;
            sleep(1);
        }
        if (matriz_transmissores[i][TEMPO_BACKOFF]>0){
		sleep(1);
		matriz_transmissores[i][TEMPO_BACKOFF]=matriz_transmissores[i][TEMPO_BACKOFF]-1;
		}

    }
}
void *meio(void *j) {
    int i = *(int *)j;
    while (1) {
       /* if (matriz_transmissores[i][TEMPO_BACKOFF]!=0) {
            sleep(matriz_transmissores[i][TEMPO_BACKOFF]);
            matriz_transmissores[i][TEMPO_BACKOFF]=0;
            if (matriz_transmissores[i][NUM_COLISOES]==16) {

                pthread_create(&transmissor[i], NULL, meio, &i);

            }
        } */
        if (verificarMeio()==0 && matriz_transmissores[i][TEMPO_BACKOFF]==0) {
            matriz_transmissores[i][TEMPO_BACKOFF] = 0;
            emEspera[i]=0;
            escreverMeio(i);
            avancarNoMeio(i);
            if (enviarJam[i]==1) {
                tacaJam(i);
            }
            avancouTudoDireita[i]=0;
            avancouTudoEsquerda[i]=0;
            while (verificarSeAcabou()==0) {
                sleep(1);
            }
            limparMeio();
            limparMeioDestino();
            slot_time=1;
            arrayEmTransmissao[i]=0;
            sinal[i] = 1;
            sleep(3);
        } else {
            emEspera[i]=1;
        }
    }
}
void avancarNoMeio(int i) {
    while(avancouTudoDireita[i] ==0 && avancouTudoEsquerda[i]==0) {


        if (matriz_transmissores[i][POSICAO_MEIO_D]!=-1) {
            if (matriz_transmissores[i][POSICAO_MEIO_D]<TAMANHO_MEIO-1) {
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
    if (sinal[i]==1) {
        arrayEmTransmissao[i]=0;
        matriz_transmissores[i][NUM_COLISOES]=0;
        matriz_transmissores[i][POSICAO_ALVO]= randPosicaoAlvo(i);
        //	printf("vamos mudar meu alvo para %d",matriz_transmissores[i][POSICAO_ALVO] );
        //sleep(10);

    } else {
        arrayEmTransmissao[i]=1;
        avancouTudoEsquerda[i]=0;
        avancouTudoDireita[i]=0;
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
     backoff(i);
    limparMeioDestino();
    avancouTudoDireita[i]=0;
    avancouTudoEsquerda[i]=0;
    escreverMeio(i);
    sleep(3);
    avancarNoMeio(i);
    arrayEmTransmissao[i]=0;

}
void andarMeioDireita(int i) {
    int j =0;
    j=matriz_transmissores[i][POSICAO_MEIO_D];
    if (array[j+1]==1) {
        emitirJam(i);
        sleep(1);
    }
    pthread_mutex_lock(&lock);
    array[j+1]=sinal[i];
    pthread_mutex_unlock(&lock);
    array_destino[j+1]=matriz_transmissores[i][POSICAO_ALVO];
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
    pthread_mutex_lock(&lock_rand);
    matriz_transmissores[i][NUM_COLISOES]=matriz_transmissores[i][NUM_COLISOES]+1;
    tempo_backoff[i] = pow(2,matriz_transmissores[i][NUM_COLISOES]) * SLOT_TIME_ENVIO;

    srand(time(NULL));
    sleep(1);
    aleatorio[i] = rand() % (tempo_backoff[i]);

    sleep(1);
    matriz_transmissores[i][TEMPO_BACKOFF] = aleatorio[i];

    printf("\n Backoff para %d de tempo %d \n", i, matriz_transmissores[i][TEMPO_BACKOFF]);
    sleep(1);
    pthread_mutex_unlock(&lock_rand);

}
void *interface() {
    while(1) {
        pthread_mutex_lock(&lock_meio);

        system("@cls||clear");
       // emTransmissao();
        mostrarMeio();
       // listaDeEspera();
       // mostrarEmBackoff();
        pthread_mutex_unlock(&lock_meio);
        sleep(1);
    }
}
void listaDeEspera() {
    int k;

    printf("\n Transmissores em espera: ");
    for(k=0; k<NUMERO_TRANSMISSORES; k++) {
        if (emEspera[k]==1) {
            printf(" %d - ", k);
        }
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


        if (matriz_transmissores[i][POSICAO_MEIO]==TAMANHO_MEIO-1) {
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
//    printf(" \n colisao!");
    sinal[i] = -2;
//	slot_time = 1;
}
void limparMeio() {

    int i;
    pthread_mutex_lock(&lock);
    for ( i=0; i<TAMANHO_MEIO; i++) {
        array[i]=0;
    }
    pthread_mutex_unlock(&lock);
}

void limparMeioDestino() {

    int i;
    for ( i=0; i<TAMANHO_MEIO; i++) {
        array_destino[i]=-9;
    }
}
void mostrarMeio() {

    int i,k;
    int temBackoff=0;
     printf("\n [ ");
       for (i=0; i<TAMANHO_MEIO; i++) {
        temBackoff =0;
        for (k=0; k<NUMERO_TRANSMISSORES; k++) {

            if (matriz_transmissores[k][POSICAO_MEIO]==i) {
            	
      
        		 printf("[%d]", matriz_transmissores[k][TEMPO_BACKOFF]);
			
                
                temBackoff =1;
            }
            }
        
        if (temBackoff==0) {
            printf("[ ]");
        }
}
    
	  
	
	int tem;
	 printf(" ]\n");
//	printf("\n");
	 printf("\n [ ");
    for (i=0; i<TAMANHO_MEIO; i++) {
        tem =0;
        for (k=0; k<NUMERO_TRANSMISSORES; k++) {

            if (matriz_transmissores[k][POSICAO_MEIO]==i) {
                printf("[%d]", k);
                tem =1;
            }
        }
        if (tem==0) {
            printf("[ ]");
        }

    }
   printf(" ]\n");

    printf("\n [ ");
    for (i=0; i<TAMANHO_MEIO; i++) {
        printf(" %d ", array[i]);
    }
    printf(" ]\n");

    printf("\n [ ");
    for (i=0; i<TAMANHO_MEIO; i++) {
        printf(" %d", array_destino[i]);
    }
    printf(" ]\n"); 
    sleep(1);
}

int verificarMeio() {

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


    if (array[matriz_transmissores[i][POSICAO_MEIO]]==-2 ) {
        enviarJam[i]=1;
        sleep(2);
    } else if (array[matriz_transmissores[i][POSICAO_MEIO]]==1) {
        if (array_destino[matriz_transmissores[i][POSICAO_MEIO]]==matriz_transmissores[i][POSICAO_MEIO]) {

            //printf("\n Eu, %d Recebi um pacote \n", i);
            sleep(3);
        }
    }

}
void preencherArrayDestino() {
    int i;
    for ( i=0; i<TAMANHO_MEIO; i++) {
        array_destino[i]=-9;
    }
}

void mostrarEmBackoff() {
    int j;
    printf("\n \n Em backoff: ");
    for ( j=0; j<NUMERO_TRANSMISSORES; j++) {
        if(matriz_transmissores[j][TEMPO_BACKOFF] !=0) {
            printf(" %d -", j);
        }
    }
}
void iniciarDados() {
    int h;
    for(h=0; h<NUMERO_TRANSMISSORES; h++) {
        avancouTudoDireita[h]=0;
        avancouTudoEsquerda[h]=0;
        enviarJam[h]=0;
        sinal[h]=1;
    }
}

void iniciarTransmissores() {
    int h;
    for (h=0; h<NUMERO_TRANSMISSORES; h++) {
        matriz_transmissores[h][NUM_COLISOES]=0;
        matriz_transmissores[h][POSICAO_MEIO]=h*(TAMANHO_MEIO/NUMERO_TRANSMISSORES);
        matriz_transmissores[h][TEMPO_BACKOFF]=0;
    }
    	if (NUMERO_TRANSMISSORES==2){
    		matriz_transmissores[0][POSICAO_ALVO]= matriz_transmissores[1][POSICAO_MEIO];
    		matriz_transmissores[1][POSICAO_ALVO]= matriz_transmissores[0][POSICAO_MEIO];
		}else{
			for (h=0; h<NUMERO_TRANSMISSORES; h++) {
    
       		matriz_transmissores[h][POSICAO_ALVO]= randPosicaoAlvo(h);
        	 }
		}
    
}
int randPosicaoAlvo(i) {
    int transmissorAleatorio;
    srand(time(NULL));
    do {
    	
        transmissorAleatorio = rand()% (NUMERO_TRANSMISSORES-1);
    } while(transmissorAleatorio==i);
    return matriz_transmissores[transmissorAleatorio][POSICAO_MEIO];
}
main() {
    iniciarTransmissores();


    iniciarDados();
    preencherArrayDestino();
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&lock_meio, NULL);
    pthread_mutex_init(&lock_rand, NULL);
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
