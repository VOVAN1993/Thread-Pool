/* 
 * File:   Task.h
 * Author: vovan
 *
 * Created on 10 Апрель 2013 г., 16:04
 */

#ifndef TASK_H
#define	TASK_H
#include <map>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <utility>
class Task {
public:
    Task(const double _begin,const double _end,const int _step):begin(_begin),end(_end),step(_step),id(++curId){};
    Task():begin(0),end(0),step(0),id(++curId){};
    void run();
    inline unsigned int getId(){return id;};
    Task(const Task& orig);
    virtual ~Task();
    void setResult(double _result);
    static double getResult(unsigned int);
    double getBegin();
    double getEnd();
    static std::map<unsigned int,double> getAllResult();
private:
    static unsigned int curId;
    static std::map<unsigned int,double> result;
    const double begin;
    const double end;
    const int step;
    const unsigned id;

};

#endif	/* TASK_H */

