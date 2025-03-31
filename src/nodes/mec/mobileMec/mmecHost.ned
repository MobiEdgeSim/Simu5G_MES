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

import simu5g.nodes.Upf;
import simu5g.nodes.mec.MECHost;
import simu5g.stack.ICellularNic;
import inet.mobility.contract.IMobility;
import inet.node.ethernet.Eth10G;
import simu5g.nodes.mec.mobileMec.mecHostConfigurator.MecHostConfigurator;

package simu5g.nodes.mec.mobileMec;


//the parameter withcellular of UPF shoule be set as true in ini
module mmecHost extends MECHost
{
    parameters:
        //ifVim= false;
        virtualisationInfrastructure.withcellular = true;//important! Only this, cellular module can be added
        upf_mec.withcellular = false;//if true fon't forget to set the parameter in ini file
        							//*.mmecHost[*].upf_mec.cellularNic.nrPhy.dynamicCellAssociation = true

        mobility.typename = default("VeinsInetMobility");


        //# Network Layer specs
        upf_mec.**.routingTableModule = default(absPath(".upf_mec.ipv4.routingTable"));
        //upf_mec.**.routingTableModule = default("^.ipv4.routingTable");


        upf_mec.ipv4.configurator.typename = "HostAutoConfigurator";
        upf_mec.ipv4.configurator.interfaces = "lo0 pppIf";
        //upf_mec.ipv4.configurator.interfaces = "pppIf";
        //upf_mec.ipv4.configurator.interfaces = "lo0 pppIf";
        //upf_mec.ipv4.configurator.interfaces = "cellular lo0";

        virtualisationInfrastructure.**.routingTableModule = default(absPath(".virtualisationInfrastructure.ipv4.routingTable"));
        virtualisationInfrastructure.ipv4.configurator.typename = "HostAutoConfigurator";
        virtualisationInfrastructure.ipv4.configurator.interfaces = "cellular";
        //virtualisationInfrastructure.ipv4.configurator.interfaces = "cellular";

        connectUpf = true;

    gates:
//        input radioIn @directIn;
//		input nrRadioIn @directIn;

    submodules:
//        mecHostConfigurator: MecHostConfigurator {
//            @display("p=86.595,165.65999");
//        }
//        mobility: <default("")> like IMobility if typename != "" {
//            @display("p=86.595,67.77");
//        }//"VeinsInetMobility"

    connections:
//        radioIn --> virtualisationInfrastructure.radioIn_vir;
//        nrRadioIn--> virtualisationInfrastructure.nrRadioIn_vir;

}
