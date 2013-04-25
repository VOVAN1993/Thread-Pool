/* 
 * File:   main.cpp
 * Author: vovan
 *
 * Created on 8 Апрель 2013 г., 23:47
 */

#include <iostream>
#include "Task.h"
#include "Pool.h"
using namespace std;

int main(int argc, char** argv) {

    boost::shared_ptr<Pool> pool(boost::shared_ptr<Pool > (new Pool(1, 10)));
    while (1) {
        std::string command;
        cin >> command;
        if (command == "int") {
            int a, b, c;
            cin >> a >> b >> c;
            pool->addTask(a, b, c);
        } else if (command == "exit") {
            pool->exit();
            break;
        } else if (command == "del") {
            int _id;
            cin >> _id;
            bool b = pool->removeTask(_id);
            if (b == true) {
                cout << "The task '" << _id << "' was removed\n";
            } else {
                cout << "No such task found\n";
            }
        } else {
            cerr << "Invalid command\n";
        }
    }
    return 0;
}
