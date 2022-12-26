simple Generator
{
    parameters:
        volatile double generationInterval; // sec
        volatile double packetByteSize;
        @display("i=block/source");
    gates:
        output out;
}

simple Queue
{
    parameters:
        volatile double serviceTime; // sec
        volatile int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        input in;
        output out;
}

simple TransportTX
{
    parameters:
        int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        input toApp;
        inout toOut;
}

simple TransportRX
{
    parameters:
        int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        output toApp;
        inout toOut;
}

simple Sink
{
    parameters:
        @display("i=block/sink");
    gates:
        input in;
}

module NodeTx
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
        output out;
    submodules:
        gen: Generator {
            parameters:
                @display("p=75,50");
        }

        transTx: TransportTX {
            parameters:
                @display("p=75,190");
        }
    connections:
        gen.out --> transTx.toApp;
        in --> transTx.toOut$i;
        transTx.toOut$o --> out;
}

module NodeRx
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
        output out;
    submodules:
        sink: Sink {
            parameters:
                @display("p=75,50");
        }
        transRx: TransportRX {
            parameters:
                @display("p=75,190");
        }
    connections:
        in --> transRx.toOut$i;
        transRx.toOut$o --> out;
        //case study 1:
        //transRx.toApp --> {  datarate = 0.5Mbps; } --> sink.in;
        //case study 2:
        transRx.toApp --> {  datarate = 1Mbps; } --> sink.in;
}

network Network
{
    @display("bgl=2;bgb=281,166");
    submodules:
        nodeTx: NodeTx {
            @display("p=30,40");
        }
        queuePacketData: Queue {
            @display("p=150,40");
        }
        queuePacketControl: Queue {
            @display("p=150,120");
        }
        nodeRx: NodeRx {
            @display("p=250,40");
        }
    connections:
        nodeTx.out --> {  datarate = 1Mbps; delay = 100us; } --> queuePacketData.in;
        //caso 1:
        //queuePacketData.out --> {  datarate = 1Mbps; delay = 100us; } --> nodeRx.in;
        //caso 2:
        queuePacketData.out --> {  datarate = 0.5Mbps; delay = 100us; } --> nodeRx.in;

        nodeRx.out --> {  datarate = 1Mbps; delay = 100us; } --> queuePacketControl.in;
        //case study 1:
        //queuePacketControl.out --> {  datarate = 1Mbps; delay = 100us; } --> nodeTx.in;
        //case study 2:
        queuePacketControl.out --> {  datarate = 0.5Mbps; delay = 100us; } --> nodeTx.in;

}
