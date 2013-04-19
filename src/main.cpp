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

int main(int argc, char** argv) {

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
            }else if(command=="del"){
                int _id;
                cin>>_id;
                pool->getID(_id);
            }else if(command=="all"){
                cerr<<pool->getAll()<<std::endl;
            }else{
                cerr<<"Invalid command\n";
            }
        }
    
//    worker2();
    return 0;
}
