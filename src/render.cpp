#include "render.hpp"
#include "hardware_monitor.hpp"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <thread>
#include <atomic>
#include <windows.h>
#include <shlobj.h>
#include <filesystem>

std::atomic<double> cpuUsage = 0.0;
std::atomic<double> gpuUsage = 0.0;
std::atomic<double> ramUsage = 0.0;
std::atomic<double> temperature = 0.0;
std::atomic<double> gpuTemperature = 0.0;

std::string GetAppDataPath() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        return std::string(path) + "\\Tempus\\";
    }
    return "";
}

void SaveImGuiSettings() {
    std::string appDataPath = GetAppDataPath();
    std::filesystem::create_directories(appDataPath);
    std::string iniFilePath = appDataPath + "imgui.ini";
    ImGui::SaveIniSettingsToDisk(iniFilePath.c_str());
}

void LoadImGuiSettings() {
    std::string appDataPath = GetAppDataPath();
    std::string iniFilePath = appDataPath + "imgui.ini";
    ImGui::LoadIniSettingsFromDisk(iniFilePath.c_str());
}

void UpdateHardwareData(HardwareMonitor& hwMonitor) {
    while (true) {
        cpuUsage = hwMonitor.GetCPULoad();
        gpuUsage = hwMonitor.GetGPUUsage();
        ramUsage = hwMonitor.GetRAMUsage();
        temperature = hwMonitor.GetTemperatureInfo();
        gpuTemperature = hwMonitor.GetGPUTemperature();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void ApplyCustomStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.5f, 0.2f, 0.7f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.6f, 0.3f, 0.8f, 1.0f);
    style.ScaleAllSizes(1.3f);
}

void WindowClass::Draw(std::string_view label)
{
    constexpr static auto window_flags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    constexpr static auto window_size = ImVec2(240.0F, 500.0F);
    constexpr static auto window_pos = ImVec2(0.0F, 0.0F);

    ImGui::SetNextWindowSize(window_size);
    ImGui::SetNextWindowPos(window_pos);
    static HardwareMonitor hwMonitor;
    static bool initialized = false;

    if (!initialized) {
        std::thread updateThread(UpdateHardwareData, std::ref(hwMonitor));
        updateThread.detach();
        initialized = true;
        ApplyCustomStyle();
        LoadImGuiSettings();
    }

    ImGui::Begin("Tempusv1", nullptr, window_flags);
    
    ImGui::BeginGroup();
    if (ImGui::BeginChild("CPU Info", ImVec2(110, 30), true)) {
        ImGui::Text("CPU Usage:");
        ImGui::EndChild();
    }
    ImGui::SameLine();
    if (ImGui::BeginChild("CPU Value", ImVec2(110, 30), true)) {
        ImGui::Text("%.2f%%", cpuUsage.load());
        ImGui::EndChild();
    }
    ImGui::EndGroup();
    
    ImGui::End();
    
    SaveImGuiSettings();
}

void render(WindowClass &window_obj)
{
    window_obj.Draw("Label");
}
