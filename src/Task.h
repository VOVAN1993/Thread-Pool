/* 
 * File:   Task.h
 * Author: vovan
 *
 * Created on 10 Апрель 2013 г., 16:04
 */

#ifndef TASK_H
#define	TASK_H
#include <utility>
class Task {
public:
    Task(const double _begin=0,const double _end=0,const int _step=0):begin(_begin),end(_end),step(_step),id(++curId){};
    std::pair<unsigned int,double> run();
    inline unsigned int getId(){return id;};
    virtual ~Task();
    double getBegin();
    double getEnd();
private:
    Task(const Task& orig);
    static unsigned int curId;
    const double begin;
    const double end;
    const int step;
    const unsigned id;

};

#endif	/* TASK_H */

