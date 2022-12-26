#ifndef QUEUE
#define QUEUE

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Queue: public cSimpleModule {
private:
    cQueue buffer;
    cMessage *endServiceEvent;
    simtime_t serviceTime;
    cOutVector bufferSizeVector;
    cOutVector packetDropVector;
    //Variable para setear el minimo de buffer libre que deseamos tener para
    //comenzar a hacer el control de congestion
    float bufferMin = 0.2; //por defecto el 20% del buffer
public:
    Queue();
    virtual ~Queue();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Queue);

Queue::Queue() {
    endServiceEvent = NULL;
}

Queue::~Queue() {
    cancelAndDelete(endServiceEvent);
}

void Queue::initialize() {
    buffer.setName("buffer");
    endServiceEvent = new cMessage("endService");
}

void Queue::finish() {
}

void Queue::handleMessage(cMessage *msg) {
    int lengBuffer = 0;
    int freeBuffer = 0;

    // if msg is signaling an endServiceEvent
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty()) {
            // dequeue packet
            cPacket *pkt = (cPacket*) buffer.pop();

            //obtenemos la longitud total del buffer
            lengBuffer = par("bufferSize").intValue();
            //obtenemos el tamaño del buffer libre
            freeBuffer = lengBuffer - buffer.getLength();

            //pedimos disminuir la velocidad si el buffer está al 80% de uso
            if(freeBuffer<lengBuffer*bufferMin){
                pkt->setKind(2);
            }

            // send packet
            send(pkt, "out");
            // start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime, endServiceEvent);

        }
    } else { // if msg is a data packet
    	if(buffer.getLength() >= par("bufferSize").longValue()){
			// drop the packet
			delete msg;
			this -> bubble("packet dropped");
			packetDropVector.record(1);
		}
		else {
			// enqueue the packet
			buffer.insert(msg);
			bufferSizeVector.record(buffer.getLength());
			// if the server is idle
			if (!endServiceEvent->isScheduled()) {
				// start the service
				scheduleAt(simTime() + 0, endServiceEvent);

			}

		}
    }
}

#endif /* QUEUE */
