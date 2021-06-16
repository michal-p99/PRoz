#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while (TRUE) {
	debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		incBiggerLamport( pakiet.ts);
		


        switch ( status.MPI_TAG ) {

		case REQ_I:
			debug("Dostałem wiadomość PAIRING od %d ts %d",pakiet.src,pakiet.ts);
			q_element_t elem;
			elem.priority = pakiet.ts;
			elem.process = pakiet.src;
			insertElem(&queue, elem);
			if (size >= 4)
				debug("4 pierwsze elementu kolejki: [%d, %d, %d, %d, ...", queue.data[0].process, queue.data[1].process, queue.data[2].process, queue.data[3].process);

			sendPacket(0, pakiet.src, ACK_I);
			break;

        case ACK_I:
			debug("Dostałem wiadomość ACK_I od %d ts %d", pakiet.src,pakiet.ts);
			if (stan == PAIRING) {
				int myPos = findPosition(&queue, rank);
				if (myPos % 2 == 1) {
					przeciwnik = queue.data[myPos - 1].process;
					debug("Moim przeciwnikiem jest %d", przeciwnik);
					changeState(START_SALA, "START_SALA");

					int zegar = lamport;
					for (int i = 0; i < size; i++) {
						if (i != rank) {
							packet_t pakiet;
							pakiet.ts = zegar;
							pakiet.data = queue.data[myPos - 1].process;
							sendPacketR(&pakiet, i, PAIR);
						}
					}
					removeProcess(&queue, queue.data[myPos].process);
					removeProcess(&queue, queue.data[myPos - 1].process);
					debug("4 pierwsze elementu kolejki: [%d, %d, %d, %d, ...", queue.data[0].process, queue.data[1].process, queue.data[2].process, queue.data[3].process);


					ackCountSala = 0;
					ackSPriority = zegar;
					debug("priority sala %d", ackSPriority);
					for (int i = 0; i < size; i++)
					{
						if (i != rank)
						{
							packet_t* pkt = malloc(sizeof(packet_t));
							pkt->ts = lamport;
							sendPacketR(pkt, i, REQ_SALA);
						}
					}



				}
			}
            break;

		case PAIR:
			debug("Dostałem wiadomość PAIR od %d z %d ts %d", pakiet.src,pakiet.data,pakiet.ts);
			removeProcess(&queue, pakiet.src);
			removeProcess(&queue, pakiet.data);
			debug("4 pierwsze elementu kolejki: [%d, %d, %d, %d, ...", queue.data[0].process, queue.data[1].process, queue.data[2].process, queue.data[3].process);
			if (pakiet.data == rank) {
				debug("Moim przeciwnikiem jest %d ", pakiet.src);
				changeState(START_ZASOB, "CZEKAJ_SALA");
			}


			break;

		case REQ_SALA:
			debug("Otrzymałem REQ_SALA od %d", pakiet.src);
			if ((stan == START_SALA && pakiet.ts > ackSPriority) ||
				(stan == START_SALA && pakiet.ts == ackSPriority) && rank < pakiet.src)
			{
				elem.priority = pakiet.ts;
				elem.process = pakiet.src;
				insertElem(&queue, elem);
			}
			else
			{
				packet_t* pkt = malloc(sizeof(packet_t));
				pkt->ts = pakiet.ts;
				debug("pakiet ts %d", pakiet.ts);
				sendPacket(pkt, pakiet.src, ACK_SALA);
			}



			break;
		case ACK_SALA:
			debug("Otrzymałem ACKSALA od %d prio %d", pakiet.src,pakiet.ts);
			if (stan == START_SALA && pakiet.ts == ackSPriority)
			{
				ackCountSala++;
				if (ackCountSala == size - SALE)
				{
					ackCountSala = 0;
					sendPacket(0, przeciwnik, JEST_SALA);
					changeState(START_ZASOB, "STAT_ZASOB");
				}
			}

			break;

		case JEST_SALA:
			if (stan == CZEKAJ_SALA) {
				debug("Przeciwnik %d wynajął salę", przeciwnik);
				changeState(START_ZASOB, "STAT_ZASOB");
			}


			break;

	    default:
	    break;
        }
    }
}
