#pragma once
#include <cstdint>

namespace uwp_bridge {
    // Chamar durante a carga de componentes (p.ex. no component_loader ou no inicio do client)
    void init();

    // Retorna true se a ponte UWP/Steam estiver ativa
    bool is_active();

    // Opcional: força re-check (para debug)
    bool detect_uwp_exe();
}
