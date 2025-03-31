//
//                  Simu5G
//
// Authors: Giovanni Nardini, Giovanni Stea, Antonio Virdis (University of Pisa)
//
// This file is part of a software released under the license included in file
// "license.pdf". Please read LICENSE and README files before using it.
// The above files and the present reference are part of the software itself,
// and cannot be removed from it.
//

package simu5g.apps.mecLatencyApp;

import inet.applications.contract.IApp;


simple MECLatencyApp like IApp
{
    parameters:
        int uePort = default(8000);
        int destPort = default(9000);
        
        @display("i=block/app;is=s");
        volatile int packetSize = intuniform(10,20);
        int localPort = default(9000);
        double coreNetworkDelay @unit("s") = default(0.1s);
        int tos = default(-1); // if not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
//        
//        @signal[recvRequestSno];
//        @statistic[recvRequestSno](title="Received Sequence number"; unit=""; source="recvRequestSno"; record=vector); 
        double period @unit("s") = default(1s);
        @signal[latency];
        @statistic[latency](title="Mec Node test the latency from UE"; unit="s"; ource="latency"; record=vector);
        
    gates:
        output socketOut;
        input socketIn;
}

