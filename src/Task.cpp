/* 
 * File:   Task.cpp
 * Author: vovan
 * 
 * Created on 10 Апрель 2013 г., 16:04
 */

#include "Task.h"
#include <math.h>
#include <boost/thread/mutex.hpp>
#include <iostream>
unsigned int Task::curId=0;
std::map<unsigned int,double> Task::result; 
std::map<unsigned int,boost::thread::id> Task::taskToId;
double func(const double v1) {
        return v1;
}

void Task::run() {
    double res = 0;
    double interval = (this->end - this->begin) / this->step;

    for (int i = 0; i < this->step - 1; ++i) {
        double a = begin + interval * i;
        double b = begin + interval * (i + 1);
        res += (func(a) + func(b)) * (b - a) / 2;
    }
    {
        boost::mutex::scoped_lock result_lock;
        Task::result.insert(std::make_pair(this->id,res));
        //std::cout<<"aaaaa"<<res<<std::endl;
    }
}
double Task::getResult(unsigned int _id){
    
    boost::mutex::scoped_lock result_lock;
    return Task::result.find(_id)->second;
   
}

Task::~Task() {
}

double Task::getBegin(){
    return this->begin;
}
double Task::getEnd(){
    return this->end;
}

std::map<unsigned int,double> Task::getAllResult(){
    return Task::result;
}
