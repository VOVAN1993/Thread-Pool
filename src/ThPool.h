/* 
 * File:   ThPool.h
 * Author: vovan
 *
 * Created on 12 Апрель 2013 г., 17:47
 */

#ifndef THPOOL_H
#define	THPOOL_H

class ThPool {
public:
    ThPool(unsigned int countHotTh);
    virtual ~ThPool();
private:
    ThPool(const ThPool& orig);
};

#endif	/* THPOOL_H */

