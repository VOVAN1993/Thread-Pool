/* 
 * File:   Pool.cpp
 * Author: vovan
 * 
 * Created on 15 Апрель 2013 г., 13:25
 */

#include "Pool.h"
typedef boost::mutex::scoped_lock scoped_lock;

Pool::Pool() {
    int quantityHot = std::max((int) boost::thread::hardware_concurrency(), 1);
    {
        scoped_lock group(this->groupMutex);
        for (int i = 0; i < quantityHot; i++) {
            boost::thread *thr = this->myGroup.create_thread(boost::bind(&worker));
    //        {   
    //            scoped_lock l(vecThMutex);
    //            vec.push_back(thr);
    //        }

        }
    }
}

Pool::Pool(unsigned int _quantityHot){
    for (int i = 0; i < _quantityHot; i++) {
        boost::thread *thr = this->myGroup.create_thread(boost::bind(&worker));
//        {   
//            scoped_lock l(vecThMutex);
//            vec.push_back(thr);
//        }

    }
}

Pool::~Pool() {
}

