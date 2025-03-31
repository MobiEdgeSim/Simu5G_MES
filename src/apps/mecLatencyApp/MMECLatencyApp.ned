//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package simu5g.apps.mecLatencyApp;
import inet.applications.contract.IApp;


simple MMECLatencyApp like IApp
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
        double period @unit("s") = default(1s);//period to send the latency test udp
        @signal[latency];
        @statistic[latency](title="Mec Node test the latency from UE"; unit="s"; ource="latency"; record=vector);
        
    gates:
        output socketOut;
        input socketIn;
}
