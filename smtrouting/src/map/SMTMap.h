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

#ifndef __SMTROUTING_SMTMAP_H_
#define __SMTROUTING_SMTMAP_H_

#include <omnetpp.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include "SMTLaunchd.h"
#include "SMTComInterface.h"
#include "SMTSegment.h"
#include "SMTConnection.h"

using std::string;
using std::vector;
using std::map;
using std::list;
using std::set;

class SMTEdge;
class SMTLane;
class SMTJunction;
class SMTTLLogic;
class SMTPhase;
class SMTRoute;
/**
 * SMTEdge:道路edge.
 */
class SMTEdge {
public:
    SMTEdge() :
            priority(-1), isInternal(false) {
    }
    virtual ~SMTEdge();
    // xml attributes
    string id;
    string from;
    string to;
    int priority;
    string function;

    // SMT attributes
    bool isInternal;    // 标识是否为internal edge
    vector<SMTLane*> laneVector;    // vector of related lanes
    vector<SMTConnection*> conVector;   // vector of related connections

    // optimized attributes
    map<SMTEdge*, vector<SMTRoute*> > routeVecMap;
};
/**
 * SMTLane:车道lane.
 */
class SMTLane {
public:
    SMTLane() :
            index(0), speed(0), length(0), edge(NULL) {
    }
    virtual ~SMTLane();
    // xml attributes
    string id;  // The id of the lane
    int index;  // A running number, starting with zero at the right-most lane
    double speed;   // The maximum speed allowed on this lane [m/s]
    double length;  // The length of this lane [m]
    // shape is ignored

    // SMT attributes
    SMTEdge* edge;  // parent edge
    vector<SMTConnection*> conVector;   // vector of related connections
    vector<SMTLane*> nextVector;   // vector of next lanes
};
/**
 * SMTJunction:路口junction.
 */
class SMTJunction {
    // TODO not finished now
public:
    // seems useless.
    string id; // The id of the junction; please note, that a traffic light definition must use the same ID when controlling this intersection.
    double x; // The x-coordinate of the intersection
    double y; // The y-coordinate of the intersection
    list<string> incLanes; // The ids of the lanes that end at the intersection; sorted by direction, clockwise, with direction up = 0
    list<string> intLanes; // The IDs of the lanes within the intersection
    // shape is ignored

    // SMT attributes
};
/**
 * SMTTlLogic:交通灯逻辑tlLogic.
 */
class SMTTLLogic {
    // TODO not finished now
public:
    SMTTLLogic() :
            offset(0.0) {
    }
    virtual ~SMTTLLogic();
    string id; // The id of the traffic light
    string type; // The type of the traffic light
    string programID; // The id of the traffic light program
    int offset; // The initial time offset of the program
    vector<SMTPhase> phases; // the phase list

    // SMT attributes
    vector<SMTSegment> segments; // the phase segments ordered by link index
};

/**
 * SMTPhase:交通灯相位定义.
 */
class SMTPhase {
    // TODO not finished now
public:
    SMTPhase() :
            duration(0) {
    }
    virtual ~SMTPhase();
    int duration; // time (int),The duration of the phase
    string state; // list of signal states,The traffic light states for this phase
    // minDur,time (int),The minimum duration of the phase (optional), defaults to duration
    // maxDur,time (int),The maximum duration of the phase (optional), defaults to duration
    // minDur and maxDur are ignored.
};

/**
 * SMTRoute:edge到edge的路径.
 */
class SMTRoute {
public:
    SMTRoute() {
    }
    virtual ~SMTRoute();

    list<SMTEdge*> via;    // 路径包含的edge列表(目的街道为终点)
    SMTEdge* start;
    SMTEdge* target;
    double getViaLength();  // 返回via路径的长度
};
/**
 * SMTMap:地图系统,负责管理地图拓扑结构.
 */
class SMTMap: public cSimpleModule {
public:
    SMTMap() :
            debug(false), hasInitialized(false), launchd(0), rouXML(0), netXML(
                    0), stepMsg(0) {
    }
    virtual ~SMTMap();

    SMTLaunchd* getLaunchd();
    bool isReady() {
        return hasInitialized;
    }

    // Map topology API
    SMTEdge* getSMTEdge(string id);

protected:
    // parameters
    bool debug;
    bool hasInitialized;
    SMTLaunchd* launchd;
    cXMLElement* rouXML;
    cXMLElement* netXML;
    cMessage* stepMsg;

    map<string, SMTEdge*> edgeMap;
    map<string, SMTLane*> laneMap;
    map<string, SMTTLLogic*> tlMap;

    // functions
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual void initNetFromXML(cXMLElement* xml);
    virtual void optimizeNet();
    virtual void initVechileTypeFromXML(cXMLElement* xml);
    void addEdgeFromEdgeXML(cXMLElement* xml);
    void addTLFromTLXML(cXMLElement* xml);
    void addConFromConXML(cXMLElement* xml);
    void verifyNetConfig();
};

class SMTMapAccess {
public:
    SMTMap* get() {
        return FindModule<SMTMap*>::findGlobalModule();
    }
};
#endif
