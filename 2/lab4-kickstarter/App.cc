#ifndef APP
#define APP

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>
#include <time.h>

using namespace omnetpp;

class App: public cSimpleModule {
private:
    cMessage *sendMsgEvent;
    cStdDev delayStats;
    cOutVector delayVector;
    bool firstPkt = true;
    bool sendOk = true;
public:
    App();
    virtual ~App();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(App);

#endif /* APP */

App::App() {
}

App::~App() {
}

void App::initialize() {

    // If interArrivalTime for this node is higher than 0
    // initialize packet generator by scheduling sendMsgEvent
    if (par("interArrivalTime").doubleValue() != 0) {
        sendMsgEvent = new cMessage("sendEvent");
        scheduleAt(par("interArrivalTime"), sendMsgEvent);
    }

    // Initialize statistics
    delayStats.setName("TotalDelay");
    delayVector.setName("Delay");
}

void App::finish() {
    // Record statistics
    recordScalar("Average delay", delayStats.getMean());
    recordScalar("Number of packets", delayStats.getCount());
}

void App::handleMessage(cMessage *msg) {

    // if msg is a sendMsgEvent, create and send new packet
    if (msg == sendMsgEvent) {
        // create new packet
        Packet *pkt = new Packet("packet");
        pkt->setByteLength(par("packetByteSize"));
        pkt->setSource(this->getParentModule()->getIndex());
        pkt->setDestination(par("destination"));
        //seteamos el kind en 0 para darnos cuenta que es un paquete
        //de datos
        if(firstPkt){
            //seteamos el kind en 1 para avisar que ademas de ser un paquete
            //de datos, estamos pidiendo que nos respondan con los datos de
            //delay de cada ruta
            pkt->setKind(1);
            firstPkt = false;
            sendOk = false;
        }else{
            pkt->setKind(0);
        }

        // send to net layer
        send(pkt, "toNet$o");

        // compute the new departure time and schedule next sendMsgEvent
        // en caso de ser el pimer paquete no ingresaría y se quedaría esperando
        // al paquete de respuesta con la topología de la red para comenzar a enviar
        // el resto
        if(sendOk){
            simtime_t departureTime = simTime() + par("interArrivalTime");
            scheduleAt(departureTime, sendMsgEvent);
        }

    }
    // else, msg is a packet from net layer
    else {
        // compute delay and record statistics
        // como se inicializa en true, nunca entraría en caso de solo recibir paquetes
        // ingresaría solo si se envió un primer paquete para el analisas de la red
        if(!sendOk){
            simtime_t departureTime = simTime() + par("interArrivalTime");
            scheduleAt(departureTime, sendMsgEvent);
            sendOk = true;
        }
        simtime_t delay = simTime() - msg->getCreationTime();
        delayStats.collect(delay);
        delayVector.record(delay);

        // delete msg
        delete (msg);
    }

}
