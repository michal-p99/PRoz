#ifndef GLOBALH
#define GLOBALH

#define _GNU_SOURCE
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "kolejka.h"
/* odkomentować, jeżeli się chce DEBUGI */
//#define DEBUG 
/* boolean */
#define TRUE 1
#define FALSE 0

#define WINNER_TIME 5

#define SLIPKI 3
#define MISKA 3
#define PINEZKI 3
#define SALE 1

/* używane w wątku głównym, determinuje jak często i na jak długo zmieniają się stany */
#define STATE_CHANGE_PROB 50
#define SEC_IN_STATE 2
#define DEBATE_TIME 10

#define ROOT 0

/* stany procesu */
typedef enum {REST,  PAIRING,START_SALA,CZEKAJ_SALA,START_ZASOB, START_DEBATE,WAIT_READY, InFinish} state_t;
extern state_t stan;
extern int rank;
extern int size;
extern int lamport;
extern pthread_mutex_t stateMut;
extern pthread_mutex_t lamportMut;
extern int countQueueZasobSize;
extern int countQueueSalaSize;
extern int ackSPriority;
extern int ackZPriority;
extern int przeciwnik;
extern process_q_t queue;
extern process_q_t queue_zasob;
extern int ackCountZasob;
extern process_q_t queue_sala;
extern int ackCountSala;
extern int pickedZasob;
extern int rezerwujacy;
extern int enemyPickedZasob;
extern int enemyReady;
/* to może przeniesiemy do global... */
typedef struct {
    int ts;       /* timestamp (zegar lamporta */
    int src;      /* pole nie przesyłane, ale ustawiane w main_loop */

    int data;     /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
	int enemy;
} packet_t;
extern MPI_Datatype MPI_PAKIET_T;

/* Typy wiadomości */
#define REQ_SALA 1
#define ACK_SALA 2
#define REQ_PINEZKI 3
#define ACK_PINEZKI 4
#define REQ_MISKA 5
#define ACK_MISKA 6
#define REQ_SLIPKI 7
#define ACK_SLIPKI 8
#define REQ_I 9
#define ACK_I 10
#define PAIR 11
#define READY 12
#define JEST_SALA 13
#define RESULT 14;

/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta 
   
   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.

   w printfie: definicja znaku specjalnego "%c[%d;%dm [%d]" escape[styl bold/normal;kolor [RANK]
                                           FORMAT:argumenty doklejone z wywołania debug poprzez __VA_ARGS__
					   "%c[%d;%dm"       wyczyszczenie atrybutów    27,0,37
                                            UWAGA:
                                                27 == kod ascii escape. 
                                                Pierwsze %c[%d;%dm ( np 27[1;10m ) definiuje styl i kolor literek
                                                Drugie   %c[%d;%dm czyli 27[0;37m przywraca domyślne kolory i brak pogrubienia (bolda)
                                                ...  w definicji makra oznacza, że ma zmienną liczbę parametrów
                                            
*/
int incLamport();
int incBiggerLamport(int);
extern int lamport;

#ifdef DEBUG
//#define debug(FORMAT,...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);
#define debug(FORMAT,...) printf("%c[%d;%dm [tid %d]:[ts %d] " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, lamport, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

#define P_WHITE printf("%c[%d;%dm",27,1,37);
#define P_BLACK printf("%c[%d;%dm",27,1,30);
#define P_RED printf("%c[%d;%dm",27,1,31);
#define P_GREEN printf("%c[%d;%dm",27,1,33);
#define P_BLUE printf("%c[%d;%dm",27,1,34);
#define P_MAGENTA printf("%c[%d;%dm",27,1,35);
#define P_CYAN printf("%c[%d;%d;%dm",27,1,36);
#define P_SET(X) printf("%c[%d;%dm",27,1,31+(6+X)%7);
#define P_CLR printf("%c[%d;%dm",27,0,37);

/* printf ale z kolorkami i automatycznym wyświetlaniem RANK. Patrz debug wyżej po szczegóły, jak działa ustawianie kolorków */
#define println(FORMAT, ...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);
void sendPacketR(packet_t* pkt, int destination, int tag);
void changeState( state_t ,const char* name);
#endif
