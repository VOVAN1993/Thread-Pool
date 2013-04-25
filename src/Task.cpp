/* 
 * File:   Task.cpp
 * Author: vovan
 * 
 * Created on 10 Апрель 2013 г., 16:04
 */

#include "Task.h"
unsigned int Task::curId=0;
double func(const double v1) {
        return v1;
}

std::pair<unsigned int,double> Task::run() {
    double res = 0;
    double interval = (this->end - this->begin) / this->step;

    for (int i = 0; i < this->step - 1; ++i) {
        double a = begin + interval * i;
        double b = begin + interval * (i + 1);
        res += (func(a) + func(b)) * (b - a) / 2;
    }
    return std::make_pair(id,res);
}


Task::~Task() {
}

double Task::getBegin(){
    return this->begin;
}
double Task::getEnd(){
    return this->end;
}
