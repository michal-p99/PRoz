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
	//debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		incBiggerLamport( pakiet.ts);
		


        switch ( status.MPI_TAG ) {

		case REQ_I:
			debug("Dostałem wiadomość PAIRING od %d ",pakiet.src);
			q_element_t elem;
			elem.priority = pakiet.data;
			elem.process = pakiet.src;
			insertElem(&queue, elem);
			
			//if (size >= 4)
				//debug("4 elem kolejki: [%d, %d, %d, %d, ...", queue.data[0].process, queue.data[1].process, queue.data[2].process, queue.data[3].process);

			sendPacket(0, pakiet.src, ACK_I);
			break;

        case ACK_I:
			//debug("Dostałem wiadomość ACK_I od %d ts %d", pakiet.src,pakiet.data);
			if (stan == PAIRING) {
				ackCountI++;
				if (ackCountI == size - 1) {
					ackCountI = 0;
					int myPos = findPosition(&queue, rank);
					if (myPos % 2 == 1) {
						przeciwnik = queue.data[myPos - 1].process;
						debug("Moim przeciwnikiem jest %d", przeciwnik);
						changeState(START_SALA, "START_SALA");

						int zegar = lamport;
						for (int i = 0; i < size; i++) {
							if (i != rank) {
								packet_t* pkt = malloc(sizeof(packet_t));
								pkt->data = zegar;
								pkt->enemy = queue.data[myPos - 1].process;
								sendPacket(pkt, i, PAIR);
							}
						}
						removeProcess(&queue, queue.data[myPos].process);
						removeProcess(&queue, queue.data[myPos - 1].process);
						//debug("4 elem kolejki: [%d, %d, %d, %d, ...", queue.data[0].process, queue.data[1].process, queue.data[2].process, queue.data[3].process);


						ackCountSala = 0;
						ackSPriority = zegar;
						rezerwujacy = TRUE;
						//debug("priority sala %d", ackSPriority);
						for (int i = 0; i < size; i++)
						{
							if (i != rank)
							{
								packet_t* pkt = malloc(sizeof(packet_t));
								pkt->data = ackSPriority;
								sendPacket(pkt, i, REQ_SALA);
							}
						}



					}
				}
			}
            break;

		case PAIR:
			//debug("Dostałem wiadomość PAIR od %d z %d ts %d", pakiet.src,pakiet.enemy,pakiet.data);
			removeProcess(&queue, pakiet.src);
			removeProcess(&queue, pakiet.enemy);
			//debug("4 elem kolejki: [%d, %d, %d, %d, ...", queue.data[0].process, queue.data[1].process, queue.data[2].process, queue.data[3].process);
			if (pakiet.enemy == rank) {
				debug("Moim przeciwnikiem jest %d ", pakiet.src);
				przeciwnik = pakiet.src;
				changeState(CZEKAJ_SALA, "CZEKAJ_SALA");
			}

			 
			break;

		case REQ_SALA:
			//debug("Otrzymałem REQ_SALA od %d", pakiet.src);
			if (((stan == START_ZASOB ||stan == START_DEBATE) && rezerwujacy) ||
				(stan == START_SALA && pakiet.data > ackSPriority) ||
				(stan == START_SALA && pakiet.data == ackSPriority) && rank < pakiet.src)
			{
				countQueueSalaSize++;
				elem.priority = pakiet.data;
				elem.process = pakiet.src;
				insertElem(&queue_sala, elem);
			}
			else
			{
				packet_t* pkt = malloc(sizeof(packet_t));
				pkt->data = pakiet.data;
				//debug("pakiet ts %d", pakiet.data);
				sendPacket(pkt, pakiet.src, ACK_SALA);
			}



			break;
		case ACK_SALA:
			//debug("Otrzymałem ACKSALA od %d prio %d", pakiet.src,pakiet.data);
			if (stan == START_SALA && pakiet.data == ackSPriority)
			{
				ackCountSala++;
				if (ackCountSala == size - SALE)
				{
					debug("Sekcja krytyczna sal");
					ackCountSala = 0;
					sendPacket(0, przeciwnik, JEST_SALA);
					changeState(START_ZASOB, "START_ZASOB");
					getZasob();
				}
			}

			break;

		case JEST_SALA:
			if (stan == CZEKAJ_SALA) {
				debug("Przeciwnik %d wynajął salę", przeciwnik);
				changeState(START_ZASOB, "START_ZASOB");
				getZasob();
			}


			break;

		case REQ_MISKA:
			//debug("Otrzymałem REQ_MISKA od %d prio %d", pakiet.src, pakiet.data);
			
			if (pickedZasob == 0 && (stan == START_DEBATE || (stan == START_ZASOB && pakiet.data > ackZPriority) || (stan == START_ZASOB && pakiet.data == ackZPriority && rank < pakiet.src))) {
				countQueueZasobSize++;
				elem.priority = pakiet.data;
				elem.process = pakiet.src;
				insertElem(&queue_zasob, elem);
			}
			else {
				packet_t* pkt = malloc(sizeof(packet_t));
				pkt->data = pakiet.data;
				sendPacket(pkt, pakiet.src, ACK_MISKA);
			}
			if (pakiet.src == przeciwnik)
				enemyPickedZasob = 0;
				
			break;


		case REQ_PINEZKI:
			//debug("Otrzymałem REQ_PINEZKI od %d prio %d", pakiet.src, pakiet.data);

			if (pickedZasob == 1 && ((stan == START_ZASOB && pakiet.data > ackZPriority) || (stan == START_ZASOB && pakiet.data == ackZPriority && rank < pakiet.src))) {
				countQueueZasobSize++;
				elem.priority = pakiet.data;
				elem.process = pakiet.src;
				insertElem(&queue_zasob, elem);
			}
			else {
				packet_t* pkt = malloc(sizeof(packet_t));
				pkt->data = pakiet.data;
				sendPacket(pkt, pakiet.src, ACK_PINEZKI);
			}
			if (pakiet.src == przeciwnik)
				enemyPickedZasob = 1;

			break;


		case REQ_SLIPKI:
			//debug("Otrzymałem REQ_SLIPKI od %d prio %d", pakiet.src, pakiet.data);

			if (pickedZasob == 2 && ((stan == START_ZASOB && pakiet.data > ackZPriority) || (stan == START_ZASOB && pakiet.data == ackZPriority && rank < pakiet.src))) {
				countQueueZasobSize++;
				elem.priority = pakiet.data;
				elem.process = pakiet.src;
				insertElem(&queue_zasob, elem);
			}
			else {
				packet_t* pkt = malloc(sizeof(packet_t));
				pkt->data = pakiet.data;
				sendPacket(pkt, pakiet.src, ACK_SLIPKI);
			}
			if (pakiet.src == przeciwnik)
				enemyPickedZasob = 2;

			break;

		case ACK_MISKA:
			//debug("Otrzymałem ACK_MISKA od %d prio %d", pakiet.src, pakiet.data);
			if (stan == START_ZASOB && pickedZasob == 0 && pakiet.data == ackZPriority) {
				ackCountZasob++;
				if (ackCountZasob >= size - MISKA) {
					ackCountZasob = 0;
					debug("Sekcja krytyczna zasobu - miska");
					if (rezerwujacy) {
						changeState(START_DEBATE, "START_DEBATE");
						sendPacket(0, przeciwnik, READY);
					}
					else {
						if (enemyReady) {
							changeState(START_DEBATE, "START_DEBATE");
						}
						else {
							changeState(WAIT_READY, "WAIT_READY");
						}
							
					}
					
				}
			}
			break;

		case ACK_PINEZKI:
			//debug("Otrzymałem ACK_PINEZKI od %d prio %d", pakiet.src, pakiet.data);
			if (stan == START_ZASOB && pickedZasob == 1 && pakiet.data == ackZPriority) {
				ackCountZasob++;
				if (ackCountZasob >= size - PINEZKI) {
					ackCountZasob = 0;
					debug("Sekcja krytyczna zasobu - pinezki");
					if (rezerwujacy) {
						changeState(START_DEBATE, "START_DEBATE");
						sendPacket(0, przeciwnik, READY);
					}
					else {
						if (enemyReady) {
							changeState(START_DEBATE, "START_DEBATE");
						}
						else {
							changeState(WAIT_READY, "WAIT_READY");
						}

					}
					
				}
			}
			break;


		case ACK_SLIPKI:
			//debug("Otrzymałem ACK_SLIPKI od %d prio %d", pakiet.src, pakiet.data);
			if (stan == START_ZASOB && pickedZasob == 2 && pakiet.data == ackZPriority) {
				ackCountZasob++;
				if (ackCountZasob >= size - SLIPKI) {
					ackCountZasob = 0;
					debug("Sekcja krytyczna zasobu - slipki");
					if (rezerwujacy) {
						changeState(START_DEBATE, "START_DEBATE");
						sendPacket(0, przeciwnik, READY);
					}
					else {
						if (enemyReady) {
							changeState(START_DEBATE, "START_DEBATE");
						}
						else {
							changeState(WAIT_READY, "WAIT_READY");
						}

					}
					
				}
			}
			break;

		case READY:
			debug("Otrzymałem READY od %d prio %d", pakiet.src, pakiet.data);
			enemyReady = TRUE;
			if (stan == WAIT_READY) {
				changeState(START_DEBATE, "START_DEBATE");
			}
			
			break;

		case RESULT:
			debug("Otrzymałem RESULT od %d wynik %d", pakiet.src,pakiet.enemy);
			
			wynik = pakiet.enemy;
			resultReady = TRUE;

	    default:
	    break;
        }
    }
}

void getZasob() {
	pickedZasob = rand() % 3;
	ackZPriority = lamport;
	if (pickedZasob == 0) {
		debug("Wybieram MISKĘ");
		
		for (int i = 0; i < size; i++)
		{
			if (i != rank)
			{
				packet_t* pkt = malloc(sizeof(packet_t));
				pkt->data = ackZPriority;
				sendPacket(pkt, i, REQ_MISKA);
			}
		}
	}
	else if (pickedZasob == 1) {
		debug("Wybieram PINEZKI");
		for (int i = 0; i < size; i++)
		{
			if (i != rank)
			{
				packet_t* pkt = malloc(sizeof(packet_t));
				pkt->data = ackZPriority;
				sendPacket(pkt, i, REQ_PINEZKI);
			}
		}
	}
	else if (pickedZasob == 2) {
		debug("Wybieram SLIPKI");
		for (int i = 0; i < size; i++)
		{
			if (i != rank)
			{
				packet_t* pkt = malloc(sizeof(packet_t));
				pkt->data = ackZPriority;
				sendPacket(pkt, i, REQ_SLIPKI);
			}
		}
	}

	


}
