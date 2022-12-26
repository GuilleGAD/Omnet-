#ifndef TRANSPORTRX
#define TRANSPORTRX

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class TransportRX: public cSimpleModule {
private:
    cQueue buffer;
    cMessage *endServiceEvent;
    simtime_t serviceTime;
    cOutVector bufferSizeVector;
    cOutVector packetDropVector;
    //Variable para checkear si hay pedimos al transportTx que suba o baje la
    //valocidad de transmisión
    float delayCheck = 0.0;
    //Variable para setear el minimo de buffer libre que deseamos tener para
    //comenzar a hacer el control de congestion
    float bufferMin = 0.2; //por defecto el 20% del buffer
public:
    TransportRX();
    virtual ~TransportRX();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportRX);

TransportRX::TransportRX() {
    endServiceEvent = NULL;
}

TransportRX::~TransportRX() {
    cancelAndDelete(endServiceEvent);
}

void TransportRX::initialize() {
    buffer.setName("buffer");
    endServiceEvent = new cMessage("endService");
}

void TransportRX::finish() {
}

void TransportRX::handleMessage(cMessage *msg) {
    int lengBuffer = 0;
    int freeBuffer = 0;

    // if msg is signaling an endServiceEvent
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty()) {
            // dequeue packet
            cPacket *pkt = (cPacket*) buffer.pop();
            // send packet to Sink
            send(pkt, "toApp");
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
           //obtenemos la longitud total del buffer
           lengBuffer = par("bufferSize").intValue();
           //obtenemos el tamaño del buffer libre
           freeBuffer = lengBuffer - buffer.getLength();

           //creamos el paquete que vamos a mandar en caso de aumentar
           //o disminuir la velocidad
           cPacket *feedbackPkt = new cPacket();
           //verificamos si el buffer está lleno al 80% o si nos llegó un
           //paquete de control de congestion desde la Queue
           if(freeBuffer<lengBuffer*bufferMin || msg->getKind()==2){
               feedbackPkt->setKind(2);
               delayCheck += 0.5;
           }else{
               if(delayCheck>0){
                   //seteamos el kind en 3 para indicar que vuelva a aumentar
                   //la velocidad
                   feedbackPkt->setKind(3);
                   delayCheck--;
                   if(delayCheck<0)
                       delayCheck = 0.0;
               }
           }
           if (delayCheck>0 || feedbackPkt->getKind()==3){
               feedbackPkt->setByteLength(20);
               send(feedbackPkt,"toOut$o");
           }else{
               //en caso de no necesitarlo eliminamos el paquete
               //de control
               delete feedbackPkt;
           }

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

#endif /* TRANSPORTRX */
