#include "Pool.h"
#include "Task.h"
#include <iostream>
#include <exception>
typedef boost::mutex::scoped_lock scoped_lock;
typedef boost::shared_lock<boost::shared_mutex> ReadLock;
typedef boost::upgrade_lock<boost::shared_mutex> WriteLock;

void Pool::removeTask(unsigned int _taskid) {
    WriteLock all_mutex(all_tmp);
    WriteLock task(taskToIdMutex);
    for (std::set<std::pair<boost::shared_ptr<boost::thread>,bool> >::iterator it = allThreads.begin(); it != allThreads.end(); ++it) {
        if ((*it).first->get_id() == taskToID[_taskid]) {
            if(it->second==false){
                (*it).first->interrupt();
                allThreads.erase(it);
            }else{
                boost::shared_ptr<boost::thread> tmp = (*it).first;
                allThreads.erase(it);
                tmp->interrupt();
               // if(taskToID.find(tmp->get_id())!=taskToID.end()){
                taskToID.erase(_taskid);
                changeWorkThreads(-1);
//                }
                std::cout<<"TUT\n";
                boost::shared_ptr<boost::thread> thr(new boost::thread(boost::bind(&Pool::worker2, this)));
                allThreads.insert(std::make_pair(thr,true));
            }
            break;
        }
    }
}

bool Pool::findGeneral(boost::thread::id _thread_id) {
    ReadLock all_mutex(all_tmp);
    for (std::set<std::pair<boost::shared_ptr<boost::thread>,bool> >::iterator it = allThreads.begin(); it != allThreads.end(); ++it) {
        if( it->first->get_id() == _thread_id && it->second==true){
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
    //scoped_lock group_mutex(generalThreadMutex);
    WriteLock all_mutex(all_tmp);
    {
        scoped_lock io(ioMutex);
        std::cerr << "Try remove id = " << boost::this_thread::get_id() << std::endl;
    }
    for (std::set<std::pair<boost::shared_ptr<boost::thread>,bool> >::iterator it = allThreads.begin();
            it != allThreads.end(); ++it) {
        if ((*it).first->get_id() == boost::this_thread::get_id()) {
            //removeThread = *it;
            {
                scoped_lock io(ioMutex);
                std::cerr << "dellll = " << (*it).first->get_id() << std::endl;
            }
            //myGroup.remove_thread(*it);
            {
                scoped_lock io_mutex(ioMutex);
                std::cerr << "Exit sub thread id=" << (*it).first->get_id() << std::endl;
                io_mutex.unlock();
            }
            //remove_from_tasktoid(boost::this_thread::get_id());
            allThreads.erase(it);
            break;
        }
    }


//    all_mutex.unlock();
    return;
}

void Pool::worker2() {
    try{
        boost::shared_ptr<Task> task;
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
                    //bool isGeneral = false;
                    if (findGeneral(boost::this_thread::get_id()) == true) {
                        {
                            scoped_lock io_mutex(this->ioMutex);
                            std::cerr << "waitt general thread "<<boost::this_thread::get_id()<<"\n";
                        }
                        this->qCond.wait(queueMutex);
                    } else {
                        {
                            scoped_lock io_mutex(ioMutex);
                            std::cerr << "sub thread wait " << boost::this_thread::get_id() << "\n";
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
                    WriteLock task_id_m(taskToIdMutex);
                    taskToID[task->getId()] = boost::this_thread::get_id();
                }
                changeWorkThreads(1);
               //workThreads++;
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
            //workThreads--;
        }
    
    }catch(boost::thread_interrupted& e){
        std::cout<<" AAAAAAAAAAAAAA\n";
    }


}


Pool::Pool(unsigned int _quantityHot, unsigned int _timeOut) : quantityHot(_quantityHot), timeOut(_timeOut) {
    {
        scoped_lock l(interMutex);
            isInterrupt = false;
    }
    workThreads = 0;
    {
        std::cerr << "_cou=" << _quantityHot << std::endl;
        for (int i = 0; i < this->getQuantityHotThreads(); i++) {
            boost::shared_ptr<boost::thread> thr(new boost::thread(boost::bind(&Pool::worker2, this)));
            {
                scoped_lock io(ioMutex);
                std::cerr << thr->get_id() << " hello\n";
                io.unlock();
            }


            {
                WriteLock allTh_mutex(all_tmp);
                allThreads.insert(std::make_pair(thr,true));
                allTh_mutex.unlock();
            }
        }
        //group.unlock();
    }
}

void Pool::addTask(double _begin, double _end, double _step) {
    {
        scoped_lock queue_mutex(this->queueMutex);
        {
            WriteLock allTh_mutex(all_tmp);
            if (getWorkThreads() + queueTask.size() >= allThreads.size()) {
                scoped_lock io_mutex(ioMutex);
                boost::shared_ptr<boost::thread> thr(new boost::thread(boost::bind(&Pool::worker2, this)));
                allThreads.insert(std::make_pair(thr,false));
                
                std::cerr << "New Thread =  " << thr->get_id() << std::endl;
            }
            allTh_mutex.unlock();
        }

        this->queueTask.push(boost::shared_ptr<Task> (new Task(_begin, _end, _step)));
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
//        scoped_lock io(ioMutex);
//        std::cerr << "-----------\n";
//    }
//    {
//        ReadLock all_m(all_tmp);
//        scoped_lock io_mutex(ioMutex);
//        std::cerr << " all = " << allThreads.size() << std::endl;
//        all_m.unlock();
//    }

}

bool Pool::getIsInterrupt() {
    scoped_lock l(interMutex);
    return isInterrupt;
}

unsigned int Pool::getWorkThreads() {
    scoped_lock work_threads_mutex(workThreadsMutex);
    return workThreads;
}

void Pool::exit() {
    {
        scoped_lock l(interMutex);
        isInterrupt = true;
    }

    {
        scoped_lock queue_mutex(queueMutex);
        std::cerr << "STOP\n";
        qCond.notify_all();
        queue_mutex.unlock();
    }
    {
        WriteLock all_th_mutex(all_tmp);
        {
            scoped_lock io(ioMutex);
            std::cerr << "-----------\n";
            for (std::set<std::pair<boost::shared_ptr<boost::thread>,bool> >::iterator it = allThreads.begin(); it != allThreads.end(); ++it) {
                std::cerr << (*it).first->get_id() << std::endl;
            }
        }
        all_th_mutex.unlock();
        myjoin_all();
        all_th_mutex.lock();
        allThreads.erase(allThreads.begin(),allThreads.end());
        all_th_mutex.unlock();
    }




}

int Pool::getAll() {
    ReadLock all_mutex(all_tmp);
    return allThreads.size();
}

void Pool::myjoin_all() {
    ReadLock t(all_tmp);
    for ( std::set<std::pair<boost::shared_ptr<boost::thread>,bool> >::iterator it = allThreads.begin(); it != allThreads.end();++it){
        (*it).first->join();
    }
    t.unlock();
}
