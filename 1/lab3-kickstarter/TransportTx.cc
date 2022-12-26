#ifndef TRANSPORTTX
#define TRANSPORTTX

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class TransportTX: public cSimpleModule {
private:
    cQueue buffer;
    cMessage *endServiceEvent;
    simtime_t serviceTime;
    cOutVector bufferSizeVector;
    cOutVector packetDropVector;
    float delay = 0.0;
public:
    TransportTX();
    virtual ~TransportTX();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportTX);

TransportTX::TransportTX() {
    endServiceEvent = NULL;
}

TransportTX::~TransportTX() {
    cancelAndDelete(endServiceEvent);
}

void TransportTX::initialize() {
    buffer.setName("buffer");
    endServiceEvent = new cMessage("endService");
}

void TransportTX::finish() {
}

void TransportTX::handleMessage(cMessage *msg) {

    //Si el kind es 2 significa que hay que disminuir la velocidad
    //de transmisión
    if(msg->getKind()==2){
        delay = delay + 0.5;
        delete msg;
    //si kind es 3 hay que aumentar la velocidad de transmisión
    //notar que aumentamos mas rapido de lo que disminumimos
    }else if(msg->getKind()==3){
        if(delay>0){
            delay--;
            if(delay<0)
                delay = 0.0;
        }
        delete msg;
    // if msg is signaling an endServiceEvent
    }else if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty()) {
            // dequeue packet
            cPacket *pkt = (cPacket*) buffer.pop();
            // send packet
            send(pkt, "toOut$o");
            // start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime + delay, endServiceEvent);


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

#endif /* TRANSPORTTX */
