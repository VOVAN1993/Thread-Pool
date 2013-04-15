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

class Pool {
public:
    Pool();
    Pool(unsigned int _quantityHot);
    virtual ~Pool();
private:
    boost::mutex groupMutex;
    boost::thread_group myGroup;
    Pool(const Pool& orig);

};

#endif	/* POOL_H */

