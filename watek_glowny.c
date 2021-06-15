#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
    srandom(rank);
    while (TRUE) {
        int perc = random()%100; 
		debug("test");
        if (perc<STATE_CHANGE_PROB) {
			debug("test2");
            if (stan==REST) {
				sleep(SEC_IN_STATE);
				debug("test3");
				
				changeState(PAIRING, "PAIRING");

				int zegar = lamport;
				for (int i = 0; i < size; i++) {
					if (i != rank) {
						packet_t pakiet;
						pakiet.ts = zegar;

						sendPacket(&pakiet, i, S_PAIR);
					}
				}
				q_element_t elem;
				elem.priority = zegar;
				elem.process = rank;
				insertElem(&queue, elem);

				if (size >= 4)
					debug("4 pierwsze elementu kolejki: [%d, %d, %d, %d, ...", queue.data[0].process, queue.data[1].process, queue.data[2].process, queue.data[3].process);
				
            } 
        }
        sleep(SEC_IN_STATE);
    }
}
