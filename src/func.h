/* 
 * File:   func.h
 * Author: vovan
 *
 * Created on 12 Апрель 2013 г., 17:44
 */

#ifndef FUNC_H
#define	FUNC_H
#include <iostream>
double f(double a) {
    return a*a;
}

double integrate(double begin, double end, double step) {
    double res = 0;
    double interval = (end - begin) / step;
    //cout<<begin<<" "<<end<<" "<<step<<"\n";
    //cout<<interval<<"\n";
    for (int i = 0; i < (step - 1); ++i) {
        double a = begin + interval * i;
        double b = begin + interval * (i + 1);
        res += (f(a) + f(b)) * (b - a) / 2;
    }
    //cout<<res<<"\n";
    return res;
}



#endif	/* FUNC_H */

