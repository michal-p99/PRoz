#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
    srandom(rank);
    while (TRUE) {
        int perc = random()%100; 
        if (perc<STATE_CHANGE_PROB) {
            if (stan==REST) {
				sleep(SEC_IN_STATE);
				
				changeState(PAIRING, "PAIRING");
				int prio = lamport;
				for (int i = 0; i < size; i++) {
					if (i != rank) {
						packet_t* pkt = malloc(sizeof(packet_t));
						pkt->data = prio;

						sendPacket(pkt, i, REQ_I);
					}
				}
				q_element_t elem;
				elem.priority = prio ;
				elem.process = rank;
				insertElem(&queue, elem);

				//if (size >= 4)
					//debug("4 elem kolejki: [%d, %d, %d, %d, ...", queue.data[0].process, queue.data[1].process, queue.data[2].process, queue.data[3].process);
				
            } 
			if (stan == START_DEBATE){
				if (enemyReady && (rezerwujacy==FALSE)) {
					debug("DEBATA ROZPOCZ?TA z %d", przeciwnik);
					sleep(DEBATE_TIME);
					if (getResult()){
						changeState(REST, "REST");
						debug("WYGRA?EM z %d", przeciwnik);
						packet_t* pkt = malloc(sizeof(packet_t));
						pkt->enemy = getResult();
						sendPacket(pkt, przeciwnik, RESULT);
						returnEverything();
						
						sleep(WINNER_TIME);

					}
					else{
						debug("PRZEGRA?EM z %d", przeciwnik);
						changeState(REST, "REST");
						packet_t* pkt = malloc(sizeof(packet_t));
						pkt->enemy = getResult();
						sendPacket(pkt, przeciwnik, RESULT);
						returnEverything();
					}
				}
				else if (resultReady) {
					if (wynik) {
						debug("DEBATA sko?czona WYGRA?EM z %d", przeciwnik);
						changeState(REST, "REST");
						returnEverything();
						sleep(WINNER_TIME);
					}
					else {
						debug("DEBATA sko?czona PRZEGRALEM z %d", przeciwnik);
						changeState(REST, "REST");
						returnEverything();
						
					}


				}


			}
        }
        sleep(SEC_IN_STATE);
    }
}
void returnEverything() {
	if (pickedZasob == 0) {
		returnMiska();
	}
	else if (pickedZasob == 1) {
		returnPinezki();
	}
	else if (pickedZasob == 2) {
		returnSlipki();
	}
	if (rezerwujacy) {
		returnSale();
	}
	enemyReady = FALSE;
}
void returnMiska() {
	for (int i = queue_zasob.size-1; i >= 0;i--) {
		packet_t* pkt = malloc(sizeof(packet_t));
		pkt->data = queue_zasob.data[i].priority;
		sendPacket(pkt, queue_zasob.data[i].process, ACK_MISKA);
		queue_zasob.data[i].process = -1;
		queue_zasob.data[i].priority = -1;
		queue_zasob.size--;
	}
	countQueueZasobSize = 0;
	debug("Wychodz?: Sekcja krytyczna zasobu - miska");
}
void returnPinezki(){
	for (int i = queue_zasob.size-1; i >= 0; i--) {
		packet_t* pkt = malloc(sizeof(packet_t));
		pkt->data = queue_zasob.data[i].priority;
		sendPacket(pkt, queue_zasob.data[i].process, ACK_PINEZKI);
		queue_zasob.data[i].process = -1;
		queue_zasob.data[i].priority = -1;
		queue_zasob.size--;
	}
	countQueueZasobSize = 0;
	}
void returnSlipki() {
	for (int i = queue_zasob.size-1; i >= 0; i--) {
		packet_t* pkt = malloc(sizeof(packet_t));
		pkt->data = queue_zasob.data[i].priority;
		sendPacket(pkt, queue_zasob.data[i].process, ACK_SLIPKI);
		queue_zasob.data[i].process = -1;
		queue_zasob.data[i].priority = -1;
		queue_zasob.size--;
	}
	countQueueZasobSize = 0;
	debug("Wychodz?: Sekcja krytyczna zasobu - slipki");
}
void returnSale() {
	for (int i = queue_sala.size-1; i >=0; i--) {
		packet_t* pkt = malloc(sizeof(packet_t));
		pkt->data = queue_sala.data[i].priority;
		sendPacket(pkt, queue_sala.data[i].process, ACK_SALA);
		queue_sala.data[i].process = -1;
		queue_sala.data[i].priority = -1;
		queue_sala.size--;
	}
	countQueueSalaSize = 0;
	rezerwujacy = FALSE;
	debug("Wychodz?: Sekcja krytyczna zasobu - sale");
}
int getResult() {
	if (pickedZasob == enemyPickedZasob) {
		if (przeciwnik > rank) {
			return TRUE;
		}
		return FALSE;
	}
	if (pickedZasob == 0 && enemyPickedZasob == 2)
		return TRUE;
	if (pickedZasob == 1 && enemyPickedZasob == 0)
		return TRUE;
	if (pickedZasob == 2 && enemyPickedZasob == 1)
		return TRUE;
	return FALSE;
}
