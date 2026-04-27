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
#define LOOP_TEST
#define APS_TEST
#define EVS_TEST

int main() {
#ifdef LOOP_TEST
    while (true) {
#endif
#ifdef APS_TEST
        // APS_DD1_DD2
        const int aps_dd1_dd2_thread_count = 1;
        std::thread aps_dd1_dd2_threads[aps_dd1_dd2_thread_count];
        for (int i = 0; i < aps_dd1_dd2_thread_count; ++i) {
            aps_dd1_dd2_threads[i] = std::thread([i]() {
                calcDD1();
                calcDD2();
            });
        }

        for (auto &t : aps_dd1_dd2_threads) t.join();
        Sleep(1000);

        // APS
        const int aps_thread_count = 16;
        std::thread aps_threads[aps_thread_count];
        for (int i = 0; i < aps_thread_count; ++i) {
            aps_threads[i] = std::thread([i]() {
                // calcDD1();
                // calcDD2();
                calcDD3();
                calcDD4();
                calcDD5();
                calcDL1();
                calcDL2();
                calcDL3();
                calcDL4();
                calcHDD3();
                calcHDL3();
            });
        }

        for (auto &t : aps_threads) t.join();
        Sleep(1000);

#endif

#ifdef EVS_TEST
        // DVS
        const int evs_thread_count = 16;
        std::thread evs_threads[evs_thread_count];

        for (int i = 0; i < evs_thread_count; ++i) {
            evs_threads[i] = std::thread([i]() {
                calcDL5();
                calcPL11();
                calcPL16();
                CalcLightIntensity();
            });
        }

        for (auto &t : evs_threads) t.join();
        Sleep(1000);
#endif

#ifdef LOOP_TEST
        Sleep(500);
    }
#endif
    system("pause");
    return 0;
}
