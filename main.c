#include "main.h"
#include "watek_komunikacyjny.h"
#include "watek_glowny.h"
/* wątki */
#include <pthread.h>

/* sem_init sem_destroy sem_post sem_wait */
//#include <semaphore.h>
/* flagi dla open */
//#include <fcntl.h>

state_t stan=REST;
volatile char end = FALSE;
int size,rank; /* nie trzeba zerować, bo zmienna globalna statyczna */
MPI_Datatype MPI_PAKIET_T;
pthread_t threadKom;
pthread_mutex_t lamportMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;
int lamport, ackCountSala, ackSPriority, przeciwnik, pickedZasob, ackZPriority,rezerwujacy,enemyPickedZasob,ackCountZasob,enemyReady, countQueueSalaSize, countQueueZasobSize,resultReady,wynik,ackCountI;
process_q_t queue,queue_zasob,queue_sala;
void check_thread_support(int provided)
{
    printf("THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", provided);
    switch (provided) {
        case MPI_THREAD_SINGLE: 
            printf("Brak wsparcia dla wątków, kończę\n");
            /* Nie ma co, trzeba wychodzić */
	    fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
	    MPI_Finalize();
	    exit(-1);
	    break;
        case MPI_THREAD_FUNNELED: 
            printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
	    break;
        case MPI_THREAD_SERIALIZED: 
            /* Potrzebne zamki wokół wywołań biblioteki MPI */
            printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
	    break;
        case MPI_THREAD_MULTIPLE: printf("Pełne wsparcie dla wątków\n"); /* tego chcemy. Wszystkie inne powodują problemy */
	    break;
        default: printf("Nikt nic nie wie\n");
    }
}

int incLamport(){
	pthread_mutex_lock( &lamportMut );
	lamport++;
	int tmp = lamport;
	pthread_mutex_unlock(&lamportMut);
	return tmp;
}

int incBiggerLamport(int n){
	pthread_mutex_lock( &lamportMut );

	if (lamport > n)
		lamport++;
	else
		lamport = n + 1;

	int tmp = lamport;
	pthread_mutex_unlock(&lamportMut);
	return tmp;
}


/* srprawdza, czy są wątki, tworzy typ MPI_PAKIET_T
*/
void inicjuj(int *argc, char ***argv)
{
    int provided;
    MPI_Init_thread(argc, argv,MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);


    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    const int nitems=4; /* bo packet_t ma trzy pola */
    int       blocklengths[4] = {1,1,1,1};
    MPI_Datatype typy[4] = {MPI_INT, MPI_INT, MPI_INT,MPI_INT};

    MPI_Aint     offsets[4]; 
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, data);
	offsets[3] = offsetof(packet_t, enemy);

    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
    MPI_Type_commit(&MPI_PAKIET_T);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(rank);

	initQueue(&queue, size);
	initQueue(&queue_sala, size);
	initQueue(&queue_zasob, size);
    pthread_create( &threadKom, NULL, startKomWatek , 0);
    
    debug("Zainicjalizowany");
}

/* usunięcie zamkków, czeka, aż zakończy się drugi wątek, zwalnia przydzielony typ MPI_PAKIET_T
   wywoływane w funkcji main przed końcem
*/
void finalizuj()
{
    pthread_mutex_destroy( &stateMut);
	pthread_mutex_destroy(&lamportMut);
    /* Czekamy, aż wątek potomny się zakończy */
    println("czekam na wątek \"komunikacyjny\"\n" );
    pthread_join(threadKom,NULL);



    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
}


/* opis patrz main.h */
void sendPacket(packet_t *pkt, int destination, int tag)
{
    int freepkt=0;
    if (pkt==0) { pkt = malloc(sizeof(packet_t)); freepkt=1;}
    pkt->src = rank;
	int tmp = incLamport();
	pkt->ts = tmp;
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    if (freepkt) free(pkt);
}
void sendPacketR(packet_t* pkt, int destination, int tag)
{
	int freepkt = 0;
	if (pkt == 0) { 
		pkt = malloc(sizeof(packet_t)); freepkt = 1;
		
	}
	pkt->src = rank;
	pkt->ts = lamport;
	MPI_Send(pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
	if (freepkt) free(pkt);
}

void changeState( state_t newState,const char* nazwa )
{
    pthread_mutex_lock( &stateMut );
    stan = newState;
	debug("Zmieniam stan na {%s}", nazwa);
    pthread_mutex_unlock( &stateMut );
}

int main(int argc, char **argv)
{
    /* Tworzenie wątków, inicjalizacja itp */
    inicjuj(&argc,&argv); // tworzy wątek komunikacyjny w "watek_komunikacyjny.c"
    mainLoop();          // w pliku "watek_glowny.c"

    finalizuj();
    return 0;
}

