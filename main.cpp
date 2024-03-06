/* 
    CS 440 Assignment 5

    McKellam Handley - handleym@oregonstate.edu - 933654458
    Dylan Varga - vargad@oregonstate.edu - 933831567
*/  

#include <bits/stdc++.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <array>

using namespace std;

//defines how many blocks are available in the Main Memory 
#define buffer_size 22

class Records{
    public:
    
    struct EmpRecord {
        int eid;
        string ename;
        int age;
        double salary;

        bool operator < (const Records& emp) const
        {
            return (eid < emp.emp_record.eid);
        }
    }emp_record;

    struct DeptRecord {
        int did;
        string dname;
        double budget;
        int managerid;
    }dept_record;

    void print_emp(){
        cout << emp_record.eid << ","
             << emp_record.ename << ","
             << emp_record.age << ","
             << emp_record.salary << "\n";
    }

    int no_values = 0; //You can use this to check if you've don't have any more tuples
    int number_of_emp_records = 0; // Tracks number of emp_records you have on the buffer
    int number_of_dept_records = 0; //Track number of dept_records you have on the buffer
};

// Grab a single block from the Emp.csv file and put it inside the EmpRecord structure of the Records Class
Records Grab_Emp_Record(fstream &empin) {
    string line, word;
    Records emp;
    // grab entire line
    if (getline(empin, line, '\n')) {
        // turn line into a stream
        stringstream s(line);
        // gets everything in stream up to comma
        getline(s, word,',');
        emp.emp_record.eid = stoi(word);
        getline(s, word, ',');
        emp.emp_record.ename = word;
        getline(s, word, ',');
        emp.emp_record.age = stoi(word);
        getline(s, word, ',');
        emp.emp_record.salary = stod(word);

        //Ensuring that you cannot use both structure (EmpEecord, DeptRecord) at the same memory block / time 
        emp.dept_record.did = 0;
        emp.dept_record.dname = "";
        emp.dept_record.budget = 0;
        emp.dept_record.managerid = 0;

        return emp;
    } else {
        emp.no_values = -1;
        return emp;
    }
}

// Grab a single block from the Dept.csv file and put it inside the DeptRecord structure of the Records Class
Records Grab_Dept_Record(fstream &deptin) {
    string line, word;
    //DeptRecord dept;
    Records dept;
    if (getline(deptin, line, '\n')) {
        stringstream s(line);
        getline(s, word,',');
        dept.dept_record.did = stoi(word);
        getline(s, word, ',');
        dept.dept_record.dname = word;
        getline(s, word, ',');
        dept.dept_record.budget = stod(word);
        getline(s, word, ',');
        dept.dept_record.managerid = stoi(word);

        //Ensuring that you cannot use both structure (EmpEecord, DeptRecord) at the same memory block / time 
        dept.emp_record.eid = 0;
        dept.emp_record.ename = "";
        dept.emp_record.age = 0;
        dept.emp_record.salary = 0;

        return dept;
    } else {
        dept.no_values = -1;
        return dept;
    }
}

Records buffers[buffer_size]; //use this class object of size 22 as your main memory

//PASS 1
//Sorting the buffers in main_memory
void Sort_Emp_Buffer(int n = buffer_size){
    //Remember: You can use only [AT MOST] 22 blocks for sorting the records / tuples and create the runs
    // Sort buffers by emp ID
    sort(buffers, buffers + n, [](Records a, Records b){return a.emp_record.eid < b.emp_record.eid; });
    return;
}

void Sort_Dept_Buffer(int n = buffer_size){
    //Remember: You can use only [AT MOST] 22 blocks for sorting the records / tuples and create the runs
    // Sort buffers by dept manager ID
    sort(buffers, buffers + n, [](Records a, Records b){return a.dept_record.managerid < b.dept_record.managerid; });
    return;
}

// Write employees
void PrintSortedEmps(fstream &outputFile, int n = buffer_size){
    int intSalary;
    for(int i = 0; i < n; i++){
        intSalary = static_cast<int>(buffers[i].emp_record.salary);
        outputFile << to_string(buffers[i].emp_record.eid) << ","
                   << buffers[i].emp_record.ename << ","
                   << to_string(buffers[i].emp_record.age) << ","
                   << to_string(intSalary) << "\n";
    }
    return;
}

// Write departments
void PrintSortedDepts(fstream &outputFile, int n = buffer_size){
    int intBudget;
    for(int i = 0; i < n; i++){
    intBudget = static_cast<int>(buffers[i].dept_record.budget);
    outputFile << to_string(buffers[i].dept_record.did) << ","
               << buffers[i].dept_record.dname << ","
               << to_string(intBudget) << ","
               << to_string(buffers[i].dept_record.managerid) << "\n";
    }
    return;
}

// Create employee runs
int Create_Emp_Runs(fstream &empin){

    int runIndex = 0;
    int i = 0;
    while(!empin.eof()){
        buffers[i] = Grab_Emp_Record(empin);
        i++;

        if(i == buffer_size || buffers[i - 1].no_values == -1){
            // Keep novalue buffer out of mem
            if(buffers[i - 1].no_values == -1){
                i--;
            }
            // Sort and print run
            Sort_Emp_Buffer(i);

            if(i != 0){
                fstream tempEmpFile;
                tempEmpFile.open("tempEmpFile" + std::to_string(runIndex), ios::out | ios::app);
                PrintSortedEmps(tempEmpFile, i);
                tempEmpFile.close();
                runIndex++;
            }
            
            i = 0;
        }
    }
    return runIndex;
}

// Create department runs
int Create_Dept_Runs(fstream &deptin){

    int runIndex = 0;
    int i = 0;
    while(!deptin.eof()){
        buffers[i] = Grab_Dept_Record(deptin);
        i++;

        if(i == buffer_size || buffers[i - 1].no_values == -1){
            // Keep novalue buffer out of mem
            if(buffers[i - 1].no_values == -1){
                i--;
            }
            // Sort and print run
            Sort_Dept_Buffer(i);

            if(i != 0){
                fstream tempDeptFile;
                tempDeptFile.open("tempDeptFile" + std::to_string(runIndex), ios::out | ios::app);
                PrintSortedDepts(tempDeptFile, i);
                tempDeptFile.close();
                runIndex++;
            }
            
            i = 0;
        }
    }
    return runIndex;
}

void PrintJoin(fstream& outputFile, Records &emp, Records &dept){
    
    int intSalary = static_cast<int>(emp.emp_record.salary);
    int intBudget = static_cast<int>(dept.dept_record.budget);
    outputFile << to_string(emp.emp_record.eid) << ","
               << emp.emp_record.ename << ","
               << to_string(emp.emp_record.age) << ","
               << to_string(intSalary) << ","
               << to_string(dept.dept_record.did) << ","
               << dept.dept_record.dname << ","
               << to_string(intBudget) << ","
               << to_string(dept.dept_record.managerid) << "\n";
    return;
}

//PASS 2
//Use main memory to Merge the Runs 

// Helper fucntion to grab next employee index
int Get_Next_Employee(int lowerEmpIndex, int upperEmpIndex){
    int next = lowerEmpIndex;
    for(int i = lowerEmpIndex + 1; i < upperEmpIndex; i++){
        if(buffers[i].no_values != -1){
            if(buffers[i].emp_record.eid < buffers[next].emp_record.eid){
                next = i;
            }
        }
    }
    if(buffers[next].no_values == -1){
        next = -1;
    }
    return next;
}

// Helper fucntion to grab next department index
int Get_Next_Department(int lowerDeptIndex, int upperDeptIndex){
    int next = lowerDeptIndex;
    for(int i = lowerDeptIndex + 1; i < upperDeptIndex; i++){
        if(buffers[i].no_values != -1){
            if(buffers[i].dept_record.managerid < buffers[next].dept_record.managerid){
                next = i;
            }
        }
    }
    if(buffers[next].no_values == -1){
        next = -1;
    }
    return next;
}

//Use main memory to Merge and Join tuples 
//which are already sorted in 'runs' of the relations Dept and Emp
void Merge_Join_Runs(fstream &sortOut, int empRunCount, int deptRunCount){

    fstream empInputs[empRunCount];
    fstream deptInputs[deptRunCount];

    // Tracking variables for buffer
    int lowerEmpIndex = 0;
    int upperEmpIndex = empRunCount;
    int lowerDeptIndex = empRunCount;
    int upperDeptIndex = empRunCount + deptRunCount;

    // Open runs and read to buffer
    for(int i = 0; i < empRunCount; i++){
        string filename = "tempEmpFile" + to_string(i);
        empInputs[i].open(filename, ios::in);
        buffers[lowerEmpIndex + i] = Grab_Emp_Record(empInputs[i]);
    }
    for(int i = 0; i < deptRunCount; i++){
        string filename = "tempDeptFile" + to_string(i);
        deptInputs[i].open(filename, ios::in);
        buffers[lowerDeptIndex + i] = Grab_Dept_Record(deptInputs[i]);
    }

    // Get initial next-s
    int nextEmp = Get_Next_Employee(lowerEmpIndex, upperEmpIndex);
    int nextDept = Get_Next_Department(lowerDeptIndex, upperDeptIndex);

    // Loop until either table is empty
    while(nextEmp != -1 && nextDept != -1){
        if(buffers[nextEmp].emp_record.eid < buffers[nextDept].dept_record.managerid){
            buffers[nextEmp] = Grab_Emp_Record(empInputs[nextEmp - lowerEmpIndex]);
            nextEmp = Get_Next_Employee(lowerEmpIndex, upperEmpIndex);
        }else if(buffers[nextEmp].emp_record.eid > buffers[nextDept].dept_record.managerid){
            buffers[nextDept] = Grab_Dept_Record(deptInputs[nextDept - lowerDeptIndex]);
            nextDept = Get_Next_Department(lowerDeptIndex, upperDeptIndex);
        }else{
            // Match managers to departments
            while(buffers[nextEmp].emp_record.eid == buffers[nextDept].dept_record.managerid) {
                PrintJoin(sortOut, buffers[nextEmp], buffers[nextDept]);
                buffers[nextDept] = Grab_Dept_Record(deptInputs[nextDept - lowerDeptIndex]);
                nextDept = Get_Next_Department(lowerDeptIndex, upperDeptIndex);
            }

            buffers[nextEmp] = Grab_Emp_Record(empInputs[nextEmp - lowerEmpIndex]);
            nextEmp = Get_Next_Employee(lowerEmpIndex, upperEmpIndex);
        }
    }
    return;
}

int main() {

    //Open file streams to read and write
    //Opening out two relations Emp.csv and Dept.csv which we want to join
    fstream empin;
    fstream deptin;
    empin.open("Emp.csv", ios::in);
    deptin.open("Dept.csv", ios::in);
   
    //Creating the Join.csv file where we will store our joined results
    fstream joinout;
    joinout.open("Join.csv", ios::out | ios::app | ios::trunc);

    //1. Create runs for Dept and Emp which are sorted using Sort_Buffer()
    int empRunCount = Create_Emp_Runs(empin);
    int deptRunCount = Create_Dept_Runs(deptin);

    //2. Use Merge_Join_Runs() to Join the runs of Dept and Emp relations 
    Merge_Join_Runs(joinout, empRunCount, deptRunCount);

    // Cleanup temp files
    for (int i = 0; i < empRunCount; ++i) {
        std::string filename = "tempEmpFile" + std::to_string(i);
        const char* filename_cstr = filename.c_str();
        remove(filename_cstr);
    }
    for (int i = 0; i < deptRunCount; ++i) {
        std::string filename = "tempDeptFile" + std::to_string(i);
        const char* filename_cstr = filename.c_str();
        remove(filename_cstr);
    }

    cout << "Emp.csv and Dept.csv joined into Join.csv. Temp run files removed.\n";

}
