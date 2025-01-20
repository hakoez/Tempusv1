#include "hardware_monitor.hpp"
#include <windows.h>
#include <wbemidl.h>
#include <comdef.h>
#include <pdh.h>
#include <vector>
#include <string>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "pdh.lib")

HardwareMonitor::HardwareMonitor() {
    CoInitializeEx(0, COINIT_MULTITHREADED);
    CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT, 
                         RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
}

HardwareMonitor::~HardwareMonitor() {
    CoUninitialize();
}

double HardwareMonitor::GetCPULoad() {
    IWbemLocator *pLoc = nullptr;
    IWbemServices *pSvc = nullptr;
    IEnumWbemClassObject* pEnumerator = nullptr;

    HRESULT hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hres)) return 0.0;

    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 0, 0, 0, 0, &pSvc);
    if (FAILED(hres)) {
        pLoc->Release();
        return 0.0;
    }

    hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT LoadPercentage FROM Win32_Processor"),
                           WBEM_FLAG_FORWARD_ONLY, nullptr, &pEnumerator);
    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        return 0.0;
    }

    IWbemClassObject *pclsObj = nullptr;
    ULONG uReturn = 0;
    VARIANT vtProp;
    double cpuLoad = 0.0;

    if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
        pclsObj->Get(L"LoadPercentage", 0, &vtProp, 0, 0);
        cpuLoad = vtProp.intVal;
        VariantClear(&vtProp);
        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    return cpuLoad;
}

double HardwareMonitor::GetGPUUsage() {
    return 0.0; // WMI üzerinden GPU verisini almak için ekstra çalışma gerekiyor.
}

double HardwareMonitor::GetRAMUsage() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    return static_cast<double>(statex.dwMemoryLoad);
}

double HardwareMonitor::GetTemperatureInfo() {
    return 0.0; // WMI ile sıcaklık verisi alınamıyordu.
}

double HardwareMonitor::GetGPUTemperature() {
    IWbemLocator *pLoc = nullptr;
    IWbemServices *pSvc = nullptr;
    IEnumWbemClassObject* pEnumerator = nullptr;

    HRESULT hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hres)) return 0.0;

    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 0, 0, 0, 0, &pSvc);
    if (FAILED(hres)) {
        pLoc->Release();
        return 0.0;
    }

    hres = pSvc->ExecQuery(bstr_t("WQL"),
                           bstr_t("SELECT CurrentTemperature FROM Win32_TemperatureProbe"),
                           WBEM_FLAG_FORWARD_ONLY, nullptr, &pEnumerator);
    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        return 0.0;
    }

    IWbemClassObject *pclsObj = nullptr;
    ULONG uReturn = 0;
    VARIANT vtProp;
    double gpuTemp = 0.0;

    if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
        pclsObj->Get(L"CurrentTemperature", 0, &vtProp, 0, 0);
        if (vtProp.vt != VT_NULL) {
            gpuTemp = (vtProp.intVal - 2732) / 10.0; // Kelvin'den Celsius'a dönüşüm
        }
        VariantClear(&vtProp);
        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    return gpuTemp;
}

