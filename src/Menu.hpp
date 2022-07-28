#pragma once

#include "stdafx.h"
#include "font/font.hpp"
#include "SubwayGraph.hpp"
#include "Dijkstra.hpp"

class Menu
{
public:
    Menu() = default;
    ~Menu() = default;

    //call this function once when initializing menu
    bool init(GLFWwindow* window);

    void mainloop();

    void destroy();

private:
    inline void renderMainMenuBar();

    inline void renderLog();

    inline void renderControls();

    inline void renderAddControls();

    inline void renderCanvas();

    inline void renderGraph();

    //utils
    inline float ZOOM(const float val) const {
        return val * zoomScale;
    }

    static void helpMarker(const char* desc) {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    static inline void initSubwayGraph();

    inline void setupStyle();

    static inline std::string string2UTF8(const std::string& str)
    {
        int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
        wchar_t* pwBuf = new wchar_t[nwLen + 1];
        ZeroMemory(pwBuf, nwLen * 2 + 2);
        ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);
        int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
        char* pBuf = new char[nLen + 1];
        ZeroMemory(pBuf, nLen + 1);
        ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
        std::string retStr(pBuf);
        delete[]pwBuf;
        delete[]pBuf;
        pwBuf = NULL;
        pBuf = NULL;
        return retStr;
    }

    static inline std::string UTF82string(const std::string& str)
    {
        BSTR    bstrWide;
        char* pszAnsi;
        int     nLength;
        const char* pszCode = str.c_str();

        nLength = MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, NULL, NULL);
        bstrWide = SysAllocStringLen(NULL, nLength);

        MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, bstrWide, nLength);

        nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);
        pszAnsi = new char[nLength];

        WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);
        SysFreeString(bstrWide);

        std::string r(pszAnsi);
        delete[] pszAnsi;
        return r;
    }

    inline void updateTexts();


private:
    GLFWwindow* window{ nullptr };

    //controls
    bool isRunning{ true };
    bool showAddControls{ false };
    bool shouldDrawGrid{ true };
    bool shouldDrawRoute{ true };
    bool shouldShowFPS{ true };
    float gridInterval{ 64.0 };
    float zoomScale{ 1.f };
    float graphScale{ 3000.f };
    bool selectedAddControlsTab[3];

    //parameters
    int startStationIdx{0};
    int terminalStationIdx{0};

    //texts
    const char** textLines{ nullptr };
    int textLinesSize{ NULL };
    const char*** textStations{ nullptr };
    int textStationsSize{ NULL };
    int* textStationsCnts{ nullptr };

    //fonts
    ImFont* cousineRegular{ nullptr };
    ImFont* karlaRegular{ nullptr };
    ImFont* msyh{ nullptr };

    //render data
    ds::Vector<ImVec4> railwayLineColors;
    ImVec4 routeColor{ 1.f, 0, 0, 1.f };
    float stationMarkRadius{ 5.f };
    int* route{ nullptr };
    int routeLen{ NULL };

    //canvas specs
    ImVec2 canvasOrigin{ 0.f, 0.f }; //screen coordinate of the origin point in canvas
};

bool Menu::init(GLFWwindow* window)
{
	if (window == nullptr) return false;
	this->window = window;
	//setup imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
   
	//setup style
    setupStyle();

	//setup randerer backends
	if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) return false;
	if (!ImGui_ImplOpenGL2_Init()) return false;

    //init subwayGraph
    initSubwayGraph();
    updateTexts();

	return true;
}

void Menu::mainloop()
{
    //opengl clear color 
    ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 1.00f);

    //loop
    while (!glfwWindowShouldClose(window) && isRunning)
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Rendering
        renderMainMenuBar();
        renderCanvas();
        renderLog();
        renderControls();
        if (showAddControls) renderAddControls();
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }
}

void Menu::destroy()
{
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (textLines != nullptr && textLinesSize > 0) {
        for (int i = 0; i < textLinesSize; i++)
        {
            if (textLines[i] != nullptr) free((void*)textLines[i]);
        }
        free(textLines);
    }

    if (textStations != nullptr && textStationsCnts != nullptr && textStationsSize > 0) {
        for (int i = 0; i < textStationsSize; i++) {
            for (int j = 0; j < textStationsCnts[i]; j++) free((void*)textStations[i][j]);
            free((void*)textStations[i]);
        }
        free(textStations);
        free(textStationsCnts);
    }

}

inline void Menu::renderMainMenuBar()
{
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Exit")) this->isRunning = false;
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Tools"))
    {
        if (ImGui::BeginMenu("Add"))
        {
            if (ImGui::MenuItem("Station"))
            { 
                memset(selectedAddControlsTab, 0, sizeof(selectedAddControlsTab));
                selectedAddControlsTab[0] = true;
                showAddControls = true; 
            }
            if (ImGui::MenuItem("Rail line"))
            { 
                memset(selectedAddControlsTab, 0, sizeof(selectedAddControlsTab));
                selectedAddControlsTab[1] = true;
                showAddControls = true;
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Modify"))
        {
            if (ImGui::MenuItem("Arc cost"))
            {
                memset(selectedAddControlsTab, 0, sizeof(selectedAddControlsTab));
                selectedAddControlsTab[2] = true;
                showAddControls = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }

    helpMarker("This application is made by Tongji University CS student 2050250.");
    ImGui::EndMainMenuBar();
}

inline void Menu::renderLog()
{
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowPos(ImVec2(0, 317), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(559, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Log output", 0, flags);
    ImGui::End();
    g_log->draw(msyh, "Log output");
}

inline void Menu::renderControls()
{
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowPos(ImVec2(0, 18), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(558, 297), ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls", 0, flags);
    ImGui::BeginTabBar("##TabBar");
    if (ImGui::BeginTabItem("Options"))
    {
        static int startLineIdx = 0;
        static int terminalLineIdx = 0;
        static int tmpStartStationIdx = 0;
        static int tmpTerminalStationIdx = 0;
        static bool minimalStations = true;
        ImGui::PushFont(msyh);
        ImGui::PushItemWidth(200.f);
        ImGui::Text("Start station:");
        ImGui::SameLine(0.f, 36.f);
        if (ImGui::Combo("##StartLines", &startLineIdx, textLines, textLinesSize)) { 
            tmpStartStationIdx = 0;
            if (textStations[startLineIdx][tmpStartStationIdx] != nullptr)
                startStationIdx = g_graph->indexOf(UTF82string(textStations[startLineIdx][tmpStartStationIdx]));
        }
        ImGui::SameLine();
        if (ImGui::Combo("##StartStations", &tmpStartStationIdx, textStations[startLineIdx], textStationsCnts[startLineIdx]))
            startStationIdx = g_graph->indexOf(UTF82string(textStations[startLineIdx][tmpStartStationIdx]));
        ImGui::Text("Terminal station:");
        ImGui::SameLine();
        if (ImGui::Combo("##TerminalLines", &terminalLineIdx, textLines, textLinesSize)) { 
            tmpTerminalStationIdx = 0;
            if (textStations[terminalLineIdx][tmpTerminalStationIdx] != nullptr)
                terminalStationIdx = g_graph->indexOf(UTF82string(textStations[terminalLineIdx][tmpTerminalStationIdx]));
        }
        ImGui::SameLine();
        if (ImGui::Combo("##TerminalStations", &tmpTerminalStationIdx, textStations[terminalLineIdx], textStationsCnts[terminalLineIdx]))
            terminalStationIdx = g_graph->indexOf(UTF82string(textStations[terminalLineIdx][tmpTerminalStationIdx]));
        ImGui::PopFont();
        ImGui::PopItemWidth();
        if (ImGui::RadioButton("Minimal transfer stations", minimalStations)) minimalStations = !minimalStations;
        ImGui::SameLine();
        if (ImGui::RadioButton("Minimal cost", !minimalStations))minimalStations = !minimalStations;
        if (ImGui::Button("Find best route."))
        {
            if (this->route != nullptr) {
                free(this->route);
                this->route = nullptr;
            }
            int** mat = nullptr;
            int size = g_graph->asMat(mat, !minimalStations);
            this->routeLen = Dijkstra::Helper::calculate((const int**)mat, size, startStationIdx, terminalStationIdx, this->route);
        }

        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Misc"))
    {
        ImGui::Checkbox("Grid view", &shouldDrawGrid);
        ImGui::SameLine();
        ImGui::Checkbox("Draw route", &shouldDrawRoute);
        ImGui::SameLine();
        ImGui::Checkbox("Show FPS", &shouldShowFPS);
        ImGui::Separator();
        ImGui::Text("Graph scale:");
        ImGui::SameLine();
        ImGui::SliderFloat("##Graph scale", &graphScale, 2000.f, 4000.f);
        ImGui::BeginChild("Color edit", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NavFlattened);
        ImGui::PushItemWidth(-160);
        for (int i = 1; i <= g_graph->getTotalLines(); i++) {
            char buf[32];
            sprintf_s(buf, "Line %d", i);
            char buf2[32];
            sprintf_s(buf2, "##%s", buf);
            ImGui::ColorEdit4(buf2, &railwayLineColors[i].x, ImGuiColorEditFlags_AlphaBar);
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::TextUnformatted(buf);
        }
        ImGui::PopItemWidth();
        ImGui::EndChild();
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
    ImGui::End();
}

inline void Menu::renderAddControls()
{
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
    ImGui::SetNextWindowSize(ImVec2(580, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("Additional controls", &showAddControls, flags);
    ImGui::BeginTabBar("##TabBar");
    static ds::Vector<bool> isLineSelected;
    static ds::Vector<int> selectedStationsIdx;
    static ds::Vector<int> selectedLinesIdx;
    static ds::Vector<int> adjStationsIdx;
    static ds::Vector<int> adjStationsCost;
    static char stationName[256];
    static double latitude = 31.25;
    static double longitude = 121.45;
    static ds::Vector<int> lineNums;
    ds::Vector<int> eraseList;
    if(isLineSelected.size() < textLinesSize) isLineSelected.resize(textLinesSize, false);
    if (ImGui::BeginTabItem("Add station", nullptr, selectedAddControlsTab[0] ? ImGuiTabItemFlags_SetSelected : 0))
    {
        selectedAddControlsTab[0] = false; //reset flag
        ImGui::PushFont(msyh);
        ImGui::PushItemWidth(130.f);
        ImGui::Text("New station name: ");
        ImGui::SameLine();
        ImGui::InputText("##InputStationName", stationName, IM_ARRAYSIZE(stationName));
        ImGui::SameLine();
        if (ImGui::BeginCombo("##NewStationLineNum", "Line numbers")) {
            for (int i = 0; i < textLinesSize; i++) {
                if (ImGui::Selectable(textLines[i], &isLineSelected[i]))
                {
                    if (isLineSelected[i]) lineNums.push_back(i + 1);
                    else lineNums.find_erase(i + 1);
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Text("Latitude:");
        ImGui::SameLine();
        ImGui::InputDouble("##Latitude", &latitude);
        ImGui::SameLine();
        ImGui::Text("Longitude:");
        ImGui::SameLine();
        ImGui::InputDouble("##Longitude", &longitude);
        ImGui::Separator();
        ImGui::Text("Adjacent stations:");
        ImGui::BeginChild("##AdjStations", ImVec2(0, 0), true);
        for (int i = 0; i < adjStationsIdx.size(); i++)
        {
            ImGui::PushItemWidth(120.f);
            char id1[256];
            sprintf_s(id1, "##StartLines%d", i);
            char id2[256];
            sprintf_s(id2, "##StartStations%d", i);
            if (ImGui::Combo(id1, &selectedLinesIdx[i], textLines, textLinesSize)) {
                selectedStationsIdx[i] = 0;
                if (textStations[selectedLinesIdx[i]][selectedStationsIdx[i]] != nullptr) {
                    adjStationsIdx[i] = g_graph->indexOf(UTF82string(textStations[selectedLinesIdx[i]][selectedStationsIdx[i]]));
                }
            }
            ImGui::SameLine();
            if (ImGui::Combo(id2, &selectedStationsIdx[i], textStations[selectedLinesIdx[i]], textStationsCnts[selectedLinesIdx[i]])) {
                if (textStations[selectedLinesIdx[i]][selectedStationsIdx[i]] != nullptr) {
                    adjStationsIdx[i] = g_graph->indexOf(UTF82string(textStations[selectedLinesIdx[i]][selectedStationsIdx[i]]));
                }
            }
            ImGui::SameLine();
            ImGui::PopItemWidth();
            ImGui::PushItemWidth(80.f);
            char id3[256];
            sprintf_s(id3, "Remove##%d", i);
            ImGui::Text("Cost:");
            ImGui::SameLine();
            ImGui::InputInt("##Cost", &adjStationsCost[i], 0);
            ImGui::SameLine();
            if (ImGui::Button(id3)) {
                eraseList.push_back(i);
            }
            ImGui::PopItemWidth();
        }
        ImGui::EndChild();
        ImGui::PopItemWidth();
        ImGui::PopFont();
        for (int i = 0; i < eraseList.size(); i++) {
            selectedStationsIdx.erase(selectedStationsIdx.begin() + eraseList[i]);
            selectedLinesIdx.erase(selectedLinesIdx.begin() + eraseList[i]);
            adjStationsIdx.erase(adjStationsIdx.begin() + eraseList[i]);
        }
        if (ImGui::Button("Add adjacent stations")) {
            selectedLinesIdx.push_back(0);
            selectedStationsIdx.push_back(0);
            if(textStations[selectedLinesIdx.back()][selectedStationsIdx.back()] != nullptr)
                adjStationsIdx.push_back(g_graph->indexOf(UTF82string(textStations[selectedLinesIdx.back()][selectedStationsIdx.back()])));
            adjStationsCost.push_back(1);
        }
        ImGui::SameLine();

        if (ImGui::Button("Save new station")) {
            if (stationName[0] == 0) {
                LOG("[Error] A station name is required...\n");
            }
            else if (adjStationsIdx.size() > 0) {
                if (g_graph->insert(stationName, lineNums, latitude, longitude, adjStationsIdx, adjStationsCost))
                {
                    LOG("[Info] Successfully saved new station %s to subway graph...\n", stationName);
                    updateTexts();
                }
                else
                    LOG("[Error] Station name duplicates, unable to save new station...\n");
            }
            else {
                LOG("[Error] At least 1 adjacent station needs to be specified...\n");
            }
        }
        ImGui::EndTabItem();
    }
    else {
        ZeroMemory(stationName, IM_ARRAYSIZE(stationName));
        selectedStationsIdx.clear();
        selectedLinesIdx.clear();
        adjStationsIdx.clear();
        adjStationsCost.clear();
    }

    if (ImGui::BeginTabItem("Add rail line", nullptr, selectedAddControlsTab[1] ? ImGuiTabItemFlags_SetSelected : 0))
    {
        selectedAddControlsTab[1] = false; //reset flag
        ImGui::Text("dfashjkdbaskjdbaskjbdkj");
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Modify arc cost", nullptr, selectedAddControlsTab[2] ? ImGuiTabItemFlags_SetSelected : 0))
    {
        selectedAddControlsTab[2] = false; //reset flag
        ImGui::PushFont(msyh);
        ImGui::PushItemWidth(130.f);
        static int selectedLine = 0;
        static int selectedSrcVexIdx = 1;
        static int selectedDstVexIdx = 0;
        static int i1 = g_graph->indexOf(UTF82string(textStations[selectedLine][selectedSrcVexIdx]));
        static int i2 = g_graph->indexOf(UTF82string(textStations[selectedLine][selectedDstVexIdx]));
        static const char* buf[256];
        if (ImGui::Combo("##ModifyArcLines", &selectedLine, textLines, textLinesSize))
        {
            selectedSrcVexIdx = 0;
            selectedDstVexIdx = 0;
            i1 = g_graph->indexOf(UTF82string(textStations[selectedLine][selectedSrcVexIdx]));
            i2 = g_graph->indexOf(UTF82string(textStations[selectedLine][selectedDstVexIdx]));
        }
        ImGui::SameLine();
        if (ImGui::Combo("##ModifyArcVex1", &selectedSrcVexIdx, textStations[selectedLine], textStationsCnts[selectedLine]))
        {
            selectedDstVexIdx = 0;
            i1 = g_graph->indexOf(UTF82string(textStations[selectedLine][selectedSrcVexIdx]));
            i2 = g_graph->indexOf(UTF82string(textStations[selectedLine][selectedDstVexIdx]));
        }
        if (textStations[selectedLine][selectedSrcVexIdx] != nullptr)
        {
            auto vex = g_graph->vexAt(g_graph->indexOf(UTF82string(textStations[selectedLine][selectedSrcVexIdx])));
            ZeroMemory(buf, sizeof(buf));
            int i = 0;
            for (auto arc = vex.first; arc != nullptr; arc = arc->next) {
                std::string str = string2UTF8(g_graph->vexAt(arc->adjVex).name);
                buf[i] = (const char*)realloc((void*)buf[i], sizeof(char) * (str.size() + 1));
                ZeroMemory((void*)buf[i], sizeof(char)* (str.size() + 1));
                memcpy_s((void*)buf[i], str.size(), str.c_str(), str.size());
                i++;
            }
            ImGui::SameLine();
            if(ImGui::Combo("##ModifyArcVex2", &selectedDstVexIdx, buf, i))
                i2 = g_graph->indexOf(UTF82string(buf[selectedDstVexIdx]));
        }
        static int newCost = 1;
        ImGui::Text("Cost");
        ImGui::SameLine();
        ImGui::InputInt("##Cost", &newCost);
        if (ImGui::Button("Update"))
        {
            if (g_graph->updateArcCost(i1, i2, newCost))
            {
                LOG("[Info] New cost %d has been updated between %s and %s...\n", newCost, textStations[selectedLine][selectedSrcVexIdx], buf[selectedDstVexIdx]);
            }
            else
            {
                LOG("[Error] Unable to update cost...\n");
            }
        }

        //if (textStations[selectedLine][selectedSrcVexIdx] != nullptr && selectedSrcVexIdx != selectedDstVexIdx)
        //{
        //    int* cost = g_graph->getArcCost(UTF82string(textStations[selectedLine][selectedSrcVexIdx]), UTF82string(textStations[selectedLine][selectedDstVexIdx]));
        //    ImGui::InputInt("##Cost", cost, 1);
        //}

        ImGui::PopItemWidth();
        ImGui::PopFont();
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
    ImGui::End();
}

inline void Menu::renderCanvas()
{
    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowPos(ImVec2(557, 19), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(722, 698), ImGuiCond_FirstUseEver);
    ImGui::Begin("Canvas", 0, windowFlags);
    if (ImGui::Button("0.50x")) { zoomScale = 0.50f;}
    ImGui::SameLine();
    if (ImGui::Button("1.00x")) { zoomScale = 1.00f;}
    ImGui::SameLine();
    if (ImGui::Button("1.25x")) { zoomScale = 1.25;}
    ImGui::SameLine();
    if (ImGui::Button("2.00x")) { zoomScale = 2.00;}
    ImGui::SameLine();
    if (ImGui::SliderFloat("Zoom", &zoomScale, 0.25f, 5.00f)) {}
    ImVec2 canvas_tl = ImGui::GetCursorScreenPos();
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail();

    ImVec2 canvas_br = ImVec2(canvas_tl.x + canvas_sz.x, canvas_tl.y + canvas_sz.y);
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(canvas_tl, canvas_br, IM_COL32(50, 50, 50, 255));
    drawList->AddRect(canvas_tl, canvas_br, IM_COL32(255, 255, 255, 255));

    //add interaction logic
    ImGui::InvisibleButton(
        "canvas", canvas_sz,
        ImGuiButtonFlags_::ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_::ImGuiButtonFlags_MouseButtonRight);
    const bool isActive = ImGui::IsItemActive();
    const bool is_hovered = ImGui::IsItemHovered();

    //drag canvas
    static ImVec2 dragDistance = { 0, 0 }; //the x and y distance of dragging
    canvasOrigin = { canvas_tl.x + dragDistance.x + ZOOM(canvas_sz.x) / 2, canvas_tl.y + dragDistance.y + ZOOM(canvas_sz.y) / 2 }; //the screen coord for canvas origin point
    const ImVec2 mouse_pos_in_canvas(io.MousePos.x - canvasOrigin.x, io.MousePos.y - canvasOrigin.y);

    //update drag distance
    if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Right, -1.0f))
    {
        dragDistance.x += io.MouseDelta.x;
        dragDistance.y += io.MouseDelta.y;
    }

    if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {

    }

    if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
    {
    }

    drawList->PushClipRect(canvas_tl, canvas_br, true);
    if (shouldDrawGrid) //draw grids
    {
        for (float x = fmodf(dragDistance.x, ZOOM(gridInterval)); x < canvas_sz.x; x += ZOOM(gridInterval)) //draw cols
            drawList->AddLine(ImVec2(canvas_tl.x + x, canvas_tl.y), ImVec2(canvas_tl.x + x, canvas_br.y), IM_COL32(200, 200, 200, 40));
        for (float y = fmodf(dragDistance.y, ZOOM(gridInterval)); y < canvas_sz.y; y += ZOOM(gridInterval)) //draw rows
            drawList->AddLine(ImVec2(canvas_tl.x, canvas_tl.y + y), ImVec2(canvas_br.x, canvas_tl.y + y), IM_COL32(200, 200, 200, 40));
    }
    renderGraph();
    if (shouldShowFPS)
    {
        char fps[256];
        sprintf_s(fps, "Frame rate: %.0f fps", ImGui::GetIO().Framerate);
        drawList->AddText(ImVec2(canvas_tl.x + 5, canvas_tl.y + 5), IM_COL32(255, 255, 255, 255), fps);
    }
    drawList->PopClipRect();

    ImGui::End();
}

inline void Menu::renderGraph()
{
    constexpr double SH_LONGITUDE = 121.45f;
    constexpr double SH_LATITUDE = 31.25f;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    for (int i = 0; i < g_graph->size(); i++)
    {
        auto vex = g_graph->vexAt(i);
        ImVec2 src((vex.coord_x - SH_LONGITUDE) * ZOOM(graphScale) + canvasOrigin.x, (vex.coord_y - SH_LATITUDE) * ZOOM(graphScale) + canvasOrigin.y);
        drawList->AddCircle(src, ZOOM(stationMarkRadius), ImGui::ColorConvertFloat4ToU32(railwayLineColors[vex.lineNum[0]]), 0, ZOOM(1.3f));
        drawList->AddText(msyh, ZOOM(10.f), src, IM_COL32(255, 255, 255, 255), string2UTF8(vex.name).c_str());
        for (auto arc = vex.first; arc != nullptr; arc = arc->next)
        {
            if (arc->adjVex < 0) continue;
            auto adjVex = g_graph->vexAt(arc->adjVex);
            ImVec2 dst((adjVex.coord_x - SH_LONGITUDE) * ZOOM(graphScale) + canvasOrigin.x, (adjVex.coord_y - SH_LATITUDE) * ZOOM(graphScale) + canvasOrigin.y);
            ImVec4 lineColor(NULL, NULL, NULL, NULL);
            bool isSrcInRoute = false;
            bool isDstInRoute = false;
            if (shouldDrawRoute && route != nullptr) {
                for (int x = 0; x < routeLen; x++) {
                    if (!isSrcInRoute && route[x] == g_graph->indexOf(vex.name)) isSrcInRoute = true;
                    if (!isDstInRoute && route[x] == g_graph->indexOf(adjVex.name)) isDstInRoute = true;
                    if (isSrcInRoute && isDstInRoute) break;
                }
            }

            //find connected line number
            uint32_t lineNum = 1;
            for (int x = 0; x < vex.lineNum.size(); x++) {
                for (int y = 0; y < adjVex.lineNum.size(); y++)
                {
                    if (vex.lineNum[x] == adjVex.lineNum[y]) {
                        lineNum = vex.lineNum[x];
                        break;
                    }
                }
            }

            lineColor = shouldDrawRoute && isSrcInRoute && isDstInRoute ? routeColor : railwayLineColors[lineNum];
            float lineWeight = shouldDrawRoute && isSrcInRoute && isDstInRoute ? 2.f : 1.5f;

            double angle = atan((src.y - dst.y) / (src.x - dst.x));
            drawList->AddLine(
                ImVec2(src.x + (src.x > dst.x ? -1 : 1) * ZOOM(stationMarkRadius) * cos(angle), src.y + (src.x > dst.x ? -1 : 1) * ZOOM(stationMarkRadius) * sin(angle)),
                ImVec2(dst.x + (src.x < dst.x ? -1 : 1) * ZOOM(stationMarkRadius) * cos(angle), dst.y + (src.x < dst.x ? -1 : 1) * ZOOM(stationMarkRadius) * sin(angle)),
                ImGui::ColorConvertFloat4ToU32(lineColor),
                ZOOM(lineWeight));
        }
    }
}

inline void Menu::setupStyle()
{
    ImGuiStyle* style = &ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.WantSaveIniSettings = false;

    //roundings
    style->WindowRounding = 8.f;
    style->FrameRounding = 3.f;
    style->ScrollbarRounding = 12.f;

    //fonts
    io.Fonts->AddFontDefault();
    karlaRegular = io.Fonts->AddFontFromMemoryCompressedTTF((void*)karla_regular_data, karla_regular_size, 200.0f, NULL);
    cousineRegular = io.Fonts->AddFontFromMemoryCompressedTTF((void*)cousine_regular_data, cousine_regular_size, 200.0f, NULL);
    msyh = io.Fonts->AddFontFromMemoryCompressedTTF((void*)msyh_data, msyh_size, 20.f, NULL, io.Fonts->GetGlyphRangesChineseFull());

    //color styles
    ImGui::StyleColorsDark();
    this->railwayLineColors.resize(20);
    railwayLineColors[1] = { 0.81, 0.10, 0.10, 0.90 };  //line 1
    railwayLineColors[2] = { 0.10, 0.64, 0.10, 0.90 };  //line 2
    railwayLineColors[3] = { 0.98, 0.88, 0.01, 0.90 };  //line 3
    railwayLineColors[4] = { 0.46, 0.10, 0.46, 0.90 };  //line 4
}

inline void Menu::updateTexts()
{
    //update station texts
    const int bufferSize = g_graph->size();
    if (bufferSize <= 0) return;  //empty check

    const int totalLines = g_graph->getTotalLines();
    if (totalLines <= 0) return;  //empty check

    //station texts
    if (this->textStations == nullptr) {
        this->textStations = (const char***)malloc(sizeof(const char**) * totalLines);
    }
    else if (totalLines > textStationsSize) {
        const char*** p = (const char***)realloc(textStations, sizeof(const char**) * totalLines);
        if (p != nullptr) textStations = p;
    }

    if (this->textStationsCnts == nullptr) {
        this->textStationsCnts = (int*)malloc(sizeof(int) * totalLines);
    }
    else if (totalLines > textStationsSize) {
        int* p = (int*)realloc(textStationsCnts, sizeof(int) * totalLines);
        if (p != nullptr) textStationsCnts = p;
    }

    ZeroMemory(textStations, sizeof(const char**) * totalLines);
    ZeroMemory(textStationsCnts, sizeof(int) * totalLines);

    for (int i = 0; i < bufferSize; i++)
    {
        auto vex = g_graph->vexAt(i);
        auto str = string2UTF8(vex.name);
        auto size = str.size() + 1;

        for (int j = 0; j < vex.lineNum.size(); j++) {
            uint32_t lineNum = vex.lineNum[j];
            textStations[lineNum - 1] = (const char**)realloc(textStations[lineNum - 1], (textStationsCnts[lineNum - 1] + 1) * sizeof(const char*));
            textStations[lineNum - 1][textStationsCnts[lineNum - 1]] = nullptr;
            auto ptr = textStations[lineNum - 1][textStationsCnts[lineNum - 1]];
            ptr = (const char*)realloc((void*)ptr, sizeof(char) * size);
            if (ptr == nullptr) return;
            textStations[lineNum - 1][textStationsCnts[lineNum - 1]] = ptr;
            memset((void*)ptr, 0, size);
            memcpy_s((void*)ptr, size, str.c_str(), size);
            textStationsCnts[lineNum - 1]++;
        }
    }

    //update line texts
    if (totalLines <= 0) return;
    if (textLines == nullptr) {
        textLines = (const char**)malloc(sizeof(const char*) * totalLines);
        memset(textLines, 0, sizeof(const char*) * totalLines);
    }
    else if (textLinesSize < bufferSize) {
        const char** p = (const char**)realloc(textLines, sizeof(const char*) * totalLines);
        if (p != nullptr) textLines = p;
    }
    if (textLines == nullptr) return;
    textLinesSize = totalLines;
    for (int i = 0; i < totalLines; i++)
    {
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        sprintf_s(buffer, "%d号线", i + 1);
        auto str = string2UTF8(buffer);
        const int size = str.size() + 1;
        textLines[i] = (const char*)realloc((void*)textLines[i], size * sizeof(const char));
        memset((void*)textLines[i], 0, size * sizeof(char));
        memcpy_s((void*)textLines[i], size, str.c_str(), size * sizeof(char));
    }
}

inline void Menu::initSubwayGraph()
{
    //railway line 1
    g_graph->insert("富锦路", { 1 }, 31.394206, 121.419948, {  }, { });
    g_graph->insert("友谊西路", { 1 }, 31.383264, 121.423247, { g_graph->indexOf("富锦路") }, { 1 });
    g_graph->insert("宝安公路", { 1 }, 31.371644, 121.426297, { g_graph->indexOf("友谊西路") }, { 1 });
    g_graph->insert("共富新村", { 1 }, 31.356997, 121.429383, { g_graph->indexOf("宝安公路") }, { 1 });
    g_graph->insert("呼兰路", { 1 }, 31.341434, 121.43311, { g_graph->indexOf("共富新村") }, { 1 });
    g_graph->insert("通河新村", { 1 }, 31.333256, 121.436764, { g_graph->indexOf("呼兰路") }, { 1 });
    g_graph->insert("共康路", { 1 }, 31.320818, 121.44242, { g_graph->indexOf("通河新村") }, { 1 });
    g_graph->insert("彭浦新村", { 1 }, 31.308528, 121.444073, { g_graph->indexOf("共康路") }, { 1 });
    g_graph->insert("汶水路", { 1 }, 31.294405, 121.445543, { g_graph->indexOf("彭浦新村") }, { 1 });
    g_graph->insert("上海马戏城", { 1 }, 31.28144, 121.447488, { g_graph->indexOf("汶水路") }, { 1 });
    g_graph->insert("延长路", { 1 }, 31.273658, 121.450876, { g_graph->indexOf("上海马戏城") }, { 1 });
    g_graph->insert("中山北路", { 1 }, 31.261058, 121.454577, { g_graph->indexOf("延长路") }, { 1 });
    g_graph->insert("上海火车站", { 1,3,4 }, 31.250757, 121.452927, { g_graph->indexOf("中山北路") }, { 1 });
    g_graph->insert("汉中路", { 1 }, 31.242947, 121.45418, { g_graph->indexOf("上海火车站") }, { 1 });
    g_graph->insert("新闸路", { 1 }, 31.240488, 121.46374, { g_graph->indexOf("汉中路") }, { 1 });
    g_graph->insert("人民广场", { 1, 2 }, 31.234805, 121.469952, { g_graph->indexOf("新闸路") }, { 1 });
    g_graph->insert("黄陂南路", { 1 }, 31.22476, 121.468717, { g_graph->indexOf("人民广场") }, { 1 });
    g_graph->insert("黄陂南路", { 1 }, 31.22476, 121.468717, { g_graph->indexOf("人民广场") }, { 1 });
    g_graph->insert("陕西南路", { 1 }, 31.217738, 121.454264, { g_graph->indexOf("黄陂南路") }, { 1 });
    g_graph->insert("常熟路", { 1 }, 31.21558, 121.445584, { g_graph->indexOf("陕西南路") }, { 1 });
    g_graph->insert("衡山路", { 1 }, 31.206849, 121.442126, { g_graph->indexOf("常熟路") }, { 1 });
    g_graph->insert("徐家汇", { 1 }, 31.196382, 121.432132, { g_graph->indexOf("衡山路") }, { 1 });
    g_graph->insert("上海体育馆", { 1, 4 }, 31.184332, 121.432371, { g_graph->indexOf("徐家汇") }, { 1 });
    g_graph->insert("漕宝路", { 1 }, 31.170181, 121.43041, { g_graph->indexOf("上海体育馆") }, { 1 });
    g_graph->insert("上海南站", { 1, 3 }, 31.155939, 121.425684, { g_graph->indexOf("漕宝路") }, { 1 });
    g_graph->insert("锦江乐园", { 1 }, 31.144096, 121.409563, { g_graph->indexOf("上海南站") }, { 1 });
    g_graph->insert("莲花路", { 1 }, 31.132708, 121.398158, { g_graph->indexOf("锦江乐园") }, { 1 });
    g_graph->insert("外环路", { 1 }, 31.123071, 121.388591, { g_graph->indexOf("莲花路") }, { 1 });
    g_graph->insert("莘庄", { 1 }, 31.112825, 121.38038, { g_graph->indexOf("外环路") }, { 1 });

    //railway line 2
    g_graph->insert("徐泾东", { 2 }, 31.191167, 121.296606, {  }, { });
    g_graph->insert("虹桥火车站", { 2 }, 31.195913, 121.316973, { g_graph->indexOf("徐泾东") }, { 1 });
    g_graph->insert("虹桥二号航站楼", { 2 }, 31.197857, 121.33034, { g_graph->indexOf("虹桥火车站") }, { 1 });
    g_graph->insert("淞虹路", { 2 }, 31.220115, 121.354602, { g_graph->indexOf("虹桥二号航站楼") }, { 1 });
    g_graph->insert("北新泾", { 2 }, 31.218304, 121.369403, { g_graph->indexOf("淞虹路") }, { 1 });
    g_graph->insert("威宁路", { 2 }, 31.216678, 121.382421, { g_graph->indexOf("北新泾") }, { 1 });
    g_graph->insert("娄山关路", { 2 }, 31.212889, 121.399621, { g_graph->indexOf("威宁路") }, { 1 });
    g_graph->insert("中山公园", { 2, 3, 4 }, 31.219906, 121.41183, { g_graph->indexOf("娄山关路") }, { 1 });
    g_graph->insert("江苏路", { 2 }, 31.222037, 121.426651, { g_graph->indexOf("中山公园") }, { 1 });
    g_graph->insert("静安寺", { 2 }, 31.224904, 121.442854, { g_graph->indexOf("江苏路") }, { 1 });
    g_graph->insert("南京西路", { 2 }, 31.230765, 121.456158, { g_graph->indexOf("静安寺"), g_graph->indexOf("人民广场") }, { 1, 1 });
    g_graph->insert("南京东路", { 2 }, 31.239933, 121.479767, { g_graph->indexOf("人民广场") }, { 1 });
    g_graph->insert("陆家嘴", { 2 }, 31.239995, 121.497778, { g_graph->indexOf("南京东路") }, { 1 });
    g_graph->insert("东昌路", { 2 }, 31.235378, 121.511011, { g_graph->indexOf("陆家嘴") }, { 1 });
    g_graph->insert("世纪大道", { 2, 4 }, 31.231022, 121.522523, { g_graph->indexOf("东昌路") }, { 1 });
    g_graph->insert("上海科技馆", { 2 }, 31.221395, 121.539865, { g_graph->indexOf("世纪大道") }, { 1 });
    g_graph->insert("世纪公园", { 2 }, 31.211731, 121.546477, { g_graph->indexOf("上海科技馆") }, { 1 });
    g_graph->insert("龙阳路", { 2 }, 31.20496, 121.553223, { g_graph->indexOf("世纪公园") }, { 1 });
    g_graph->insert("张江高科", { 2 }, 31.203967, 121.583365, { g_graph->indexOf("龙阳路") }, { 1 });
    g_graph->insert("金科路", { 2 }, 31.206382, 121.597782, { g_graph->indexOf("张江高科") }, { 1 });
    g_graph->insert("广兰路", { 2 }, 31.213204, 121.616363, { g_graph->indexOf("金科路") }, { 1 });
    g_graph->insert("唐镇", { 2 }, 31.215893, 121.651877, { g_graph->indexOf("广兰路") }, { 1 });
    g_graph->insert("创新中路", { 2 }, 31.215511, 121.66962, { g_graph->indexOf("唐镇") }, { 1 });
    g_graph->insert("华夏东路", { 2 }, 31.198916, 121.67658, { g_graph->indexOf("创新中路") }, { 1 });
    g_graph->insert("川沙", { 2 }, 31.188711, 121.693753, { g_graph->indexOf("华夏东路") }, { 1 });
    g_graph->insert("凌空路", { 2 }, 31.194885, 121.719377, { g_graph->indexOf("川沙") }, { 1 });
    g_graph->insert("远东大道", { 2 }, 31.201484, 121.751204, { g_graph->indexOf("凌空路") }, { 1 });
    g_graph->insert("海天三路", { 2 }, 31.170675, 121.792503, { g_graph->indexOf("远东大道") }, { 1 });
    g_graph->insert("浦东国际机场", { 2 }, 31.151413, 121.802256, { g_graph->indexOf("海天三路") }, { 1 });

    //railway line 3
    g_graph->insert("江杨北路", { 3 }, 31.409663, 121.435193, {  }, { });
    g_graph->insert("铁力路", { 3 }, 31.409999, 121.456811, { g_graph->indexOf("江杨北路") }, { 1 });
    g_graph->insert("友谊路", { 3 }, 31.405881, 121.471481, { g_graph->indexOf("铁力路") }, { 1 });
    g_graph->insert("宝杨路", { 3 }, 31.397432, 121.475055, { g_graph->indexOf("友谊路") }, { 1 });
    g_graph->insert("水产路", { 3 }, 31.383247, 121.48364, { g_graph->indexOf("宝杨路") }, { 1 });
    g_graph->insert("淞滨路", { 3 }, 31.372837, 121.48831, { g_graph->indexOf("水产路") }, { 1 });
    g_graph->insert("张华浜", { 3 }, 31.359973, 121.494349, { g_graph->indexOf("淞滨路") }, { 1 });
    g_graph->insert("淞发路", { 3 }, 31.347118, 121.496083, { g_graph->indexOf("张华浜") }, { 1 });
    g_graph->insert("长江南路", { 3 }, 31.333952, 121.487098, { g_graph->indexOf("淞发路") }, { 1 });
    g_graph->insert("殷高西路", { 3 }, 31.321718, 121.48033, { g_graph->indexOf("长江南路") }, { 1 });
    g_graph->insert("江湾镇", { 3 }, 31.307326, 121.480569, { g_graph->indexOf("殷高西路") }, { 1 });
    g_graph->insert("大柏树", { 3 }, 31.291275, 121.478554, { g_graph->indexOf("江湾镇") }, { 1 });
    g_graph->insert("赤峰路", { 3 }, 31.283221, 121.477932, { g_graph->indexOf("大柏树") }, { 1 });
    g_graph->insert("虹口体育场", { 3 }, 31.272819, 121.474747, { g_graph->indexOf("赤峰路") }, { 1 });
    g_graph->insert("东宝兴路", { 3 }, 31.261935, 121.475572, { g_graph->indexOf("虹口体育场") }, { 1 });
    g_graph->insert("宝山路", { 3, 4 }, 31.253462, 121.471965, { g_graph->indexOf("东宝兴路"), g_graph->indexOf("上海火车站") }, { 1, 1 });
    g_graph->insert("中潭路", { 3,4 }, 31.256515, 121.43642, { g_graph->indexOf("上海火车站") }, { 1 });
    g_graph->insert("镇坪路", { 3,4 }, 31.248484, 121.425746, { g_graph->indexOf("中潭路") }, { 1 });
    g_graph->insert("曹杨路", { 3,4 }, 31.240572, 121.413032, { g_graph->indexOf("镇坪路") }, { 1 });
    g_graph->insert("金沙江路", { 3,4 }, 31.233724, 121.408356, { g_graph->indexOf("曹杨路"), g_graph->indexOf("中山公园") }, { 1, 1 });
    g_graph->insert("延安西路", { 3, 4 }, 31.211608, 121.412437, { g_graph->indexOf("中山公园") }, { 1 });
    g_graph->insert("虹桥路", { 3, 4 }, 31.198768, 121.416953, { g_graph->indexOf("延安西路") }, { 1 });
    g_graph->insert("宜山路", { 3, 4 }, 31.188189, 121.42273, { g_graph->indexOf("虹桥路"), g_graph->indexOf("上海体育馆") }, { 1, 1 });
    g_graph->insert("漕溪路", { 3 }, 31.178503, 121.433715, { g_graph->indexOf("宜山路") }, { 1 });
    g_graph->insert("龙漕路", { 3 }, 31.171816, 121.439595, { g_graph->indexOf("漕溪路") }, { 1 });
    g_graph->insert("石龙路", { 3 }, 31.159849, 121.438624, { g_graph->indexOf("龙漕路"), g_graph->indexOf("上海南站") }, { 1, 1 });

    //railway line 4
    g_graph->insert("海伦路", { 4 }, 31.260967, 121.484378, { g_graph->indexOf("宝山路") }, { 1 });
    g_graph->insert("临平路", { 4 }, 31.262932, 121.496581, { g_graph->indexOf("海伦路") }, { 1 });
    g_graph->insert("大连路", { 4 }, 31.260017, 121.508603, { g_graph->indexOf("临平路") }, { 1 });
    g_graph->insert("杨树浦路", { 4 }, 31.253975, 121.512971, { g_graph->indexOf("大连路") }, { 1 });
    g_graph->insert("浦东大道", { 4 }, 31.241961, 121.515107, { g_graph->indexOf("杨树浦路"), g_graph->indexOf("世纪大道") }, { 1, 1 });
    g_graph->insert("浦电路", { 4 }, 31.225019, 121.527683, { g_graph->indexOf("世纪大道") }, { 1 });
    g_graph->insert("蓝村路", { 4 }, 31.213805, 121.523474, { g_graph->indexOf("浦电路") }, { 1 });
    g_graph->insert("塘桥", { 4 }, 31.211585, 121.514446, { g_graph->indexOf("蓝村路") }, { 1 });
    g_graph->insert("南浦大桥", { 4 }, 31.21066, 121.495302, { g_graph->indexOf("塘桥") }, { 1 });
    g_graph->insert("西藏南路", { 4 }, 31.204011, 121.485054, { g_graph->indexOf("南浦大桥") }, { 1 });
    g_graph->insert("鲁班路", { 4 }, 31.200967, 121.46993, { g_graph->indexOf("西藏南路") }, { 1 });
    g_graph->insert("大木桥路", { 4 }, 31.196182, 121.459064, { g_graph->indexOf("鲁班路") }, { 1 });
    g_graph->insert("东安路", { 4 }, 31.192935, 121.45013, { g_graph->indexOf("大木桥路") }, { 1 });
    g_graph->insert("上海体育场", { 4 }, 31.187709, 121.439235, { g_graph->indexOf("东安路"), g_graph->indexOf("上海体育馆") }, { 1, 1 });
}

inline auto g_menu = std::make_unique<Menu>();


