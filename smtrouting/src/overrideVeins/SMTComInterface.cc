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

#include "SMTComInterface.h"
#include "TraCIBuffer.h"
#include "TraCIConnection.h"
#include "TraCIConstants.h"

using Veins::TraCIBuffer;

SMTComInterface::~SMTComInterface() {

}

SMTComInterface::SMTComInterface(Veins::TraCIConnection& con) :
        Veins::TraCICommandInterface(con), smtConnection(con) {
}

std::list<std::string> SMTComInterface::getLaneLinkedLaneIds(
        std::string laneId) {
    uint8_t commandId = CMD_GET_LANE_VARIABLE;
    uint8_t variableId = LANE_LINKS;
    uint8_t responseId = RESPONSE_GET_LANE_VARIABLE;
    uint8_t resultTypeId = TYPE_COMPOUND;
    std::list<std::string> res;

    TraCIBuffer buf = smtConnection.query(commandId,
            TraCIBuffer() << variableId << laneId);

    uint8_t cmdLength;
    buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r;
    buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId;
    buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r;
    buf >> objectId_r;
    ASSERT(objectId_r == laneId);
    uint8_t resType_r;
    buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint32_t count;
    buf >> count;
    uint8_t numType;
    buf >> numType;
    ASSERT(numType == TYPE_INTEGER);
    uint32_t linkNumber;
    buf >> linkNumber;
    for (uint32_t i = 0; i < linkNumber; i++) {
        // external id
        // consecutive not internal lane
        {
            uint8_t typeId;
            buf >> typeId;
            ASSERT(typeId == TYPE_STRING);
            std::string str;
            buf >> str;
            res.push_back(str);
        }
        // internal id
        // consecutive internal lane
        {
            uint8_t typeId;
            buf >> typeId;
            ASSERT(typeId == TYPE_STRING);
            std::string str;
            buf >> str;
        }
        // has priority (=1) or not (=0)
        {
            uint8_t typeId;
            buf >> typeId;
            ASSERT(typeId == TYPE_UBYTE);
            uint8_t u;
            buf >> u;
        }
        // is opened (=1) or not (=0)
        {
            uint8_t typeId;
            buf >> typeId;
            ASSERT(typeId == TYPE_UBYTE);
            uint8_t u;
            buf >> u;
        }
        // has approaching foe (=1) or not (=0)
        {
            uint8_t typeId;
            buf >> typeId;
            ASSERT(typeId == TYPE_UBYTE);
            uint8_t u;
            buf >> u;
        }
        // (current) state (not yet implemented, always="")
        {
            uint8_t typeId;
            buf >> typeId;
            ASSERT(typeId == TYPE_STRING);
            std::string str;
            buf >> str;
        }
        // direction (not yet implemented, always="")
        {
            uint8_t typeId;
            buf >> typeId;
            ASSERT(typeId == TYPE_STRING);
            std::string str;
            buf >> str;
        }
        // length [m] - only valid if not using internal lanes
        {
            uint8_t typeId;
            buf >> typeId;
            ASSERT(typeId == TYPE_DOUBLE);
            double d;
            buf >> d;
        }
    }

    ASSERT(buf.eof());

    return res;
}

void SMTComInterface::setLaneChangeMode(std::string nodeId,
        SMTLaneChangeMode mode) {
    uint8_t variableId = VAR_LANECHANGE_MODE;
    uint8_t variableType = TYPE_INTEGER;
    TraCIBuffer buf = smtConnection.query(CMD_SET_VEHICLE_VARIABLE,
            TraCIBuffer() << variableId << nodeId << variableType << mode);
    ASSERT(buf.eof());
}

void SMTComInterface::changeLane(std::string nodeId, uint8_t laneIndex,
        uint32_t duration) {
    uint8_t variableId = CMD_CHANGELANE;
    uint8_t variableType = TYPE_COMPOUND;
    uint8_t ByteType = TYPE_BYTE;
    uint8_t IntType = TYPE_INTEGER;

    uint32_t num = 2;
    TraCIBuffer buf = TraCIBuffer();
    buf << variableId << nodeId << variableType << num << ByteType << laneIndex
            << IntType << duration;
    TraCIBuffer buf0 = smtConnection.query(CMD_SET_VEHICLE_VARIABLE, buf);
    ASSERT(buf0.eof());
}

void SMTComInterface::setVehicleArrived(std::string nodeId) {
    uint8_t variableId = REMOVE;
    uint8_t variableType = TYPE_BYTE;
    uint8_t value = REMOVE_ARRIVED;
    TraCIBuffer buf = smtConnection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}

void SMTComInterface::changeVehicleTarget(std::string nodeId,
        std::string destination) {
    uint8_t variableId = CMD_CHANGETARGET;
    uint8_t variableType = TYPE_STRING;
    TraCIBuffer buf = smtConnection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << destination);
    ASSERT(buf.eof());
}

void SMTComInterface::addRoute(std::string routeId,
        std::list<std::string> route) {
    uint8_t variableId = ADD;
    uint8_t variableType = TYPE_STRINGLIST;
    int32_t count = route.size();
    TraCIBuffer buf = TraCIBuffer();
    buf << variableId << routeId << variableType << count;
    for(std::list<std::string>::iterator it = route.begin(); it != route.end(); it++){
        buf << (*it);
    }
    buf = smtConnection.query(CMD_SET_ROUTE_VARIABLE, buf);
    ASSERT(buf.eof());
}
