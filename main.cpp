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
using namespace std;
typedef boost::mutex::scoped_lock scoped_lock;
boost::mutex qMutex;
boost::mutex io_Mutex;
boost::condition qCond;
boost::mutex countMutex;
boost::mutex allMutex;
boost::mutex vecThMutex;
boost::mutex groupMutex;
queue<int> q;
vector<boost::thread*> vec;
boost::thread_group myGroup;
volatile bool isInterrupt = false;
/*
 * 
 */
int occupied = 0;
int all = 2;

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
        int k;
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
                    //find(vec.begin(),vec.end(),boost::this_thread) !=vec.end() 
                    if (vec[0]->get_id() == boost::this_thread::get_id() || vec[1]->get_id() == boost::this_thread::get_id()) {
                        scoped_lock io(io_Mutex);
                        //cout << "isTwo=true" << endl;
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
                            cout<<boost::this_thread::get_id()<<" "<<tmp->get_id()<<endl;
                        }
                        {

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
                qCond.notify_all();
                return;
            }
            k = q.front();
            q.pop();
        }
        printTask(k);
        boost::this_thread::sleep(boost::posix_time::seconds(5));
        changeCount(-1);
        printEndTask(k);
        //printSizeQ();
    }
}

int main(int argc, char** argv) {
    int col = std::max((int) boost::thread::hardware_concurrency(), 1);

    for (int i = 0; i < col; i++) {
        boost::thread *thr = myGroup.create_thread(boost::bind(&worker));
        {
            scoped_lock l(vecThMutex);
            vec.push_back(thr);
        }

    }
    int a = 0, k;
    cin >> a;
    for (int i = 0; i < a; i++) {
        cin >> k;
        scoped_lock lock(qMutex);
        q.push(k);
        if (getCount() + 1 > getAll()) {
            {
                incAll(1);
                scoped_lock w(io_Mutex);
                cout << "getCount= " << getCount() << "\n";
            }
            boost::thread *tmp = myGroup.create_thread(boost::bind(&worker));
            {
                scoped_lock l(vecThMutex);
                vec.push_back(tmp);
            }
            qCond.notify_one();
        } else {
            qCond.notify_one();
        }
        changeCount(1);
    }
    isInterrupt = true;
    myGroup.join_all();
    std::cout << myGroup.size() << endl;
    std::cout << "end";
    return 0;
}