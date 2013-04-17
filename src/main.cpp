/* 
 * File:   main.cpp
 * Author: vovan
 *
 * Created on 8 Апрель 2013 г., 23:47
 */

#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <vector>
#include <queue>
#include "Task.h"
#include "func.h"
#include "Pool.h"
#include <map>
using namespace std;
typedef boost::mutex::scoped_lock scoped_lock;
boost::mutex qMutex;
boost::mutex io_Mutex;
boost::condition qCond;
boost::mutex countMutex;
boost::mutex allMutex;
boost::mutex vecThMutex;
boost::mutex groupMutex;
boost::mutex TaskToIdMutex;
queue<Task*> q;
vector<boost::thread*> vec;
boost::thread_group myGroup;
volatile bool isInterrupt = false;
/*
 * 
 */
int occupied = 0;
int all = 2;

int getAll() {
    scoped_lock l(allMutex);
    return all;
}

int incAll(int a) {
    scoped_lock l(allMutex);
    all += a;
}

void printSizeQ() {
    scoped_lock io(io_Mutex);
    scoped_lock qq(qMutex);
    //cout<<"sizeQ= "<<q.size()<<'\n';
    bool b = q.size() == 0 && isInterrupt;
    cout << "inter=" << (int) isInterrupt << " " << b << "\n";
}

void printTask(int i) {
    scoped_lock io(io_Mutex);
    cout << "task= " << i << "\n";
}

void printEndTask(int i) {
    scoped_lock io(io_Mutex);
    cout << "end task= " << i << "\n";
}

int getCount() {
    scoped_lock c(countMutex);
    return occupied;
}

void changeCount(int diff) {
    scoped_lock c(countMutex);
    occupied += diff;
}

void worker() {
    while (true) {
        Task* k;
        {
            scoped_lock l(qMutex);
            cout << "enter  id= " << boost::this_thread::get_id() << endl;
            while (q.size() == 0 && !isInterrupt) {
                bool isTwo = false;
                {
                    scoped_lock io(io_Mutex);
                    cout << "wait  id= " << boost::this_thread::get_id() << endl;
                }
                {
                    scoped_lock vecM(vecThMutex);
                    if (vec[0]->get_id() == boost::this_thread::get_id() || vec[1]->get_id() == boost::this_thread::get_id()) {
                        scoped_lock io(io_Mutex);
                        isTwo = true;
                    }
                }
                if (isTwo == true) {
                    {
                        scoped_lock io(io_Mutex);
                        cout << "wait main thread" << endl;
                    }
                    qCond.wait(qMutex);
                } else {
                    {
                        scoped_lock io(io_Mutex);
                        cout << "wait NO main thread" << endl;
                    }
                    bool b = qCond.timed_wait(qMutex, boost::posix_time::seconds(5));
                    if (b == false) {
                        {
                            scoped_lock io(io_Mutex);
                            cout << "exit after wait" << endl;
                        }
                        incAll(-1);
                        boost::thread *tmp;
                        {
                            scoped_lock vecM(vecThMutex);

                            for (vector<boost::thread*>::iterator i = vec.begin(); i != vec.end();
                                    ++i) {
                                if ((*i)->get_id() == boost::this_thread::get_id()) {
                                    tmp = *i;
                                }
                            }
                            scoped_lock io(io_Mutex);
                            //cout<<boost::this_thread::get_id()<<" "<<tmp->get_id()<<endl;
                        }
                        {
                            {
                                scoped_lock io(io_Mutex);
                                cout << tmp->get_id() << endl;
                            }
                            scoped_lock grM(groupMutex);
                            myGroup.remove_thread(tmp);
                        }
                        return;
                    }
                }
            }
            if (q.size() == 0 && isInterrupt) {
                scoped_lock io(io_Mutex);
                cout << "exit id=" << boost::this_thread::get_id() << endl;
                //qCond.notify_all();
                return;
            }
            k = q.front();
            q.pop();
        }
        //printTask(0);
        //boost::this_thread::sleep(boost::posix_time::seconds(5));
//        {
//            scoped_lock mapMutex(TaskToIdMutex);
//            Task::taskToId[k->getId()]=boost::this_thread::get_id();
//        }
//        //double tmp = k->run();
//        boost::this_thread::sleep(boost::posix_time::seconds(4));
//        {
//            scoped_lock mapMutex(TaskToIdMutex);
//            Task::taskToId.erase(k->getId());
//        }
//        {
//            scoped_lock lock(io_Mutex);
//            cout << "id=" << k->getId() << " " << "begin=" << k->getBegin() << " end=" << k->getEnd() << " result=" << Task::getResult(k->getId()) << " end" << endl;
//        }
//        changeCount(-1);
        //printEndTask(1);
        //printSizeQ();
    }
}

int main(int argc, char** argv) {
//    int col = std::max((int) boost::thread::hardware_concurrency(), 1);
//
//    for (int i = 0; i < col; i++) {
//        boost::thread *thr = myGroup.create_thread(boost::bind(&worker));
//        {   
//            scoped_lock l(vecThMutex);
//            vec.push_back(thr);
//        }
//
//    }
//    int a = 0, begin, end, step;
//    string command;
//    while (1) {
//        cin >> command;
//        if (command == "exit") {
//            break;
//        } else if(command=="del"){
//            unsigned int id;
//            cin>>id;
//            {
//                scoped_lock mmap(TaskToIdMutex);
//                std::map<unsigned int,boost::thread::id>::iterator it = Task::taskToId.find(id);
//                if(it!=Task::taskToId.end()){
//                    boost::thread* tmp;
//                    scoped_lock io(io_Mutex);
//                    {
//                        scoped_lock vecM(vecThMutex);
//                        
//                        for(unsigned int i=0;i<vec.size();i++){
//                            if(vec.at(i)->get_id()==it->second){
//                                tmp=vec.at(i);
//                            }
//                        }
//                    }
//                    tmp->interrupt();
//                    cout<<"Yes id\n";
//                }else{
//                    scoped_lock io(io_Mutex);
//                    cout<<"No id\n";
//                }
//            }
//        }else if (command == "int") {
//            cin >> begin >> end >> step;
//            scoped_lock lock(qMutex);
//            q.push(new Task(begin, end, step));
//            if (getCount() + 1 > getAll()) {
//                {
//                    incAll(1);
//                    scoped_lock w(io_Mutex);
//                    //cout << "getCount= " << getCount() << "\n";
//                }
//                boost::thread *tmp = myGroup.create_thread(boost::bind(&worker));
//                {
//                    scoped_lock l(vecThMutex);
//                    vec.push_back(tmp);
//                }
//                qCond.notify_one();
//            } else {
//                qCond.notify_one();
//            }
//            changeCount(1);
//        }
//    }
//    isInterrupt = true;
//    {
//        scoped_lock l(qMutex);
//        qCond.notify_all();
//    }
//
//    myGroup.join_all();
//    std::cout << myGroup.size() << endl;
//    std::cout << "end" << endl;
//    map<unsigned int, double> myMap = Task::getAllResult();
//    for (map<unsigned int, double>::iterator it = myMap.begin(); it != myMap.end(); ++it) {
//        cout << "id= " << it->first << " result=" << it->second << endl;
//    }
    //Pool p(2);
    
        boost::shared_ptr<Pool> pool(new Pool(1,10));
        while(1){
            std::string command;
            cin>>command;
            if(command=="int"){
                int a,b,c;
                cin>>a>>b>>c;
                pool->addTask(a,b,c);
            }else if(command=="exit"){
                pool->exit();
               
                break;
            }else if(command=="removeTask"){
                
            }else if(command=="all"){
                cout<<pool->getAll()<<std::endl;
            }else{
                cout<<"Invalid command\n";
            }
        }
    
//    worker2();
    return 0;
}
