#include <iostream>
#include "DL5.h"
#include "PL11.h"
#include "PL16.h"
#include <windows.h>

#include "CalcLightIntensity.h"
#include "DD1.h"
#include "DD2.h"
#include "DD3.h"
#include "DD4.h"
#include "DD5.h"
#include "DL1.h"
#include "DL2.h"
#include "DL3.h"
#include "DL4.h"
#include "HDD3.h"
#include "HDL3.h"
#include <thread>
// #define LOOP_TEST
// #define APS_TEST
#define DVS_TEST
#define LOOP_TEST

int main() {
#ifdef LOOP_TEST
    while (true) {
#endif
#ifdef APS_TEST
        // APS
        calcDD1();
        calcDD2();
        calcDD3();
        calcDD4();
        calcDD5();
        calcDL1();
        calcDL2();
        calcDL3();
        calcDL4();
        calcHDD3();
        calcHDL3();
#endif

#ifdef DVS_TEST
        // DVS
        const int thread_count = 16;
        std::thread threads[thread_count];

        for (int i = 0; i < thread_count; ++i) {
            threads[i] = std::thread([i]() {
                calcDL5();
                calcPL11();
                calcPL16();
                CalcLightIntensity();
            });
        }

        for (auto &t : threads) t.join();
        Sleep(1000);
#endif

#ifdef LOOP_TEST
        Sleep(50);
    }
#endif
    system("pause");
    return 0;
}
