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

#include "SMTPhaseSegment.h"

SMTPhaseSegment::~SMTPhaseSegment() {
}

void SMTPhaseSegment::setState(double start, double end, string value) {
    if (end <= start) {
        std::cout << "Error: end time mush bigger than start time."
                << std::endl;
    }
    // 设置segment的某一段为指定value
    // 合并与前方状态一致的节点
    // 寻找起始位置
    if (segIt == states.end() || segIt->time > start) {
        segIt = states.begin();
        preState = "x";
    }
    // 跳过小于start时间点的节点
    while (segIt != states.end() && segIt->time < start) {
        preState = segIt->value;
        segIt++;
    }
    // 修改start节点
    string originState = preState; // 用于记录end时间后原始的状态
    if (segIt != states.end() && segIt->time == start) {
        // 时间相等进行修改
        // 记录原始后续状态(理论上原始后续状态肯定与前方状态不同)
        // (因为相同的节点应该被删除)
        originState = segIt->value;
        if (preState != value) {
            // 若start后状态与之前状态不一致,则修改当前节点
            // 修改节点后续状态
            segIt->value = value;
            // 同步迭代器与preState状态,迭代器后移
            preState = value;
            segIt++;
        } else {
            // 若修改后的状态与start之前状态一致,则删除该节点
            // 此时preState与originState均未改变
            segIt = states.erase(segIt);
        }
    } else {
        // 如果segIt为end()或者大于start的节点,且之前状态与value不一致
        // 则需要新建节点,反之不需要修改
        if (preState != value) {
            State state;
            state.time = start;
            state.value = value;
            states.insert(segIt, state);
            // 记录原始后续状态
            originState = preState;
            // 同步迭代器与preState状态,
            preState = value;
        }
    }
    // 删除所有start之后,end之前的状态
    while (segIt != states.end() && segIt->time < end) {
        // 保存后续状态
        originState = segIt->value;
        segIt = states.erase(segIt);
    }
    // 修改end节点
    if (segIt != states.end() && segIt->time == end) {
        // 若存在时间为end的节点
        // end后的状态与value状态一致,则删除该节点
        // 反之不做修改
        if (preState == segIt->value) {
            // 此时preState未改变
            segIt = states.erase(segIt);
        }
    } else {
        // 若segIt为end()或大于end的节点,且end前后状态不同
        // 则新建节点,若前后状态一致,则不做改变
        if (preState != originState) {
            State state;
            state.time = end;
            state.value = originState;
            states.insert(segIt, state);
            // 为便于连续修改segment状态,迭代器返回end时刻
            segIt--;
            // 返回后preState状态未改变
        }
    }
    updateHeadTailPeriod();
}

void SMTPhaseSegment::resetState(list<double> &segLens, list<string> &values,
        double start) {
    if (segLens.size() != values.size()) {
        std::cout << "unmatched segLens and values." << std::endl;
    }
    // 重置状态线段
    states.clear();
    segIt = states.begin();
    double begin = start;
    list<double>::iterator lenIt = segLens.begin();
    list<string>::iterator valIt = values.begin();
    // 依此设置各状态段落
    while (lenIt != segLens.end() && valIt != values.end()) {
        double end = begin + *lenIt;
        setState(begin, end, *valIt);
        begin = end;
        lenIt++;
        valIt++;
    }
}

bool SMTPhaseSegment::moveCertainStateAHead(string value) {
    // 将特定value移动至最前方
    // mCSAHead-b: a->b->c ==>> b->c->a
    // 算法分为两步:
    // 1. 截取b前方队列
    // 2. 将截取的队列插入末位状态
    // 队列不能为空
    if (states.begin() == states.end()) {
        std::cout << "Error: Cannot move state in empty segment" << std::endl;
        return false;
    }
    list<State> t;
    // 记录结尾状态时间,用于更新截取的队列
    list<State>::iterator endIt = states.end();
    --endIt;
    double end = endIt->time;
    // 获得结尾状态,用于判定节点是否需要合并
    if (endIt == states.begin()) {
        debugMoreThanOneNode();
        return false;
    }
    --endIt;
    string lastState = endIt->value;
    ++endIt;
    segIt = states.begin();
    // 记录开始状态时间,用于更新截取的队列
    double start = segIt->time;
    double period = end - start;
    while (segIt != states.end()) {
        // 截取节点,直到状态为value的节点
        if (segIt->value != value) {
            t.push_back(*segIt);
            segIt = states.erase(segIt);
        } else {
            // 更新移动后线段的开始时间
            start = segIt->time;
            // 更新t列表时间状态
            for (list<State>::iterator it = t.begin(); it != t.end(); ++it) {
                it->time += period;
            }
            // 若结尾状态与t列表开头状态一致,则进行合并,删除t第一个节点
            if (t.begin()->value == lastState) {
                t.pop_front();
            }
            // 合并列表,并修改最后节点位置
            double offset = period - end + start;
            states.insert(endIt, t.begin(), t.end());
            endIt->time += offset;
            break;
        }
    }
    if (segIt == states.end()) {
        states.swap(t);
        segIt = states.begin();
        preState = "x";
        std::cout << "Error: Cannot move the state not in the segment"
                << std::endl;
        return false;
    }
    // 重设segIt与preState
    // 若segment包含状态为value的节点,则segIt指向节点segment.begin()
    // 反之,指向end()?.因此,此处重设为begin,并对应修改preState
    segIt = states.begin();
    preState = "x";
    updateHeadTailPeriod();
    return true;
}

void SMTPhaseSegment::debugPrint() {
    std::cout << std::endl << "debug: print the segment content." << std::endl;
    std::cout << "head:" << head << " " << "tail:" << tail << " " << "period:"
            << period << std::endl;
    for (list<State>::iterator it = states.begin(); it != states.end();) {
        std::cout << it->time << ":" << it->value;
        ++it;
        if (it != states.end()) {
            std::cout << ", ";
        }
    }
    std::cout << std::endl;
}

double SMTPhaseSegment::getHead() {
    if (states.size() < 2) {
        debugMoreThanOneNode();
        return -1;
    }
    return states.front().time;
}

double SMTPhaseSegment::getTail() {
    if (states.size() < 2) {
        debugMoreThanOneNode();
        return -1;
    }
    return states.back().time;
}

double SMTPhaseSegment::getPeriod() {
    if (states.size() < 2) {
        debugMoreThanOneNode();
        return -1;
    }
    return states.back().time - states.front().time;
}

void SMTPhaseSegment::updateHeadTailPeriod() {
    head = getHead();
    tail = getTail();
    period = getPeriod();
}

void SMTPhaseSegment::debugMoreThanOneNode() {
    std::cout << "Error: Segment must have more than one node" << std::endl;
}
