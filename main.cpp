#include <hyprland/src/devices/IKeyboard.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/managers/eventLoop/EventLoopManager.hpp>
#include <hyprland/src/helpers/Monitor.hpp>
#include <hyprland/src/desktop/Workspace.hpp>

inline HANDLE PHANDLE = nullptr;

static CHyprSignalListener g_keyListener;
static SP<CEventLoopTimer> g_holdTimer;
static bool                g_overlayShown = false;
static bool                g_superHeld    = false;

static constexpr uint32_t KEY_SUPER_L = 125;

#define CFG(name) std::string(std::any_cast<Hyprlang::STRING>( \
    HyprlandAPI::getConfigValue(PHANDLE, "plugin:super-ws-overlay:" name)->getValue()))

static int cfgInt(const char* name) {
    auto* v = HyprlandAPI::getConfigValue(PHANDLE, std::string("plugin:super-ws-overlay:") + name);
    return v ? std::any_cast<Hyprlang::INT>(v->getValue()) : 0;
}

static void execAsync(const std::string& cmd) {
    g_pEventLoopManager->doLater([cmd]() {
        HyprlandAPI::invokeHyprctlCommand("dispatch", "exec " + cmd);
    });
}

static void showOverlays() {
    if (g_overlayShown)
        return;
    g_overlayShown = true;

    auto bg     = CFG("bg_color");
    auto fg     = CFG("text_color");
    auto border = CFG("border_color");
    auto radius = cfgInt("border_radius");
    auto fsize  = cfgInt("font_size");
    auto style = std::format(
        "background: {}; color: {}; border: 1px solid {}; border-radius: {}px; font-size: {}px;",
        bg, fg, border, radius, fsize);

    std::string cmd = "bash -c '";
    cmd += std::format("eww update ws-overlay-style=\"{}\" ; ", style);

    for (auto const& mon : g_pCompositor->m_monitors) {
        auto ws = mon->m_activeWorkspace;
        if (!ws)
            continue;
        cmd += std::format("eww update ws-overlay-text{}={} ; ", mon->m_id, ws->m_id);
        cmd += std::format("eww open ws-overlay-{} ; ", mon->m_id);
    }
    cmd += "'";
    execAsync(cmd);
}

static void hideOverlays() {
    if (!g_overlayShown)
        return;
    g_overlayShown = false;

    std::string args;
    for (auto const& mon : g_pCompositor->m_monitors) {
        if (!args.empty())
            args += " ";
        args += std::format("ws-overlay-{}", mon->m_id);
    }
    if (!args.empty())
        execAsync("eww close " + args);
}

static void cancelTimer() {
    if (g_holdTimer) {
        g_holdTimer->cancel();
        g_pEventLoopManager->removeTimer(g_holdTimer);
        g_holdTimer.reset();
    }
}

static void onKeyEvent(const IKeyboard::SKeyEvent& ev, Event::SCallbackInfo&) {
    if (ev.keycode == KEY_SUPER_L) {
        if (ev.state == WL_KEYBOARD_KEY_STATE_PRESSED) {
            g_superHeld = true;
            cancelTimer();

            auto holdMs = std::chrono::milliseconds(cfgInt("hold_ms"));

            g_holdTimer = makeShared<CEventLoopTimer>(
                std::optional<Time::steady_dur>{holdMs},
                [](SP<CEventLoopTimer>, void*) { showOverlays(); }, nullptr);
            g_pEventLoopManager->addTimer(g_holdTimer);
        } else {
            g_superHeld = false;
            cancelTimer();
            hideOverlays();
        }
    } else if (g_superHeld && ev.state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        cancelTimer();
    }
}

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const auto VER = HyprlandAPI::getHyprlandVersion(handle);
    if (VER.hash != GIT_COMMIT_HASH) {
        HyprlandAPI::addNotification(handle, "super-ws-overlay: version mismatch!", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
    }

    HyprlandAPI::addConfigValue(handle, "plugin:super-ws-overlay:hold_ms",       Hyprlang::INT{800});
    HyprlandAPI::addConfigValue(handle, "plugin:super-ws-overlay:bg_color",       Hyprlang::STRING{"rgba(10, 8, 16, 0.85)"});
    HyprlandAPI::addConfigValue(handle, "plugin:super-ws-overlay:text_color",     Hyprlang::STRING{"#7888a0"});
    HyprlandAPI::addConfigValue(handle, "plugin:super-ws-overlay:border_color",   Hyprlang::STRING{"rgba(255, 255, 255, 0.08)"});
    HyprlandAPI::addConfigValue(handle, "plugin:super-ws-overlay:border_radius",  Hyprlang::INT{16});
    HyprlandAPI::addConfigValue(handle, "plugin:super-ws-overlay:font_size",      Hyprlang::INT{48});

    g_keyListener = Event::bus()->m_events.input.keyboard.key.listen(
        [](const IKeyboard::SKeyEvent& ev, Event::SCallbackInfo& info) { onKeyEvent(ev, info); });

    HyprlandAPI::addNotification(handle, "super-ws-overlay loaded!", CHyprColor{0.2, 1.0, 0.2, 1.0}, 3000);

    return {"super-ws-overlay", "Show workspace overlay on Super hold", "qwexvf", "0.1"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    cancelTimer();
    hideOverlays();
    g_keyListener.reset();
}
