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
#include <set>
#include <sstream>
class Worker;
class Pool {
public:
    Pool(unsigned int _quantityHot,unsigned int _timeOut);
    void addTask(double a,double b,double c);
    inline unsigned int getTimeOut(){
        return timeOut;
    }
    inline unsigned int getQuantityHotThreads(){
        return quantityHot;
    }
    virtual ~Pool();
    void exit();
    bool removeTask(unsigned int _taskid);
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
    bool isInterrupt;
    boost::mutex interMutex;
    bool getIsInterrupt();
    unsigned int getWorkThreads();
    boost::shared_mutex taskToIdMutex;
    boost::shared_mutex all_thread_mutex;
    boost::mutex ioMutex;
    boost::mutex queueMutex;
    boost::condition qCond;
    unsigned int workThreads;
    boost::mutex workThreadsMutex;
    const unsigned int timeOut;
    const unsigned int quantityHot;
    std::queue<boost::shared_ptr<Task> > queueTask;
    std::set<std::pair<boost::shared_ptr<boost::thread> ,bool> > allThreads;
    Pool(const Pool& orig);
    void myPrint(std::string _str);

};

#endif	/* POOL_H */
