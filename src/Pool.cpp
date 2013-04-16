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

bool Pool::findGeneral(boost::thread::id _thread_id) {
    scoped_lock general_mutex(generalThreadMutex);
    for (int i = 0; i < generalThreads.size(); ++i) {
        if (generalThreads[i]->get_id() == _thread_id) {
            return true;
        }
    }
    return false;
}

void Pool::worker2() {
    
    Task *task = NULL;
    while (true) {
        //boost::this_thread::sleep(boost::posix_time::seconds(5));
        {
            scoped_lock io_mutex(this->ioMutex);
            std::cout << "enter id= " << boost::this_thread::get_id() << std::endl;
        }

        {
            scoped_lock queue_mutex(queueMutex);
            bool timed_wait=false;
            while (this->queueTask.size() == 0 && getIsInterrupt() == false) {
                bool isGeneral = false;
                if (findGeneral(boost::this_thread::get_id()) == true) {
                    {
                        scoped_lock io_mutex(this->ioMutex);
                        std::cout << "waitt general thread\n";
                    }
                    this->qCond.wait(queueMutex);
                } else {
                    timed_wait = qCond.timed_wait(queue_mutex,boost::posix_time::seconds(timeOut));
                    if (!timed_wait || getIsInterrupt()){
                        boost::thread *removeThread;
                        {
                            scoped_lock all_mutex(allThreadMutex);
                            for(std::vector<boost::thread*>::iterator it = allThreads.begin();
                                    it!=allThreads.end();++it){
                                if((*it)->get_id()==boost::this_thread::get_id()){
                                    removeThread=*it;
                                    allThreads.erase(it);
                                    break;
                                }
                            }
                        }
                        {
                            scoped_lock io_mutex(ioMutex);
                            std::cout<<"Exit sub thread id="<<removeThread->get_id()<<std::endl;
                        }
                        {
                            scoped_lock group_mutex(groupMutex);
                            myGroup.remove_thread(removeThread);
                        }
                        return ;    
                    }
                }
            }
            if (this->queueTask.size() == 0 && getIsInterrupt()) {
                scoped_lock io_mutex(ioMutex);
                std::cout << "exit id=" << boost::this_thread::get_id() << std::endl;
                return;
            }
            task = queueTask.front();
            queueTask.pop();
            changeWorkThreads(1);
        }
        {
            scoped_lock io_mutex(ioMutex);
            std::cout << "Task begin= " << task->getBegin() << " end=" << task->getEnd() << " id=" << task->getId() << std::endl;
        }
        std::pair<unsigned int,double> tmp = task->run();
        {
            scoped_lock res_mutex(resultMutex);
            result.insert(tmp);
        }
        boost::this_thread::sleep(boost::posix_time::seconds(5));
        {
            scoped_lock io_mutex(ioMutex);
            std::cout << "Task begin= " << task->getBegin() << " end=" << task->getEnd() << " id=" << task->getId() << " result=" << tmp.second << std::endl;
        }
        changeWorkThreads(-1);
    }



}

Pool::Pool() : quantityHot(std::max((int) boost::thread::hardware_concurrency(), 1)), timeOut(5) {
    isInterrupt = false;
    workThreads=0;
    //std::cout<<"size="<<queueTask.size();
    //setQuantityHotThreads(std::max((int) boost::thread::hardware_concurrency(), 1));
    {
        scoped_lock group(this->groupMutex);
        for (int i = 0; i < quantityHot; i++) {
            boost::thread *thr = this->myGroup.create_thread(boost::bind(&Pool::worker2, this));
            {
                scoped_lock genTh_mutex(generalThreadMutex);
                generalThreads.push_back(thr);
            }
            {
                scoped_lock allTh_mutex(allThreadMutex);
                allThreads.push_back(thr);
            }
        }
    }
}

Pool::Pool(unsigned int _quantityHot, unsigned int _timeOut) : quantityHot(_quantityHot), timeOut(_timeOut) {
    isInterrupt = false;
    workThreads=0;
    {
        scoped_lock group(this->groupMutex);
        std::cout << "_cou=" << _quantityHot << std::endl;
        for (int i = 0; i < this->getQuantityHotThreads(); i++) {
            boost::thread *thr = this->myGroup.create_thread(boost::bind(&Pool::worker2, this));
            //generalThreads.push_back(thr);
            {
                scoped_lock genTh_mutex(generalThreadMutex);
                generalThreads.push_back(thr);
            }
            {
                scoped_lock allTh_mutex(allThreadMutex);
                allThreads.push_back(thr);
            }
        }
    }
}

void Pool::addTask(double a, double b, double c) {
    {
        scoped_lock queue_mutex(this->queueMutex);
        {
            scoped_lock io_mutex(ioMutex);
            std::cout << "queue size"<<queueTask.size() << " " << "count Hot="<<this->getQuantityHotThreads() << std::endl;
        }          
        {
            
          scoped_lock allTh_mutex(allThreadMutex);
                if (getWorkThreads()+queueTask.size()>=allThreads.size()) {
                    scoped_lock io_mutex(ioMutex);
                    
                    boost::thread *tmp = myGroup.create_thread(boost::bind(&Pool::worker2, this));
                    allThreads.push_back(tmp);
                    std::cout << "New Thread =  " << tmp->get_id()<<std::endl;
                }
        }

        this->queueTask.push(new Task(a, b, c));
        this->qCond.notify_one();
    }
}

void Pool::changeWorkThreads(int _a){
    scoped_lock occ_mutex(workThreadsMutex);
    workThreads+=_a;
}
Pool::~Pool() {
    this->myGroup.join_all();
    {
        scoped_lock io_mutex(ioMutex);
        std::cout<<"myGr = "<<myGroup.size()<<" all = "<<allThreads.size()<<std::endl;
    }
}
bool Pool::getIsInterrupt(){
    scoped_lock inter_mutex(isInterrup_mutex);
    return isInterrupt;
}
unsigned int Pool::getWorkThreads(){
    scoped_lock work_threads_mutex(workThreadsMutex);
    return workThreads;
}
void Pool::exit(){
    {
        scoped_lock inter_mutex(isInterrup_mutex);
        isInterrupt = true;
    }
    {
        scoped_lock queue_mutex(queueMutex);
        qCond.notify_all();
    }
}


