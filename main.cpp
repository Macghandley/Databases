/*
Skeleton code for storage and buffer management
*/

#include <string>
#include <ios>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include "classes.h"
using namespace std;


int main(int argc, char* const argv[]) {

    // Create the EmployeeRelation file from Employee.csv
    StorageBufferManager manager("EmployeeRelation");
    manager.createFromFile("Employee.csv");
    
    // Loop to lookup IDs until user is ready to quit
    int employeeId = 0;
    while (employeeId != -1) {
        cout << "Enter Employee ID to search (or -1 to quit): ";
        cin >> employeeId;

        if (employeeId != -1) {
            Record result = manager.findRecordById(employeeId);
            result.print();
        }

    }
    
    return 0;
}
