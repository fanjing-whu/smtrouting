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

package smt.overrideVeins;

import org.car2x.veins.modules.mobility.traci.TraCIScenarioManagerLaunchd;
//
// 用来获取launchConfig信息
//
simple SMTLaunchd extends TraCIScenarioManagerLaunchd
{
    parameters:
        @class(SMTLaunchd);
        @display("i=block/cogwheel");
}
