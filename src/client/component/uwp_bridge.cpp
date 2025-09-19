#include "uwp_bridge.hpp"
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

// use existing log macro if available; fallback:
#ifndef ALOG
  #include <iostream>
  #define ALOG(fmt, ...) do { char buf[1024]; sprintf_s(buf, fmt, __VA_ARGS__); OutputDebugStringA(buf); std::cout << buf << std::endl; } while(0)
#endif

static bool g_uwp_bridge_active = false;

// externs: nomes típicos no BO3Enhanced. Ajuste se diferente no seu bdAuthXB1toSteam.
extern "C" {
    void apply_bdAuthXB1toSteam_patches(); // aplica hooks/patches
    void bdAuthXB1toSteam_createSteamRequestData(uint8_t* out); // cria request data (opcional)
    // se existir uma função init específ., adicione aqui e chame
    // void bdAuthXB1toSteam_init();
}

static bool path_contains_windowsapps(const std::string& path) {
    std::string low = path;
    std::transform(low.begin(), low.end(), low.begin(), ::tolower);
    return (low.find("windowsapps") != std::string::npos) || (low.find("microsoft") != std::string::npos && low.find("store") != std::string::npos);
}

bool uwp_bridge::detect_uwp_exe() {
    char exe_path[MAX_PATH] = {0};
    if (!GetModuleFileNameA(NULL, exe_path, MAX_PATH)) {
        return false;
    }
    std::string path(exe_path);

    // Heurística 1: WindowsApps no caminho
    if (path_contains_windowsapps(path)) {
        ALOG("[uwp_bridge] detected UWP via path: %s\n", path.c_str());
        return true;
    }

    // Heurística 2: procurar por assinatura de Arxan no bin (se quiser)
    std::ifstream f(path, std::ios::binary);
    if (f) {
        const std::string needle = "arxan";
        std::vector<char> buffer((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        std::string content(buffer.begin(), buffer.end());
        std::string low = content;
        std::transform(low.begin(), low.end(), low.begin(), ::tolower);
        if (low.find(needle) == std::string::npos) {
            ALOG("[uwp_bridge] no arxan string found -> likely UWP/no-DRM\n");
            return true; // sem Arxan => provável UWP/loose exe
        } else {
            ALOG("[uwp_bridge] arxan signature found -> likely Steam exe\n");
            return false;
        }
    }

    // fallback: assume false
    return false;
}

void uwp_bridge::init() {
    if (g_uwp_bridge_active) return;

    bool is_uwp = detect_uwp_exe();
    if (!is_uwp) {
        ALOG("[uwp_bridge] not a UWP exe; bridge inactive\n", "");
        return;
    }

    // Ativa ponte: aplicar patches do bdAuth
    ALOG("[uwp_bridge] activating UWP->Steam bridge\n", "");
    // proteja chamadas com try/catch se quiser
    apply_bdAuthXB1toSteam_patches();

    // opcional: se houver um init específico, chame aqui
    // bdAuthXB1toSteam_init();

    g_uwp_bridge_active = true;
    ALOG("[uwp_bridge] bridge active\n", "");
}

bool uwp_bridge::is_active() {
    return g_uwp_bridge_active;
}
