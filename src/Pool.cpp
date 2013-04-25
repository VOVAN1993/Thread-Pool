#include "Pool.h"
#include "Task.h"
#include <iostream>
#include <algorithm>
typedef boost::mutex::scoped_lock scoped_lock;
typedef boost::shared_lock<boost::shared_mutex> ReadLock;
typedef boost::upgrade_lock<boost::shared_mutex> WriteLock;

bool Pool::removeTask(unsigned int _taskid) {
    WriteLock all_mutex(all_thread_mutex);
    WriteLock task_mutex(taskToIdMutex);
    for (std::set < std::pair < boost::shared_ptr<boost::thread>, bool> >::iterator it = allThreads.begin(); it != allThreads.end(); ++it) {
        if ((*it).first->get_id() == taskToID[_taskid]) {
            if (it->second == false) {
                changeWorkThreads(-1);
                (*it).first->interrupt();
                allThreads.erase(it);
            } else {
                boost::shared_ptr<boost::thread> tmp = (*it).first;
                allThreads.erase(it);
                taskToID.erase(_taskid);
                changeWorkThreads(-1);
                boost::shared_ptr<boost::thread> thr(new boost::thread(boost::bind(&Pool::worker2, this)));
                allThreads.insert(std::make_pair(thr, true));
                tmp->interrupt();
            }
            return true;
        }
    }
    return false;
}

bool Pool::findGeneral(boost::thread::id _thread_id) {
    ReadLock all_mutex(all_thread_mutex);
    for (std::set < std::pair < boost::shared_ptr<boost::thread>, bool> >::iterator it = allThreads.begin(); it != allThreads.end(); ++it) {
        if (it->first->get_id() == _thread_id && it->second == true) {
            return true;
        }
    }
    return false;
}

void Pool::remove_from_tasktoid(boost::thread::id _id) {

    WriteLock task(taskToIdMutex);
    for (std::map<unsigned int, boost::thread::id>::iterator it = taskToID.begin();
            it != taskToID.end();) {
        if (it->second == _id) {
            taskToID.erase(it);
        } else {
            ++it;
        }
    }

}

void Pool::remove_threads() {
    WriteLock all_mutex(all_thread_mutex);
//    std::stringstream str;
//    str << "Try remove id = " << boost::this_thread::get_id() << std::endl;
//    myPrint(str.str());
    for (std::set < std::pair < boost::shared_ptr<boost::thread>, bool> >::iterator it = allThreads.begin();
            it != allThreads.end(); ++it) {
        if ((*it).first->get_id() == boost::this_thread::get_id()) {
//            std::stringstream str;
//            str << "Exit sub thread id=" << (*it).first->get_id() << std::endl;
//            myPrint(str.str());
            allThreads.erase(it);
            break;
        }
    }
    return;
}

void Pool::worker2() {
    boost::shared_ptr<Task> task;
    std::stringstream str;
    while (true) {
        {
            scoped_lock queue_mutex(queueMutex);
            bool timed_wait = false;
            while (this->queueTask.size() == 0 && getIsInterrupt() == false) {
                if (findGeneral(boost::this_thread::get_id()) == true) {
                    this->qCond.wait(queueMutex);
                } else {
                    timed_wait = qCond.timed_wait(queue_mutex, boost::posix_time::seconds(timeOut + 5));

                    if (!timed_wait || getIsInterrupt()) {
                        remove_threads();
                        return;
                    }
                }
            }
            if (this->queueTask.size() == 0 && getIsInterrupt()) {
                if (findGeneral(boost::this_thread::get_id()) == false) {
                    remove_threads();
                }
                return;
            }
            task = queueTask.front();
            queueTask.pop();
            WriteLock task_id_m(taskToIdMutex);
            taskToID[task->getId()] = boost::this_thread::get_id();
            task_id_m.unlock();
            changeWorkThreads(1);
            queue_mutex.unlock();
        }
        str << "Task begin= " << task->getBegin() << " end=" << task->getEnd() << " id=" << task->getId() << boost::this_thread::get_id() << std::endl;
        myPrint(str.str());
        str.str(std::string());
        std::pair<unsigned int, double> tmp = task->run();
        {
            scoped_lock res_mutex(resultMutex);
            result.insert(tmp);
        }
        boost::this_thread::sleep(boost::posix_time::seconds(7));
        str << "Task id=" << task->getId() << " result=" << tmp.second << std::endl;
        myPrint(str.str());
        str.str(std::string());
        changeWorkThreads(-1);
    }
}

Pool::Pool(unsigned int _quantityHot = std::max((int) boost::thread::hardware_concurrency(), 1), unsigned int _timeOut = 5) : quantityHot(_quantityHot), timeOut(_timeOut) {
    {
        scoped_lock interruptMutex(interMutex);
        isInterrupt = false;
    }
    workThreads = 0;
    {
        WriteLock allTh_mutex(all_thread_mutex);
        for (int i = 0; i < this->getQuantityHotThreads(); i++) {
            boost::shared_ptr<boost::thread> thr(new boost::thread(boost::bind(&Pool::worker2, this)));
            allThreads.insert(std::make_pair(thr, true));
        }
        allTh_mutex.unlock();
    }
}

void Pool::addTask(double _begin, double _end, double _step) {
    {
        scoped_lock queue_mutex(this->queueMutex);
        {
            WriteLock allTh_mutex(all_thread_mutex);
            if (getWorkThreads() + queueTask.size() >= allThreads.size()) {
                boost::shared_ptr<boost::thread> thr(new boost::thread(boost::bind(&Pool::worker2, this)));
                allThreads.insert(std::make_pair(thr, false));
            }
            allTh_mutex.unlock();
        }

        this->queueTask.push(boost::shared_ptr<Task > (new Task(_begin, _end, _step)));
        this->qCond.notify_one();
        queue_mutex.unlock();
    }
}

void Pool::changeWorkThreads(int _a) {
    scoped_lock occ_mutex(workThreadsMutex);
    workThreads += _a;
}

Pool::~Pool() {
}

bool Pool::getIsInterrupt() {
    scoped_lock is_interrupt(interMutex);
    return isInterrupt;
}

unsigned int Pool::getWorkThreads() {
    scoped_lock work_threads_mutex(workThreadsMutex);
    return workThreads;
}

void Pool::exit() {
    {
        scoped_lock isInterrupt_mutex(interMutex);
        isInterrupt = true;
    }

    {
        scoped_lock queue_mutex(queueMutex);
        qCond.notify_all();
    }
    {
        WriteLock all_th_mutex(all_thread_mutex);
        all_th_mutex.unlock();
        myjoin_all();
        all_th_mutex.lock();
        allThreads.erase(allThreads.begin(), allThreads.end());
        all_th_mutex.unlock();
    }
}


void Pool::myjoin_all() {
    ReadLock all_thread(all_thread_mutex);
    for (std::set < std::pair < boost::shared_ptr<boost::thread>, bool> >::iterator it = allThreads.begin(); it != allThreads.end(); ++it) {
        (*it).first->join();
    }
}

void Pool::myPrint(std::string _str) {
    scoped_lock io(ioMutex);
    std::cout << _str;
}
