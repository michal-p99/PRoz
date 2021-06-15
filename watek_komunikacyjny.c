#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( stan!=InFinish ) {
	debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		incBiggerLamport( pakiet.ts);
		
        switch ( status.MPI_TAG ) {
		case FINISH: 
				changeState(InFinish);
	    break;
		case S_PAIR:
                debug("Dostałem wiadomość PAIRING od %d ",pakiet.src);
				q_element_t elem;
				elem.priority = pakiet.ts;
				elem.process = pakiet.src;
				insertElement(&queue, elem);
				if (size >= 4)
					debug("4 pierwsze elementu kolejki: [%d, %d, %d, %d, ...", queue.data[0].process, queue.data[1].process, queue.data[2].process, queue.data[3].process);
	    break;
        case REQ_MISKA:
			numberReceived++;
            globalState += pakiet.data;
            if (numberReceived > size-1) {
				debug("W magazynach mamy %d funtów łoju.", globalState);
                } 
            break;
	    default:
	    break;
        }
    }
}
