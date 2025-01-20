#ifndef HARDWARE_MONITOR_HPP
#define HARDWARE_MONITOR_HPP

#include <iostream>
#include <windows.h>



class HardwareMonitor {
public:
    HardwareMonitor();
    ~HardwareMonitor();
    double GetCPULoad();
    double GetRAMUsage();
    double GetGPUUsage();
    double GetTemperatureInfo();
    double GetGPUTemperature();
};

#endif // HARDWARE_MONITOR_HPP
