#include "main.h"
#include "fetcher.h"
#include "imgui.h"
#include "implot.h"
#include "implot_internal.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include <atomic>
#include <cstring>
#include <filesystem>
#include <mutex>
#include <thread>
#include <tuple>
#include <vector>
#include <stdio.h>
#include <ctime>
#include <SDL3/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif



// pls dont use direct x....

//cfg
bool cfgLoaded = false;
bool csvLoaded = false;
std::string cfgPath = "red.cfg";

std::tuple<std::string, std::string, std::string, std::string> getCfg()
{
    if (!cfgLoaded) {
        cfg.load(cfgPath);
        cfgLoaded = true;
    }
    std::string ticker = cfg.getStr("ticker");
    std::string tf = cfg.getStr("tf");
    std::string data = cfg.getStr("data");
    std::string results = cfg.getStr("results");

    transform(ticker.begin(), ticker.end(), ticker.begin(), ::toupper);

    return { ticker, tf, data, results };
}

void getChart(){
    if (!cfgLoaded) {
         cfg.load(cfgPath);
         cfgLoaded = true;
    }
   std::string ticker = cfg.getStr("ticker");
   std::string tf = cfg.getStr("tf");
   std::string data = cfg.getStr("data");
   std::string results = cfg.getStr("results");
    
   std::string csvPath; 
   csvPath = (data+ticker+tf+".csv");
   if (!csvLoaded) {
       csv.load(csvPath);
       csvLoaded = true;
    
   }
};

double TimeToUnixSeconds(const Time& t) {
    std::tm tm{};
    tm.tm_year = t.year - 1900; 
    tm.tm_mon = t.month - 1;    
    tm.tm_mday = t.day;
    tm.tm_hour = t.hour;
    tm.tm_min = t.minute;
    tm.tm_sec = t.second;
    tm.tm_isdst = -1;  

    return static_cast<double>(std::mktime(&tm));
}


namespace candlePlot {template <typename T>
int BinarySearch(const T* arr, int l, int r, T x) {
    if (r >= l) {
        int mid = l + (r - l) / 2;
        if (arr[mid] == x)
            return mid;
        if (arr[mid] > x)
            return BinarySearch(arr, l, mid - 1, x);
        return BinarySearch(arr, mid + 1, r, x);
    }
    return -1;
}

void PlotCandlestick(const char* label_id, const double* xs, const double* opens, const double* closes, const double* lows, const double* highs, int count, bool tooltip, float width_percent, ImVec4 bullCol, ImVec4 bearCol) {

    ImDrawList* draw_list = ImPlot::GetPlotDrawList();
    double half_width = count > 1 ? (xs[1] - xs[0]) * width_percent : width_percent;

    if (ImPlot::IsPlotHovered() && tooltip) {
        ImPlotPoint mouse   = ImPlot::GetPlotMousePos();
        mouse.x             = ImPlot::RoundTime(ImPlotTime::FromDouble(mouse.x), ImPlotTimeUnit_Day).ToDouble();
        float  tool_l       = ImPlot::PlotToPixels(mouse.x - half_width * 1.5, mouse.y).x;
        float  tool_r       = ImPlot::PlotToPixels(mouse.x + half_width * 1.5, mouse.y).x;
        float  tool_t       = ImPlot::GetPlotPos().y;
        float  tool_b       = tool_t + ImPlot::GetPlotSize().y;
        ImPlot::PushPlotClipRect();
        draw_list->AddRectFilled(ImVec2(tool_l, tool_t), ImVec2(tool_r, tool_b), IM_COL32(128,128,128,64));
        ImPlot::PopPlotClipRect();
        int idx = BinarySearch(xs, 0, count - 1, mouse.x);
        if (idx != -1) {
            ImGui::BeginTooltip();
            char buff[32];
            ImPlot::FormatDate(ImPlotTime::FromDouble(xs[idx]),buff,32,ImPlotDateFmt_DayMoYr,ImPlot::GetStyle().UseISO8601);
            ImGui::Text("Day:   %s",  buff);
            ImGui::Text("Open:  $%.2f", opens[idx]);
            ImGui::Text("Close: $%.2f", closes[idx]);
            ImGui::Text("Low:   $%.2f", lows[idx]);
            ImGui::Text("High:  $%.2f", highs[idx]);
            ImGui::EndTooltip();
        }
    }

    if (ImPlot::BeginItem(label_id)) {
        ImPlot::GetCurrentItem()->Color = IM_COL32(64,64,64,255);
        if (ImPlot::FitThisFrame()) {
            for (int i = 0; i < count; ++i) {
                ImPlot::FitPoint(ImPlotPoint(xs[i], lows[i]));
                ImPlot::FitPoint(ImPlotPoint(xs[i], highs[i]));
            }
        }
        for (int i = 0; i < count; ++i) {
            ImVec2 open_pos  = ImPlot::PlotToPixels(xs[i] - half_width, opens[i]);
            ImVec2 close_pos = ImPlot::PlotToPixels(xs[i] + half_width, closes[i]);
            ImVec2 low_pos   = ImPlot::PlotToPixels(xs[i], lows[i]);
            ImVec2 high_pos  = ImPlot::PlotToPixels(xs[i], highs[i]);
            ImU32 color      = ImGui::GetColorU32(opens[i] > closes[i] ? bearCol : bullCol);
            draw_list->AddLine(low_pos, high_pos, color);
            draw_list->AddRectFilled(open_pos, close_pos, color);
        }

        ImPlot::EndItem();
    }
}
}


int main(int, char**)
{
    // Setup SDL
    // [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_TRANSPARENT | SDL_WINDOW_BORDERLESS;
    SDL_Window* window = SDL_CreateWindow("RedChart", (int)(1280 * main_scale), (int)(720 * main_scale), window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr)
    {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Red theme overrides
    {
        ImVec4* c = ImGui::GetStyle().Colors;
        ImVec4 red       = ImVec4(0.75f, 0.10f, 0.10f, 1.00f);
        ImVec4 redHover  = ImVec4(0.90f, 0.20f, 0.20f, 1.00f);
        ImVec4 redActive = ImVec4(1.00f, 0.30f, 0.30f, 1.00f);
        ImVec4 redDim    = ImVec4(0.40f, 0.05f, 0.05f, 1.00f);
        c[ImGuiCol_FrameBg]            = redDim;
        c[ImGuiCol_FrameBgHovered]     = red;
        c[ImGuiCol_FrameBgActive]      = redHover;
        c[ImGuiCol_TitleBgActive]      = red;
        c[ImGuiCol_CheckMark]          = redActive;
        c[ImGuiCol_SliderGrab]         = red;
        c[ImGuiCol_SliderGrabActive]   = redActive;
        c[ImGuiCol_Button]             = redDim;
        c[ImGuiCol_ButtonHovered]      = red;
        c[ImGuiCol_ButtonActive]       = redActive;
        c[ImGuiCol_Header]             = redDim;
        c[ImGuiCol_HeaderHovered]      = red;
        c[ImGuiCol_HeaderActive]       = redActive;
        c[ImGuiCol_Separator]          = red;
        c[ImGuiCol_SeparatorHovered]   = redHover;
        c[ImGuiCol_SeparatorActive]    = redActive;
        c[ImGuiCol_ResizeGrip]         = redDim;
        c[ImGuiCol_ResizeGripHovered]  = red;
        c[ImGuiCol_ResizeGripActive]   = redActive;
        c[ImGuiCol_Tab]                = redDim;
        c[ImGuiCol_TabHovered]         = redHover;
        c[ImGuiCol_TabSelected]        = red;
        c[ImGuiCol_PlotLines]          = redActive;
        c[ImGuiCol_PlotLinesHovered]   = redHover;
        c[ImGuiCol_PlotHistogram]      = red;
        c[ImGuiCol_PlotHistogramHovered] = redHover;
        c[ImGuiCol_TextSelectedBg]     = redDim;
        c[ImGuiCol_NavCursor]          = redActive;
        c[ImGuiCol_DragDropTarget]     = redActive;
    }

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding   = 10.0f;
    style.FrameRounding    = 6.0f;
    style.GrabRounding     = 6.0f;
    style.PopupRounding    = 8.0f;
    style.ChildRounding    = 8.0f;
    style.TabRounding      = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

    ImVec4 clear_color = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Main loop
    bool done = false;
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!done)
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        
        // Window for Red-Chart
        {

            ImGuiWindowFlags flags = 0;

            ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_FirstUseEver);
            ImGui::Begin("RedChart", nullptr, flags);

            static char ticker_buf[64] = "";
            static char tf_buf[64] = "";
            static char data_buf[128] = "";
            static char results_buf[128] = "";

            if (ImGui::Button("Show Config")) {
                auto [ticker, tf, data, results] = getCfg();
                strncpy(ticker_buf, ticker.c_str(), sizeof(ticker_buf));
                strncpy(tf_buf, tf.c_str(), sizeof(tf_buf));
                strncpy(data_buf, data.c_str(), sizeof(data_buf));
                strncpy(results_buf, results.c_str(), sizeof(results_buf));
                ImGui::OpenPopup("Config Popup");
            }

            if (ImGui::BeginPopup("Config Popup")) {
                static std::string cfg_status;
                static bool show_data = false;
                static std::vector<std::string> data_files;
                static const char* known_tfs[] = {
                    "1mo","3mo","1wk","90m","60m","30m","15m",
                    "5d","1d","1h","5m","2m","1m","15"
                };

                ImGui::Text("Configuration Settings");
                ImGui::InputText("Ticker", ticker_buf, IM_ARRAYSIZE(ticker_buf));
                ImGui::InputText("Time Frame", tf_buf, IM_ARRAYSIZE(tf_buf));
                ImGui::InputText("Data Path", data_buf, IM_ARRAYSIZE(data_buf));
                ImGui::InputText("Results Path", results_buf, IM_ARRAYSIZE(results_buf));
                if (ImGui::Button("Save")) {
                    cfg.set("ticker",  ticker_buf);
                    cfg.set("tf",      tf_buf);
                    cfg.set("data",    data_buf);
                    cfg.set("results", results_buf);
                    bool ok = cfg.save(cfgPath);
                    csvLoaded = false;
                    cfg_status = ok ? "Saved to " + cfgPath
                                    : "Save failed: " + cfgPath;
                }
                ImGui::SameLine();
                if (ImGui::Button(show_data ? "Hide Data" : "Show Data")) {
                    show_data = !show_data;
                    if (show_data) {
                        data_files.clear();
                        try {
                            for (const auto& e : std::filesystem::directory_iterator(data_buf)) {
                                if (!e.is_regular_file()) continue;
                                std::string n = e.path().filename().string();
                                if (n.size() > 4 && n.substr(n.size()-4) == ".csv")
                                    data_files.push_back(n);
                            }
                            std::sort(data_files.begin(), data_files.end());
                        } catch (const std::exception& ex) {
                            cfg_status = std::string("Data dir: ") + ex.what();
                        }
                    }
                }
                if (show_data) {
                    ImGui::Separator();
                    ImGui::Text("Files in %s (click to apply):", data_buf);
                    if (data_files.empty())
                        ImGui::TextDisabled("(no .csv files)");
                    for (const auto& fn : data_files) {
                        if (ImGui::Button(fn.c_str())) {
                            std::string stem = fn.substr(0, fn.size()-4);
                            std::string tk = stem, tf = "";
                            for (const char* kt : known_tfs) {
                                size_t klen = strlen(kt);
                                if (stem.size() > klen &&
                                    stem.compare(stem.size()-klen, klen, kt) == 0) {
                                    tk = stem.substr(0, stem.size()-klen);
                                    tf = kt;
                                    break;
                                }
                            }
                            strncpy(ticker_buf, tk.c_str(), sizeof(ticker_buf)-1);
                            ticker_buf[sizeof(ticker_buf)-1] = '\0';
                            strncpy(tf_buf, tf.c_str(), sizeof(tf_buf)-1);
                            tf_buf[sizeof(tf_buf)-1] = '\0';
                            cfg.set("ticker", tk);
                            cfg.set("tf",     tf);
                            bool ok = cfg.save(cfgPath);
                            csvLoaded = false;
                            cfg_status = ok ? "Applied " + fn
                                            : "Apply failed: " + fn;
                        }
                    }
                }
                if (!cfg_status.empty())
                    ImGui::TextWrapped("%s", cfg_status.c_str());
                ImGui::EndPopup();
            }


            if (ImGui::Button("Fetch Ticker"))
                ImGui::OpenPopup("Fetch Popup");

            if (ImGui::BeginPopup("Fetch Popup")) {
                static char fetch_ticker[16]  = "";
                static int  fetch_candles     = 300;
                static int  fetch_tf_idx      = 7; // "1d"
                static const char* tfs[]      = {"1m","2m","5m","15m","30m","60m","90m","1d","5d","1wk","1mo","3mo"};
                static std::mutex        fetch_mtx;
                static std::string       fetch_status;
                static std::atomic<bool> fetch_running{false};

                ImGui::InputText("Ticker##fetch", fetch_ticker, sizeof(fetch_ticker));
                ImGui::Combo("Interval##fetch", &fetch_tf_idx, tfs, IM_ARRAYSIZE(tfs));
                ImGui::SliderInt("Candles##fetch", &fetch_candles, 10, 2000);

                bool disabled = fetch_running || fetch_ticker[0] == '\0';
                if (disabled) ImGui::BeginDisabled();
                if (ImGui::Button("Fetch##go")) {
                    fetch_running = true;
                    {
                        std::lock_guard<std::mutex> lk(fetch_mtx);
                        fetch_status = "Fetching...";
                    }
                    auto [_, __, data, ___] = getCfg();
                    std::string t   = fetch_ticker;
                    std::string iv  = tfs[fetch_tf_idx];
                    int         n   = fetch_candles;
                    std::transform(t.begin(), t.end(), t.begin(), ::toupper);
                    std::string out = data + t + iv + ".csv";
                    std::thread([t, iv, n, out]() {
                        FetchResult r = fetchTicker(t, iv, n, out);
                        {
                            std::lock_guard<std::mutex> lk(fetch_mtx);
                            fetch_status = r.message;
                        }
                        fetch_running = false;
                    }).detach();
                }
                if (disabled) ImGui::EndDisabled();

                {
                    std::lock_guard<std::mutex> lk(fetch_mtx);
                    if (!fetch_status.empty())
                        ImGui::TextWrapped("%s", fetch_status.c_str());
                }

                ImGui::EndPopup();
            }

            static bool show_chart = false;
            if (ImGui::Button("Load Chart")) {
                getChart();
                show_chart = true;
            }

            ImGui::End();

            if (show_chart) {
                ImGui::SetNextWindowSize(ImVec2(900, 560), ImGuiCond_FirstUseEver);
                if (ImGui::Begin("Chart", &show_chart)) {
                    ImGui::Text("Chart [%s] %s", ticker_buf, tf_buf);
                    if (!csvLoaded) {
                        ImGui::Text("No chart data loaded.");
                    } else {
                        std::vector<double> xs;
                        std::vector<double> opens, highs, lows, closes;
                        std::vector<std::pair<Time, Candlestick>> sorted_chart(csv.Chart.begin(), csv.Chart.end());
                        std::sort(sorted_chart.begin(), sorted_chart.end(), [](const auto& a, const auto& b) {
                            const Time& t1 = a.first;
                            const Time& t2 = b.first;
                            return std::tie(t1.year, t1.month, t1.day, t1.hour, t1.minute, t1.second) < std::tie(t2.year, t2.month, t2.day, t2.hour, t2.minute, t2.second);
                        });
                        for (const auto& [time, candle] : sorted_chart) {
                            xs.push_back(TimeToUnixSeconds(time));
                            opens.push_back(candle.open);
                            highs.push_back(candle.high);
                            lows.push_back(candle.low);
                            closes.push_back(candle.close);
                        }
                        if (xs.size() < 2) {
                            ImGui::Text("Not enough data to plot.");
                        } else {
                            ImVec2 avail = ImGui::GetContentRegionAvail();
                            if (ImPlot::BeginPlot(ticker_buf, avail, ImPlotFlags_NoTitle | ImPlotFlags_Crosshairs)) {
                                ImPlot::SetupAxes("Time", "Price",
                                    ImPlotAxisFlags_None,
                                    ImPlotAxisFlags_None);
                                ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                                ImPlot::SetupAxisLimits(ImAxis_X1, xs.front(), xs.back(), ImPlotCond_Once);
                                ImPlot::SetupAxisLimits(ImAxis_Y1,
                                    *std::min_element(lows.begin(), lows.end()),
                                    *std::max_element(highs.begin(), highs.end()),
                                    ImPlotCond_Once);
                                candlePlot::PlotCandlestick(
                                    "Candles",
                                    xs.data(),
                                    opens.data(),
                                    closes.data(),
                                    lows.data(),
                                    highs.data(),
                                    (int)xs.size(),
                                    true,
                                    0.25f,
                                    ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                                    ImVec4(1.0f, 0.0f, 0.0f, 1.0f)
                                );
                                ImPlot::EndPlot();
                            }
                        }
                    }
                }
                ImGui::End();
            }
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppQuit() function]
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}