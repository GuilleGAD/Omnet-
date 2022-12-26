#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>

using namespace omnetpp;

class Net: public cSimpleModule {
private:
    simtime_t delayHorario = -1;
    simtime_t delayAntiHorario = -1;
public:
    Net();
    virtual ~Net();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Net);

#endif /* NET */

Net::Net() {
}

Net::~Net() {
}

void Net::initialize() {
}

void Net::finish() {
}

void Net::handleMessage(cMessage *msg) {

    // All msg (events) on net are packets
    Packet *pkt = (Packet *) msg;

    // If this node is the final destination, send to App
    if (pkt->getDestination() == this->getParentModule()->getIndex()) {
        // si el kind es 1 creamos 2 paquetes para enviar por ambos lados
        // para que el emisor conozca el camino mas corto
        if(pkt->getKind()==1){
            Packet *pktH = new Packet("horario");
            pktH->setByteLength(125000);
            pktH->setSource(this->getParentModule()->getIndex());
            pktH->setDestination(pkt->getSource());
            pktH->setKind(2);

            Packet *pktA = new Packet("antihorario");
            pktA->setByteLength(125000);
            pktA->setSource(this->getParentModule()->getIndex());
            pktA->setDestination(pkt->getSource());
            pktA->setKind(2);

            send(pktH, "toLnk$o", 1);
            send(pktA, "toLnk$o", 0);
            // send to net layer
            send(pkt, "toApp$o");
        // si el kind es 2 significa que llegó una respuesta por alguno de
        // los dos caminos diciendonos por donde debemos enviar
        }else if(pkt->getKind()==2){
            if(strcmp(pkt->getName(),"horario")==0){
                delayHorario = pkt->getArrivalTime();
            }else{
                delayAntiHorario = pkt->getArrivalTime();
            }
            // por mas que no es un paquete de datos relevante, se envia a la capa de App
            // para avisarle que comience a mandar el resto de los paquetes
            send(msg, "toApp$o");
        }else{
            send(msg, "toApp$o");
        }
    }
    // If not, forward the packet to some else... to who?
    else {
        // We send to link interface #0, which is the
        // one connected to the clockwise side of the ring
        // Is this the best choice? are there others?

        // si kind es 0 significa que fue creado por nuestra capa de App y debe ser enviada
        // a su destino por el lado mas apropiado
        if(pkt->getKind()==0){
            if(delayAntiHorario != -1 && delayHorario == -1){
                msg->setKind(5);
                send(msg, "toLnk$o", 1);
            }else if(delayAntiHorario == -1 && delayHorario != -1){
                send(msg, "toLnk$o", 0);
            }else if(delayAntiHorario < delayHorario){
                msg->setKind(5);
                send(msg, "toLnk$o", 1);
            }else {
                send(msg, "toLnk$o", 0);
            }

        // si el kind es 1 es porque es el primer paquete y por defecto se manda en horario
        }else if(pkt->getKind()==1){
            send(msg, "toLnk$o", 0);

        // si el kind está en 2 significa que es una respuesta sobre la topología de la red
        // para otro emisor por lo tanto seguimos mandando por el camino contrario al que llegó
        }else if(pkt->getKind()==2){
            if(strcmp(pkt->getName(),"horario")==0){
                send(pkt, "toLnk$o", 1);
            }else{
                send(pkt, "toLnk$o", 0);
            }
        // si kind es 5 es porque es un paquete de datos para otro receptor que debe
        // ser enviado por el lado contrareloj
        }else if(pkt->getKind()==5){
            send(msg, "toLnk$o", 1);

    }
}
