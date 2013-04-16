/* 
 * File:   Pool.h
 * Author: vovan
 *
 * Created on 15 Апрель 2013 г., 13:25
 */

#ifndef POOL_H
#define	POOL_H
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include "Worker.h"
#include "Task.h"
#include <queue>
class Worker;
class Pool {
public:
    Pool();
    Pool(unsigned int _quantityHot,unsigned int _timeOut);
    void addTask(double a,double b,double c);
    void removeTask();
    inline unsigned int getTimeOut(){
        return timeOut;
    }
    inline unsigned int getQuantityHotThreads(){
        return quantityHot;
    }
    virtual ~Pool();
private:
    bool findGeneral(boost::thread::id);
    void worker2();
    volatile bool isInterrupt;
    boost::mutex groupMutex;
    boost::mutex generalThreadMutex;
    boost::mutex allThreadMutex;
    boost::thread_group myGroup;
    boost::mutex ioMutex;
    boost::mutex queueMutex;
    boost::condition qCond;
    volatile int occupired;
    const unsigned int timeOut;
    const unsigned int quantityHot;
    std::queue<Task*> queueTask;
    std::vector<boost::thread *> generalThreads;
    std::vector<boost::thread *> allThreads;
    Pool(const Pool& orig);

};

#endif	/* POOL_H */

