/* 
 * File:   Pool.cpp
 * Author: vovan
 * 
 * Created on 15 Апрель 2013 г., 13:25
 */
#include "Pool.h"
#include "Task.h"
#include <iostream>
typedef boost::mutex::scoped_lock scoped_lock;
typedef boost::shared_lock<boost::shared_mutex> ReadLock;
typedef boost::upgrade_lock<boost::shared_mutex> WriteLock;

void Pool::getID(unsigned int _taskid){
    ReadLock all_mutex(all_tmp);
    scoped_lock t(taskToIdMutex);
    //scoped_lock io(ioMutex);
    for(std::vector<boost::thread*>::iterator it= allThreads.begin();it!=allThreads.end();++it){
        if((*it)->get_id()==taskToID[_taskid]){
            (*it)->interrupt();
            allThreads.erase(it);
            break;
        }
    }
    
    //all_mutex.unlock();
    //std::cout<< "TUT "<<taskToID[_taskid]<<std::endl;
}
bool Pool::findGeneral(boost::thread::id _thread_id) {
    scoped_lock general_mutex(generalThreadMutex);
    for (int i = 0; i < generalThreads.size(); ++i) {
        if (generalThreads[i]->get_id() == _thread_id) {
            return true;
        }
    }
    general_mutex.unlock();
    return false;
}
void Pool::remove_from_tasktoid(boost::thread::id _id){
    
    scoped_lock t(taskToIdMutex);
    for(std::map<unsigned int,boost::thread::id>::iterator it=taskToID.begin();
            it!=taskToID.end();){
        if(it->second==_id){
            taskToID.erase(it);
        }else{
            ++it;
        }
    }
    
}
void Pool::remove_threads() {
    //scoped_lock group_mutex(generalThreadMutex);
    WriteLock all_mutex(all_tmp);
    {
        scoped_lock io(ioMutex);
        std::cerr<<"Try remove id = "<<boost::this_thread::get_id()<<std::endl;
    }
    for (std::vector<boost::thread*>::iterator it = allThreads.begin();
            it != allThreads.end(); ++it) {
        {
            scoped_lock io(ioMutex);
            std::cerr<<"compare remove this_id = "<<boost::this_thread::get_id()<<"del_id="<<(*it)->get_id()<<std::endl;
        }
        if ((*it)->get_id() == boost::this_thread::get_id()) {
            //removeThread = *it;
            {
            scoped_lock io(ioMutex);
            std::cerr<<"dellll = "<<(*it)->get_id()<<std::endl;
            }
            //myGroup.remove_thread(*it);
            {
                scoped_lock io_mutex(ioMutex);
                std::cerr << "Exit sub thread id=" << (*it)->get_id() << std::endl;
                io_mutex.unlock();
            }
            remove_from_tasktoid(boost::this_thread::get_id());
            allThreads.erase(it);
            break;
        }
    }


    all_mutex.unlock();
    {
        std::cerr << "asdfsdfsdf\n";
        scoped_lock io_mutex(ioMutex);
        //std::cerr << "size=" << myGroup.size() << std::endl;
        io_mutex.unlock();
    }
    //group_mutex.unlock();

    return;
}

void Pool::worker2() {

    Task *task = NULL;
    while (true) {
        //boost::this_thread::sleep(boost::posix_time::seconds(5));
        {
            scoped_lock io_mutex(this->ioMutex);
            std::cerr << "enter id= " << boost::this_thread::get_id() << std::endl;
            io_mutex.unlock();
        }

        {
            scoped_lock queue_mutex(queueMutex);
            bool timed_wait = false;
            while (this->queueTask.size() == 0 && getIsInterrupt() == false) {
                bool isGeneral = false;
                if (findGeneral(boost::this_thread::get_id()) == true) {
                    {
                        scoped_lock io_mutex(this->ioMutex);
                        std::cerr << "waitt general thread\n";
                    }
                    this->qCond.wait(queueMutex);
                } else {
                    {
                        scoped_lock io_mutex(ioMutex);
                        std::cerr << "sub thread wait\n";
                        io_mutex.unlock();
                    }
                    timed_wait = qCond.timed_wait(queue_mutex, boost::posix_time::seconds(timeOut + 5));
                    {
                        scoped_lock io_mutex(ioMutex);
                        std::cerr << "After sub wait timed_wait = " << timed_wait << " " << getIsInterrupt() << std::endl;
                        io_mutex.unlock();
                    }

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
                scoped_lock io_mutex(ioMutex);
                std::cerr << "exit id=" << boost::this_thread::get_id() << std::endl;
                return;
            }
            task = queueTask.front();
            queueTask.pop();
            {
                scoped_lock task_id_m(taskToIdMutex);
                taskToID[task->getId()]=boost::this_thread::get_id();
            }
            changeWorkThreads(1);
            queue_mutex.unlock();
        }
        {
            scoped_lock io_mutex(ioMutex);
            std::cerr << "Task begin= " << task->getBegin() << " end=" << task->getEnd() << " id=" << task->getId() << std::endl;
        }
        std::pair<unsigned int, double> tmp = task->run();
        {
            scoped_lock res_mutex(resultMutex);
            result.insert(tmp);
        }
        boost::this_thread::sleep(boost::posix_time::seconds(7));
        {
            scoped_lock io_mutex(ioMutex);
            std::cerr << "Task begin= " << task->getBegin() << " end=" << task->getEnd() << " id=" << task->getId() << " result=" << tmp.second << std::endl;
        }
        changeWorkThreads(-1);
    }



}

Pool::Pool() : quantityHot(std::max((int) boost::thread::hardware_concurrency(), 1)), timeOut(5) {
    isInterrupt = false;
    workThreads = 0;
    {
        //scoped_lock group(this->groupMutex);
        for (int i = 0; i < quantityHot; i++) {
            boost::thread *thr = this->myGroup.create_thread(boost::bind(&Pool::worker2, this));
            {
                scoped_lock io(ioMutex);
                std::cerr << thr->get_id() << " hello/n";
                io.unlock();
            }
            {
                scoped_lock genTh_mutex(generalThreadMutex);
                generalThreads.push_back(thr);
                genTh_mutex.unlock();
            }
            {
                WriteLock allTh_mutex(all_tmp);
                allThreads.push_back(thr);
                allTh_mutex.unlock();
            }
        }
        //groupMutex.unlock();
    }
}

Pool::Pool(unsigned int _quantityHot, unsigned int _timeOut) : quantityHot(_quantityHot), timeOut(_timeOut) {
    isInterrupt = false;
    workThreads = 0;
    {
        //scoped_lock group(this->groupMutex);
        std::cerr << "_cou=" << _quantityHot << std::endl;
        for (int i = 0; i < this->getQuantityHotThreads(); i++) {
            boost::thread *thr = new boost::thread(boost::bind(&Pool::worker2, this)); 
            //boost::thread *thr = this->myGroup.create_thread(boost::bind(&Pool::worker2, this));
            {
                scoped_lock io(ioMutex);
                std::cerr << thr->get_id() << " hello\n";
                io.unlock();
            }
            {
                scoped_lock genTh_mutex(generalThreadMutex);
                generalThreads.push_back(thr);
            }

            {
                WriteLock allTh_mutex(all_tmp);
                allThreads.push_back(thr);
                allTh_mutex.unlock();
            }
        }
        //group.unlock();
    }
}

void Pool::addTask(double a, double b, double c) {
    {
        scoped_lock queue_mutex(this->queueMutex);
        {
        //    scoped_lock group_mutex(groupMutex);
            WriteLock allTh_mutex(all_tmp);
            if (getWorkThreads() + queueTask.size() >= allThreads.size()) {
                scoped_lock io_mutex(ioMutex);
                boost::thread *thr = new boost::thread(boost::bind(&Pool::worker2, this));
                //boost::thread *tmp = myGroup.create_thread(boost::bind(&Pool::worker2, this));
                allThreads.push_back(thr);
                std::cerr << "New Thread =  " << thr->get_id() << std::endl;
            }
            
            allTh_mutex.unlock();
        //    group_mutex.unlock();
        }

        this->queueTask.push(new Task(a, b, c));
        this->qCond.notify_one();
        queue_mutex.unlock();
    }
}

void Pool::changeWorkThreads(int _a) {
    scoped_lock occ_mutex(workThreadsMutex);
    workThreads += _a;
}

Pool::~Pool() {
    //    {
    //        scoped_lock io_mutex(ioMutex);
    //        std::cerr << "myGr = " << myGroup.size() << " all = " << getAll() << std::endl;
    //    }
    {
        //scoped_lock group_mutex(groupMutex);
        {
            scoped_lock io(ioMutex);
            std::cerr << "-----------\n";
            //std::cerr << myGroup.size();

        }
    }
    //this->myGroup.join_all();
    myjoin_all();
    {
        ReadLock all_m(all_tmp);
        scoped_lock io_mutex(ioMutex);
        std::cerr <<  " all = " << allThreads.size() << std::endl;
        all_m.unlock();
    }
}

bool Pool::getIsInterrupt() {
    scoped_lock inter_mutex(isInterrup_mutex);
    return isInterrupt;
}

unsigned int Pool::getWorkThreads() {
    scoped_lock work_threads_mutex(workThreadsMutex);
    return workThreads;
}

void Pool::exit() {
    {
        scoped_lock inter_mutex(isInterrup_mutex);
        isInterrupt = true;
    }

    {
        scoped_lock queue_mutex(queueMutex);
        std::cerr << "STOP\n";
        qCond.notify_all();
        queue_mutex.unlock();
    }
    {
        ReadLock all_th_mutex(all_tmp);
        {
            scoped_lock io(ioMutex);
            std::cerr << "-----------\n";
            for (std::vector<boost::thread*>::iterator it = allThreads.begin(); it != allThreads.end(); ++it) {
                std::cerr << (*it)->get_id() << std::endl;
            }
        }
        all_th_mutex.unlock();
    }


}

int Pool::getAll() {
    ReadLock all_mutex(all_tmp);
    return allThreads.size();
}

void Pool::myjoin_all(){
    //boost::shared_lock<shared_mutex> guard(m);
    //scoped_lock m(allThreadMutex);
    ReadLock t(all_tmp);
            for(std::vector<boost::thread*>::iterator it=allThreads.begin(),end=allThreads.end();
                it!=end;
                ++it)
            {
                (*it)->join();
            }
    t.unlock();
}
