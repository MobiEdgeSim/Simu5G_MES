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
import simu5g.nodes.NR.NRUe;

import simu5g.apps.mecLatencyApp.UELatencyApp;
package simu5g.nodes.mec.mobileMec;

module mobileUe extends NRUe
{
//    @display("i=device/pocketpc");
//
    //# Mobility
    mobility.typename = default("VeinsInetMobility");
//
    //# Network Layer specs
    *.routingTableModule = default(absPath(".ipv4.routingTable"));

    ipv4.configurator.typename = "HostAutoConfigurator";
    ipv4.configurator.interfaces = "cellular lo0";

//    submodules:
//        ueLatencyApp: UELatencyApp {
//            //enableMigration = true;//enable to migrate the gnodeb
//            @display("p=673.608,71.604");
//        }
//////        pingApp: PingApp {
//////            //startTime = 0s;
//////            count = -1;
//////            sendInterval = 0.1s; 
//////            continuous = true; 
//////            @display("p=673.608,71.604");
//////        }
//
//    connections:
////        //        pingApp.socketOut --> at.in++;
////        //        pingApp.socketIn <-- at.out++;
//        ueLatencyApp.socketOut --> at.in++;
//        ueLatencyApp.socketIn <-- at.out++;

}
