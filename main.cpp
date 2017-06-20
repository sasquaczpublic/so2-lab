#include <stdio.h>
#include <ncurses.h>
#include <iostream>
//safe thread
#include <thread>
#include <chrono>
#include <mutex>
//stl
#include <string> 
#include <queue>
#include <vector>
#include <algorithm>
// rand
#include <stdlib.h>
#include <time.h>

using namespace std;

template <class T> 
class safeQueue {
    std::queue<T> originalQueue;
    std::mutex originalQueueGuard;

    public:
    void push(T element)
    {
        originalQueueGuard.lock();
        originalQueue.push(element);
        originalQueueGuard.unlock();
    }
    
    T frontPop()
    {
        this->empty() ? throw("Empty queue") : originalQueueGuard.lock();
        auto element = originalQueue.front();
        originalQueue.pop();
        originalQueueGuard.unlock();
        return element;
    }

    bool empty()
    {
        originalQueueGuard.lock();
        auto response = originalQueue.empty();
        originalQueueGuard.unlock();
        return response;
    }

};

// --- This values can be modified ---
const int M = 10; // row
const int N = 20; // col
const int SPEED = 50; // precentage
// ---                             ---

const int CLOCK = (100.0/(double)SPEED) * 100;

std::vector<std::vector<bool>> matrix;
std::mutex matrixMutex;

safeQueue<int> queue1;
safeQueue<int> queue2;

void printMatrix()
{
    for (int i = 0; i < matrix.size(); i++)
    {
        for (int j = 0; j < matrix[i].size(); j++)
        {
            matrix[i][j] ? mvprintw(i,j,to_string(1).c_str()) : mvprintw(i,j,to_string(0).c_str());
        }
    }
}

void shiftMatrix()
{
    while(true)
    {
        {
            std::lock_guard<std::mutex> lock(matrixMutex);
            std::for_each(matrix.begin(), matrix.end(), [](std::vector<bool>& row)
            {
                for (int j = row.size(); j >= 1; j--)
                {
                    row[j] = row[j-1];
                }
                row[0] = 0;
            });
        }
        this_thread::sleep_for(std::chrono::milliseconds(CLOCK));
    }
}

void setField()
{
    while(true)
    {
        {
            std::lock_guard<std::mutex> lock(matrixMutex);
            try
            {
              matrix[queue1.frontPop()][queue2.frontPop()] = true;
            } catch(...)
            {
                if(1);
            }
        }
        this_thread::sleep_for(std::chrono::milliseconds(CLOCK));
    }
}

// problem with generic generator
void generator(safeQueue<int>& generatorQueue, const int& range)
{
    generatorQueue.push(rand() % range);
}

void generator1()
{
    while(true)
    {
        queue1.push(rand() % M);
        this_thread::sleep_for(std::chrono::milliseconds(CLOCK));
    }
}

void generator2()
{
    while(true)
    {
        queue2.push(rand() % N);
        this_thread::sleep_for(std::chrono::milliseconds(CLOCK));
    }
}

void printer()
{
    while(true)
    {
        printMatrix();

        refresh();        
        this_thread::sleep_for(std::chrono::milliseconds(CLOCK));
    }
}

void killer()
{
    getch();
}

int main(int argc, char **argv) 
{
    srand (time(NULL));
    matrix.resize(M);
    for (int i = 0; i < matrix.size(); i++)
    {
        matrix[i].resize(N);
        for (int j = 0; j < matrix[i].size(); j++)
        {
            matrix[i][j] = false;
        }
    }
    
    initscr();    
    // auto generatorTh = std::thread(generator, queue1, M);
    auto generatorTh1 = std::thread(generator1);
    auto generatorTh2 = std::thread(generator2);
    auto shiftTh = std::thread(shiftMatrix);
    auto setFieldTh = std::thread(setField);
    auto printerTh = std::thread(printer);
    auto killerTh = std::thread(killer);
    killerTh.join();

    endwin();
    return 0;
}

