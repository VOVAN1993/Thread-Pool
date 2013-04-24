/* 
 * File:   main.cpp
 * Author: vovan
 *
 * Created on 8 Апрель 2013 г., 23:47
 */

#include <cstdlib>
#include <boost/interprocess/detail/atomic.hpp>
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


int main(int argc, char** argv) {

        boost::shared_ptr<Pool> pool(boost::shared_ptr<Pool> (new Pool(1,10)));
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
            }else if(command=="del"){
                int _id;
                cin>>_id;
                pool->removeTask(_id);
            }else if(command=="all"){
                cerr<<pool->getAll()<<std::endl;
            }else{
                cerr<<"Invalid command\n";
            }
        }
//    worker2();
    return 0;
}
