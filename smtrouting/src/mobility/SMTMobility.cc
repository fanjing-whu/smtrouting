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

#include "SMTMobility.h"
#include "StatisticsRecordTools.h"

Define_Module(SMTMobility);

SMTMobility::~SMTMobility() {

}

void SMTMobility::initialize(int stage) {
    Veins::TraCIMobility::initialize(stage);
    if (stage == 0) {
        smtMap = getMap();
        title = title + "external_id" + "\t" + "time" + "\t" + "road_id" + "\t"
                + "record_road" + "\t" + "position";
        isChangeAndHold = par("isChangeAndHold");
        laneChangeDuration = par("laneChangeDuration");
    }
}

void SMTMobility::finish() {
    Veins::TraCIMobility::finish();
}

void SMTMobility::preInitialize(std::string external_id, const Coord& position,
        std::string road_id, double speed, double angle) {
    Enter_Method_Silent
    ();
    Veins::TraCIMobility::preInitialize(external_id, position, road_id, speed,
            angle);
}

void SMTMobility::nextPosition(const Coord& position, std::string road_id,
        double speed, double angle,
        Veins::TraCIScenarioManager::VehicleSignal signals) {
    Enter_Method_Silent
    ();
    Veins::TraCIMobility::nextPosition(position, road_id, speed, angle,
            signals);
    if (!hasRouted) {
        if (getMap()->isReady()) {
            // Map system must be initialized first
            // initialize the route
            processAtRouting();
            hasRouted = true;
        }
    } else {
        // process after the routing
        processAfterRouting();
    }
    // road change
    if (road_id != lastRoadId) {
        // statistics process
        if (!hasInitialized) {
            // when the car first appear on the map.
            processWhenInitializingRoad();
            // switch record process trigger
            hasInitialized = true;
        } else {
            // when road changed
            processWhenChangeRoad();
        }
        // in nextPosition the car has been on the road,
        // then updata the last_road_id and enterTime
        SMTEdge* edge = getMap()->getSMTEdge(road_id);
        if (edge->isInternal) {
            if (!lastEdge->isInternal) {
                lastPrimaryRoadId = lastRoadId;
                lastPrimaryEdge = lastEdge;
            }
        } else {
            curPrimaryRoadId = road_id;
            curPrimaryEdge = edge;
            recordRoadId = convertStrToRecordId(road_id);
        }
        lastRoadId = road_id;
        lastEdge = edge;
    }
    // normal process
    processWhenNextPosition();
}

void SMTMobility::handleSelfMsg(cMessage* msg) {
    if (msg == laneChangeMsg) {
        handleLaneChangeMsg(msg);
    } else {
        // cancel and delete the unknown message.
        cancelAndDelete(msg);
    }
}

void SMTMobility::processAfterRouting() {
    // 选路之后每个周期都会执行(请确保判定完备,不要执行复杂度过高的操作)
}

void SMTMobility::statisticAtFinish() {
    // 结束时的统计方法
}

void SMTMobility::processAtRouting() {
    // 选路阶段
    // 设置车道变换模式
    cmdSetNoOvertake();
    if (debug) {
        std::cout << "car " << external_id << " will not make overtake."
                << std::endl;
    }
}

void SMTMobility::processWhenChangeRoad() {
    // 当车辆首次进入某条道路时执行
    // 取消之前设置的laneChange消息
    if (laneChangeMsg != NULL) {
        cancelAndDelete(laneChangeMsg);
        laneChangeMsg = NULL;
    }
    if (road_id == "20/14to18/14") {
        startChangeLane(1,laneChangeDuration);
    }
}

void SMTMobility::processWhenInitializingRoad() {
    // 车辆首次出现在地图上时执行
}

void SMTMobility::processWhenNextPosition() {
    // 车辆变更位置时出现(请确保判定完备,不要执行复杂度过高的操作)
    Fanjing::StatisticsRecordTools *srt =
            Fanjing::StatisticsRecordTools::request();
    double lanePos = cmdGetLanePosition();
    if (curPrimaryRoadId == "20/14to18/14") {
        if (curPrimaryEdge == lastEdge) {
            // 如果当前道路为需要记录的主要edge则记录距离路口点负距离
            double pos = lanePos - curPrimaryEdge->laneVector[0]->length;
            // title = "external_id"+"\t"+"time"+"\t"+"road_id"+"\t"+"record_road"+"\t"+"position";
            if (pos > -500) {
                if (lanePos != lastPos) {
                    srt->changeName(recordRoadId, title) << external_id
                            << simTime().dbl() << road_id << recordRoadId << pos
                            << srt->endl;
                }
            }
        } else {
            // 反之记录延伸辅道距离为正向离开路口点距离
            srt->changeName(recordRoadId, title) << external_id
                    << simTime().dbl() << road_id << recordRoadId << lanePos
                    << srt->endl;
        }
    } else if (road_id == "18/14to10/14") {
        double pos = lanePos + 30.84;
        if (pos < 100) {
            srt->changeName("20_14to18_14", title) << external_id
                    << simTime().dbl() << road_id << "20_14to18_14" << pos
                    << srt->endl;
        }
    }
    lastPos = lanePos;
}

void SMTMobility::setPreferredLaneIndex(uint8_t laneIndex) {
    preferredLaneIndex = laneIndex;
}

void SMTMobility::changeToPreferredLane(int laneIndex) {
    if (laneIndex != -1) {
        preferredLaneIndex = (uint8_t) laneIndex;
    }
    startChangeLane(preferredLaneIndex);
}

void SMTMobility::startChangeLane(uint8_t laneIndex, double delay) {
    laneChangeMsg = new cMessage("changeLane", laneIndex);
    scheduleAt(simTime() + updateInterval + delay, laneChangeMsg);
}

string SMTMobility::convertStrToRecordId(string id) {
    for (unsigned int strPos = 0; strPos < id.length();) {
        unsigned int offset = id.find('/', strPos + 1);
        if (offset != string::npos) {
            strPos = offset;
            id.replace(strPos, 1, "_");
        } else {
            break;
        }
    }
    for (unsigned int strPos = 0; strPos < id.length(); ++strPos) {
        unsigned int offset = id.find(':', strPos + 1);
        if (offset != string::npos) {
            strPos += offset;
            id.replace(strPos, 1, "x");
        } else {
            break;
        }
    }
    return id;
}

void SMTMobility::cmdSetNoOvertake() {
    getComIf()->setLaneChangeMode(external_id,
            SMTComInterface::LANEMODE_DISALLOW_OVERTAKE);
}

void SMTMobility::cmdChangeLane(uint8_t laneIndex, uint32_t duration) {
    getComIf()->changeLane(external_id, laneIndex, duration * 1000);
}

void SMTMobility::handleLaneChangeMsg(cMessage* msg) {
    uint8_t curLaneIndex = commandGetLaneIndex();
    uint8_t targetLaneIndex = laneChangeMsg->getKind();
    if (msg == laneChangeMsg) {
        if (isChangeAndHold || curLaneIndex != targetLaneIndex) {
            // 如果需要保持车道或者车道更换为成功则继续尝试
            if (curLaneIndex != targetLaneIndex) {
                // 仅在不在目标车道时进行更改车道的尝试
                cmdChangeLane((uint8_t) laneChangeMsg->getKind(),
                        laneChangeDuration);
            }
            scheduleAt(simTime() + laneChangeDuration + updateInterval,
                    laneChangeMsg);
        } else {
            // cancel and delete the message if lane changed successfully.
            cancelAndDelete(laneChangeMsg);
            laneChangeMsg = NULL;
        }
    } else {
        cancelAndDelete(msg);
    }
}

double SMTMobility::cmdGetLanePosition() {
    return getComIf()->getLanePosition(external_id);
}
