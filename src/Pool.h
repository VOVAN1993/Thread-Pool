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
#include "Task.h"
#include <utility>
#include <queue>
#include <map>
#include <atomic>
#include <boost/interprocess/detail/atomic.hpp>
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
    void exit();
    int getAll();
    void removeTask(unsigned int _taskid);
private:
    void remove_from_tasktoid(boost::thread::id _id);
    void myjoin_all();
    void remove_threads();
    boost::mutex resultMutex;
    std::map<unsigned int,double> result;
    bool findGeneral(boost::thread::id);
    std::map<unsigned int,boost::thread::id> taskToID;
    void changeWorkThreads(int);
    void worker2();
    volatile bool isInterrupt;
    bool getIsInterrupt();
    unsigned int getWorkThreads();
    boost::shared_mutex taskToIdMutex;
    boost::shared_mutex all_tmp;
    boost::mutex ioMutex;
    boost::mutex queueMutex;
    boost::condition qCond;
    //unsigned int workThreads;
    std::atomic<unsigned int> workThreads;
    boost::mutex workThreadsMutex;
    const unsigned int timeOut;
    const unsigned int quantityHot;
    std::queue<Task*> queueTask;
    std::vector<std::pair<boost::thread *,bool> > allThreads;
    Pool(const Pool& orig);

};

#endif	/* POOL_H */

