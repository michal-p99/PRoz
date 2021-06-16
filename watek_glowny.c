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
						pkt->ts = prio;

						sendPacketR(pkt, i, REQ_I);
					}
				}
				q_element_t elem;
				elem.priority = prio ;
				debug("lamport %d",prio)
				elem.process = rank;
				insertElem(&queue, elem);

				if (size >= 4)
					debug("4 pierwsze elementu kolejki: [%d, %d, %d, %d, ...", queue.data[0].process, queue.data[1].process, queue.data[2].process, queue.data[3].process);
				
            } 
        }
        sleep(SEC_IN_STATE);
    }
}
