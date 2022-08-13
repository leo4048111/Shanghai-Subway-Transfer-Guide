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

    static inline void MenuItemURL(const char* name_, const char* URL_)
    {
        if (ImGui::MenuItem(name_))
        {
#ifdef _WIN32
            ::ShellExecute(NULL, "open", URL_, NULL, NULL, SW_SHOWDEFAULT);
#else
#if __APPLE__
            const char* open_executable = "open";
#else
            const char* open_executable = "xdg-open";
#endif
            char command[256];
            snprintf(command, 256, "%s \"%s\"", open_executable, path);
            system(command);
#endif
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Open in browser: %s", URL_);
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

    inline void searchForBestTransferRoute(
        ds::Vector<int> transferAt,
        ds::Vector<int> bestTransfer,
        ds::Vector<int>& transferAtResult,
        ds::Vector<int>& bestTransferResult,
        int& minTransfers,
        int curLine,
        int curVexIdx,
        int transferCnt
        )
    {
        if (curVexIdx >= this->routeLen - 1)
        {
            if (transferCnt < minTransfers)
            {
                minTransfers = transferCnt;
                transferAtResult = transferAt;
                bestTransferResult = bestTransfer;
            }
            return;
        }

        auto vex = g_graph->vexAt(route[curVexIdx]);
        ds::Arc* arc = vex.first;
        for (arc; arc != nullptr; arc = arc->next) if (arc->adjVex == route[curVexIdx + 1]) break;
        if (arc == nullptr) {
            for (arc = g_graph->vexAt(route[curVexIdx + 1]).first; arc != nullptr; arc = arc->next) if (arc->adjVex == route[curVexIdx]) break;
        }

        if (arc == nullptr) {
            minTransfers = -1;
            return;
        }

        for (auto line : arc->lineNum)
        {
            if (line != curLine)
            {
                transferAt.push_back(route[curVexIdx]);
                bestTransfer.push_back(line);
                searchForBestTransferRoute(transferAt, bestTransfer, transferAtResult, bestTransferResult, minTransfers, line, curVexIdx + 1, transferCnt + 1);
                transferAt.pop_back();
                bestTransfer.pop_back(); 
            }
            else {
                searchForBestTransferRoute(transferAt, bestTransfer, transferAtResult, bestTransferResult, minTransfers, curLine, curVexIdx + 1, transferCnt);
            }
        }

    }

    inline void printRoute()
    {
        if (route == nullptr) return;
        ds::Vector<int> transferAt;
        ds::Vector<int> bestTransfer;
        int minTransfers = INT_MAX;
        transferAtResult.clear();
        bestTransferResult.clear();
        searchForBestTransferRoute(transferAt, bestTransfer, transferAtResult, bestTransferResult, minTransfers, 0, 0, 0);

        if (minTransfers == 0) {
            LOG(u8"[Info] ����������վ���յ�վ��ͬ�������������\n");
            return;
        }

        if (minTransfers == -1) {
            LOG("[Error] Unexpected error occured while finding best transfer route...\n");
            return;
        }

        LOG("[Info] %s", u8"������ѯ��·����Ϣ���£�\n");
        LOG("[Info] %s: %s%s%d%s\n", u8"���վ", string2UTF8(g_graph->vexAt(transferAtResult[0]).name).c_str(), u8"�ϳ�������", bestTransferResult[0], u8"����");

        for (int i = 1; i < minTransfers; i++)
        {
            LOG("[Info] %s: %s %d%s --> %d%s\n", u8"����վ", string2UTF8(g_graph->vexAt(transferAtResult[i]).name).c_str(), bestTransferResult[i - 1], u8"����", bestTransferResult[i], u8"����");
        }

        LOG("[Info] %s: %s%s%d%s\n", u8"�յ�վ", string2UTF8(g_graph->vexAt(route[routeLen - 1]).name).c_str(), u8"�³���", bestTransferResult[minTransfers - 1], u8"���߳�վ");
        LOG("[Info] %s%d%s\n", u8"�û��˷����ܹ�����", minTransfers - 1, u8"��");
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
    bool shouldDrawRouteCost{ false };
    bool shouldRouteBlink{ false };
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
    ds::Vector<bool> isRailwayLineIgnored;
    ImVec4 transferStationColor{ 0.52f, 0.52f, 0.52f, 1.f };
    ImVec4 routeColor{ 1.f, 0, 0, 1.f };
    ImVec4 graphTextColor{ 1.f, 1.f, 1.f, 1.f };
    float stationMarkRadius{ 5.f };
    float transferStationMarkRadius{ 6.5f };
    float stationMarkThickness{ 1.3f };
    int* route{ nullptr };
    int routeLen{ NULL };
    ds::Vector<int> transferAtResult;
    ds::Vector<int> bestTransferResult;
    ds::Vector<bool> isVexInRoute;

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
    this->isVexInRoute.resize(g_graph->size() + 8, false);

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
            if (ImGui::MenuItem("Arc"))
            {
                memset(selectedAddControlsTab, 0, sizeof(selectedAddControlsTab));
                selectedAddControlsTab[2] = true;
                showAddControls = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("About"))
    {
        MenuItemURL("Github page", "https://github.com/leo4048111/Shanghai-Subway-Transfer-Guide");
        ImGui::EndMenu();
    }

    helpMarker("Tongji University Data Structure course design by 2050250.");
    ImGui::EndMainMenuBar();
}

inline void Menu::renderLog()
{
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowPos(ImVec2(0, 317), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(559, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin(ICON_FA_INFO_CIRCLE "Log output", 0, flags);
    ImGui::End();
    g_log->draw(msyh, ICON_FA_INFO_CIRCLE "Log output");
}

inline void Menu::renderControls()
{
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowPos(ImVec2(0, 18), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(558, 297), ImGuiCond_FirstUseEver);
    ImGui::Begin(ICON_FA_COG"Controls", 0, flags);
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
        if (ImGui::RadioButton("Minimal stations", minimalStations)) minimalStations = !minimalStations;
        ImGui::SameLine();
        if (ImGui::RadioButton("Minimal cost", !minimalStations))minimalStations = !minimalStations;
        if (ImGui::Button(ICON_FA_SEARCH " Find best route."))
        {
            if (this->route != nullptr) {
                free(this->route);
                this->route = nullptr;
            }
            int** mat = nullptr;
            int size = g_graph->asMat(mat, !minimalStations);
            this->routeLen = Dijkstra::Helper::calculate((const int**)mat, size, startStationIdx, terminalStationIdx, this->route);
            for (int i = 0; i < size; i++) free(mat[i]);
            free(mat);
            LOG("[Info] Search strategy: %s\n", minimalStations ? "Minimal transfer stations" : "Minimal cost");
            this->isVexInRoute.clear();
            this->isVexInRoute.resize(g_graph->size() + 8, false);
            for (int i = 0; i < this->routeLen; i++) {
                this->isVexInRoute[route[i]] = true;
            }
            printRoute();
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
        ImGui::SameLine();
        ImGui::Checkbox("Show route cost", &shouldDrawRouteCost);
        ImGui::SameLine();
        ImGui::Checkbox("Blinking route", &shouldRouteBlink);
        ImGui::Separator();
        ImGui::Text("Graph scale:");
        ImGui::SameLine();
        ImGui::SliderFloat("##Graph scale", &graphScale, 2000.f, 4000.f);
        ImGui::BeginChild("Color edit", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NavFlattened);
        ImGui::PushItemWidth(-160);
        ImGui::ColorEdit4("##TransferStationColor", &transferStationColor.x, ImGuiColorEditFlags_AlphaBar);
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::TextUnformatted("Transfer station");
        ImGui::ColorEdit4("##GraphTextColor", &graphTextColor.x, ImGuiColorEditFlags_AlphaBar);
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::TextUnformatted("Graph text");
        ImGui::ColorEdit4("##RouteColor", &routeColor.x, ImGuiColorEditFlags_AlphaBar);
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::TextUnformatted("Route");
        for (int i = 1; i <= g_graph->getTotalLines(); i++) {
            char buf[32];
            sprintf_s(buf, "Line %d", i);
            char buf2[32];
            sprintf_s(buf2, "##%s", buf);
            ImGui::ColorEdit4(buf2, &railwayLineColors[i].x, ImGuiColorEditFlags_AlphaBar);
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::TextUnformatted(buf);
            char buf3[32];
            sprintf_s(buf3, "Ignore##%d", i);
            ImGui::SameLine();
            ImGui::Checkbox(buf3, &this->isRailwayLineIgnored[i]);
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
    ImGui::SetNextWindowSize(ImVec2(501, 362), ImGuiCond_FirstUseEver);
    ImGui::Begin(ICON_FA_USER_COG "Additional controls", &showAddControls, flags);
    ImGui::BeginTabBar("##TabBar");
    static char stationName[256];
    static double latitude = 31.25;
    static double longitude = 121.45;
    static ds::Vector<bool> isLineSelected;
    static ds::Vector<int> selectedStationsIdx;
    static ds::Vector<int> selectedLinesIdx;
    static ds::Vector<int> adjStationsIdx;
    static ds::Vector<int> adjStationsCost;
    static ds::Vector<int> lineNums;
    ds::Vector<int> eraseList;
    if (isLineSelected.size() < textLinesSize) isLineSelected.resize(textLinesSize, false);
    if (ImGui::BeginTabItem("Add station", nullptr, selectedAddControlsTab[0] ? ImGuiTabItemFlags_SetSelected : 0))
    {
        selectedAddControlsTab[0] = false; //reset flag
        ImGui::PushFont(msyh);
        ImGui::PushItemWidth(153.f);
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
        ImGui::PopFont();
        ImGui::BeginChild("##AdjStations", ImVec2(0, 160), true);
        for (int i = 0; i < adjStationsIdx.size(); i++)
        {
            ImGui::PushFont(msyh);
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
            sprintf_s(id3, ICON_FA_TRASH_ALT "Remove##%d", i);
            ImGui::Text("Cost:");
            ImGui::SameLine();
            char id4[32];
            sprintf_s(id4, "##Cost%d", i);
            ImGui::InputInt(id4, &adjStationsCost[i], 0);
            ImGui::SameLine();
            ImGui::PopFont();
            if (ImGui::Button(id3, ImVec2(0, 26))) {
                eraseList.push_back(i);
            }
            ImGui::PopItemWidth();
        }
        ImGui::EndChild();
        ImGui::PopItemWidth();

        for (int i = 0; i < eraseList.size(); i++) {
            selectedStationsIdx.erase(selectedStationsIdx.begin() + eraseList[i]);
            selectedLinesIdx.erase(selectedLinesIdx.begin() + eraseList[i]);
            adjStationsIdx.erase(adjStationsIdx.begin() + eraseList[i]);
        }
        if (ImGui::Button(ICON_FA_PLUS " Add adjacent stations")) {
            selectedLinesIdx.push_back(0);
            selectedStationsIdx.push_back(0);
            if (textStations[selectedLinesIdx.back()][selectedStationsIdx.back()] != nullptr)
                adjStationsIdx.push_back(g_graph->indexOf(UTF82string(textStations[selectedLinesIdx.back()][selectedStationsIdx.back()])));
            adjStationsCost.push_back(1);
        }
        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_SAVE " Save new station")) {
            if (stationName[0] == 0) {
                LOG("[Error] A station name is required...\n");
            }
            else if (lineNums.size() == 0) {
                LOG("[Error] At least 1 line number should be specified...\n");
            }
            else if (adjStationsIdx.size() > 0) {
                if (g_graph->insert(UTF82string(stationName), lineNums, latitude, longitude, adjStationsIdx, adjStationsCost))
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
        int lineNums = g_graph->getTotalLines();
        static int selectedLineIdx = 0;
        static int startStationIdx = 0;
        ImGui::Text("Current rail line count:");
        ImGui::SameLine();
        ImGui::Text("%d", lineNums);
        ImGui::Text("Rail line to be add:");
        ImGui::SameLine();
        ImGui::Text("%d", lineNums + 1);
        ImGui::ColorEdit4("New rail line color", &railwayLineColors[lineNums + 1].x, ImGuiColorEditFlags_AlphaBar);
        ImGui::Separator();
        if (ImGui::BeginChild("##SelectExistingStationAsStart", ImVec2(0, 0), true))
        {
            ImGui::PushFont(msyh);
            ImGui::Text("Select existing station as start station:");
            ImGui::PushItemWidth(233.f);
            ImGui::Combo("##SelectedStartStationLine", &selectedLineIdx, textLines, textLinesSize);
            ImGui::SameLine();
            ImGui::Combo("##SelectedStartStation", &startStationIdx, textStations[selectedLineIdx], textStationsCnts[selectedLineIdx]);
            ImGui::PopItemWidth();
            ImGui::PopFont();
            if (ImGui::Button(ICON_FA_PLUS " Add new line##1")) {
                if (g_graph->addLine(UTF82string(textStations[selectedLineIdx][startStationIdx]), lineNums + 1)) {
                    LOG("[Info] Line %d has been added, %s as start station...", lineNums + 1, textStations[selectedLineIdx][startStationIdx]);
                    updateTexts();
                }
                else
                {
                    LOG("[Error] Unable to add new line...");
                }

            }
            ImGui::Separator();

            static char startStationName[256];
            static double startStationLatitude = 31.25;
            static double startStationLongitude = 121.45;
            ImGui::PushFont(msyh);
            ImGui::Text("Create new station as start station:");
            ImGui::PushItemWidth(153.f);
            ImGui::Text("New station name: ");
            ImGui::SameLine();
            ImGui::InputText("##InputStationName", startStationName, IM_ARRAYSIZE(startStationName));
            ImGui::Text("Latitude:");
            ImGui::SameLine();
            ImGui::InputDouble("##Latitude", &startStationLatitude);
            ImGui::SameLine();
            ImGui::Text("Longitude:");
            ImGui::SameLine();
            ImGui::InputDouble("##Longitude", &startStationLongitude);
            ImGui::PopFont();
            if (ImGui::Button(ICON_FA_PLUS " Add new line##2"))
            {
                if (g_graph->indexOf(UTF82string(startStationName)) != -1)
                {
                    LOG("[Error] Unable to add new line, station name duplicated...");
                }
                else
                {
                    g_graph->insert(UTF82string(startStationName), { lineNums + 1 }, startStationLatitude, startStationLongitude, {}, {});
                    LOG("[Info] Line %d has been added, %s as start station...", lineNums + 1, startStationName);
                    updateTexts();
                }
            }
            ImGui::PopItemWidth();
            ImGui::EndChild();
        }

        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Modify arc", nullptr, selectedAddControlsTab[2] ? ImGuiTabItemFlags_SetSelected : 0))
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
        ImGui::Text("Arc info:");
        ImGui::SameLine();
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
                ZeroMemory((void*)buf[i], sizeof(char) * (str.size() + 1));
                memcpy_s((void*)buf[i], str.size(), str.c_str(), str.size());
                i++;
            }
            ImGui::SameLine();
            if (selectedDstVexIdx >= i) selectedDstVexIdx = 0;
            if (ImGui::Combo("##ModifyArcVex2", &selectedDstVexIdx, buf, i))
                i2 = buf[0] == nullptr ? -1 : g_graph->indexOf(UTF82string(buf[selectedDstVexIdx]));
            i2 = buf[0] == nullptr ? -1 : g_graph->indexOf(UTF82string(buf[selectedDstVexIdx]));
        }
        ImGui::PopFont();
        ImGui::Separator();
        if (ImGui::BeginChild("##ModifyArc", ImVec2(0, 0), true))
        {
            static int newCost = 1;
            ImGui::Text("Update arc cost:");
            ImGui::Text("New cost:");
            ImGui::SameLine();
            ImGui::InputInt("##Cost", &newCost);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_WRENCH "Update"))
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
            ImGui::Separator();
            ImGui::Text("Remove selected arc:");
            ImGui::Text(ICON_FA_INFO " Notice: This operation will remove selected arc.");
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_TRASH_ALT "Remove"))
            {
                if (g_graph->removeArc(i1, i2, selectedLine + 1))
                {
                    LOG("[Info] Line %d arc between %s and %s has been removed...\n", selectedLine + 1, textStations[selectedLine][selectedSrcVexIdx], buf[selectedDstVexIdx]);
                }
                else
                {
                    LOG("[Error] Unable to remove arc...\n");
                }
            }
            ImGui::Separator();
            ImGui::PushFont(msyh);
            ImGui::PushItemWidth(150.f);
            static int connectStationSrcLineIdx = 0;
            static int connectStationDstLineIdx = 0;
            static int connectStationSrcIdx = 0;
            static int connectStationDstIdx = 0;
            static int idx1 = g_graph->indexOf(UTF82string(textStations[connectStationSrcLineIdx][connectStationSrcIdx]));
            static int idx2 = g_graph->indexOf(UTF82string(textStations[connectStationDstLineIdx][connectStationDstIdx]));
            static int connectWithLineNum = 0;
            ImGui::Text("Connect selected stations:");
            ImGui::Text("Station 1:");
            ImGui::SameLine();
            if (ImGui::Combo("##ConnectsArcLine1",  &connectStationSrcLineIdx, textLines, textLinesSize))
            {
                connectStationSrcIdx = 0;
                idx1 = g_graph->indexOf(UTF82string(textStations[connectStationSrcLineIdx][connectStationSrcIdx]));
            }
            ImGui::SameLine();
            if (ImGui::Combo("##ConnectsArcStation1", &connectStationSrcIdx, textStations[connectStationSrcLineIdx], textStationsCnts[connectStationSrcLineIdx]))
            {
                idx1 = g_graph->indexOf(UTF82string(textStations[connectStationSrcLineIdx][connectStationSrcIdx]));
            }
            ImGui::Text("Station 2:");
            ImGui::SameLine();
            if (ImGui::Combo("##ConnectsArcLine2", &connectStationDstLineIdx, textLines, textLinesSize))
            {
                connectStationDstIdx = 0;
                idx2 = g_graph->indexOf(UTF82string(textStations[connectStationDstLineIdx][connectStationDstIdx]));
            }
            ImGui::SameLine();
            if (ImGui::Combo("##ConnectsArcStation2", &connectStationDstIdx, textStations[connectStationDstLineIdx], textStationsCnts[connectStationDstLineIdx]))
            {
                idx2 = g_graph->indexOf(UTF82string(textStations[connectStationDstLineIdx][connectStationDstIdx]));
            }
            ImGui::Text("Connect with railway line: ");
            ImGui::SameLine();
            ImGui::Combo("##ConnectWithLine", &connectWithLineNum, textLines, textLinesSize);
            ImGui::PopFont();
            ImGui::Text(ICON_FA_INFO " Notice: This operation will connect selected stations.");
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_EDIT "Connect")) {
                if (g_graph->connect(idx1, idx2, connectWithLineNum + 1))
                {
                    LOG("[Info] Line %d arc between %s and %s has been connected...\n", connectWithLineNum + 1, textStations[selectedLine][selectedSrcVexIdx], buf[selectedDstVexIdx]);
                }
                else
                {
                    LOG("[Error] Arc has existed, unable to connect...\n");
                }
            }
            ImGui::PopItemWidth();
            ImGui::EndChild();
        }

        ImGui::PopItemWidth();
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
    ImGui::Begin(ICON_FA_SOLAR_PANEL "Canvas", 0, windowFlags);
    if (ImGui::Button("0.50x")) { zoomScale = 0.50f;}
    ImGui::SameLine();
    if (ImGui::Button("1.00x")) { zoomScale = 1.00f;}
    ImGui::SameLine();
    if (ImGui::Button("1.25x")) { zoomScale = 1.25;}
    ImGui::SameLine();
    if (ImGui::Button("2.00x")) { zoomScale = 2.00;}
    ImGui::SameLine();
    ImGui::Text(ICON_FA_MINUS);
    ImGui::SameLine();
    if (ImGui::SliderFloat("##Zoom", &zoomScale, 0.25f, 5.00f)) {}
    ImGui::SameLine();
    ImGui::Text(ICON_FA_PLUS);
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
    static float counter = 1;
    counter += 3.5f;
    if (counter >= 2 * 255) counter = 1;
    float routeMarkerAlpha = ((counter > 255) ? 255 * 2 - counter : counter) / 255.f;
    for (int i = 0; i < g_graph->size(); i++)
    {
        auto vex = g_graph->vexAt(i);
        bool shouldIgnore = true;
        for (auto line : vex.lineNum) shouldIgnore &= isRailwayLineIgnored[line];
        if (shouldIgnore) continue;
        ImVec2 src((vex.coord_x - SH_LONGITUDE) * ZOOM(graphScale) + canvasOrigin.x, -(vex.coord_y - SH_LATITUDE) * ZOOM(graphScale) + canvasOrigin.y);
        if (!g_graph->isTransfer(i)) {
            drawList->AddCircle(src, ZOOM(stationMarkRadius), ImGui::ColorConvertFloat4ToU32(railwayLineColors[vex.lineNum[0]]), 0, ZOOM(stationMarkThickness));
        }
        else {
            drawList->AddCircle(src, ZOOM(transferStationMarkRadius), ImGui::ColorConvertFloat4ToU32(transferStationColor), 0, ZOOM(stationMarkThickness));
            drawList->AddLine(
                ImVec2(src.x - cos(45.f * M_PI / 180.f) * ZOOM(transferStationMarkRadius), src.y - sin(45.f * M_PI / 180.f) * ZOOM(transferStationMarkRadius)),
                ImVec2(src.x + cos(45.f * M_PI / 180.f) * ZOOM(transferStationMarkRadius), src.y + sin(45.f * M_PI / 180.f) * ZOOM(transferStationMarkRadius)),
                ImGui::ColorConvertFloat4ToU32(transferStationColor));
            drawList->AddLine(
                ImVec2(src.x - cos(45.f * M_PI / 180.f) * ZOOM(transferStationMarkRadius), src.y + sin(45.f * M_PI / 180.f) * ZOOM(transferStationMarkRadius)),
                ImVec2(src.x + cos(45.f * M_PI / 180.f) * ZOOM(transferStationMarkRadius), src.y - sin(45.f * M_PI / 180.f) * ZOOM(transferStationMarkRadius)),
                ImGui::ColorConvertFloat4ToU32(transferStationColor));
        }
        drawList->AddText(msyh, ZOOM(10.f), src, ImGui::ColorConvertFloat4ToU32(graphTextColor), string2UTF8(vex.name).c_str());
        for (auto arc = vex.first; arc != nullptr; arc = arc->next)
        {
            if (arc->adjVex < 0) continue;
            auto adjVex = g_graph->vexAt(arc->adjVex);
            ImVec2 dst((adjVex.coord_x - SH_LONGITUDE) * ZOOM(graphScale) + canvasOrigin.x, -(adjVex.coord_y - SH_LATITUDE) * ZOOM(graphScale) + canvasOrigin.y);
            ImVec4 lineColor(NULL, NULL, NULL, NULL);
            bool isSrcInRoute = false;
            bool isDstInRoute = false;
            if (shouldDrawRoute && route != nullptr) {
                isSrcInRoute = this->isVexInRoute[i];
                isDstInRoute = this->isVexInRoute[arc->adjVex];
            }
            float modifierAngle = arc->lineNum.size() > 1 ? 0.25f : 0.f;
            for (auto lineNum : arc->lineNum)
            {
                if (this->isRailwayLineIgnored[lineNum] == true) continue;
                lineColor = shouldDrawRoute && isSrcInRoute && isDstInRoute ? routeColor : railwayLineColors[lineNum];
                lineColor.w = (shouldDrawRoute && isSrcInRoute && isDstInRoute && shouldRouteBlink) ? routeMarkerAlpha : lineColor.w;
                float lineWeight = shouldDrawRoute && isSrcInRoute && isDstInRoute ? 2.f : 1.5f;
                double angle = atan((src.y - dst.y) / (src.x - dst.x));
                drawList->AddLine(
                    ImVec2(src.x + (src.x > dst.x ? -1 : 1) * ZOOM(stationMarkRadius + stationMarkThickness) * cos(angle - modifierAngle), src.y + (src.x > dst.x ? -1 : 1) * ZOOM(stationMarkRadius + stationMarkThickness) * sin(angle - modifierAngle)),
                    ImVec2(dst.x + (src.x < dst.x ? -1 : 1) * ZOOM(stationMarkRadius + stationMarkThickness) * cos(angle + modifierAngle), dst.y + (src.x < dst.x ? -1 : 1) * ZOOM(stationMarkRadius + stationMarkThickness) * sin(angle + modifierAngle)),
                    ImGui::ColorConvertFloat4ToU32(lineColor),
                    ZOOM(lineWeight));
                modifierAngle = -modifierAngle;
            }

            if (shouldDrawRouteCost) {
                char buf[32];
                sprintf_s(buf, "%d", arc->cost);
                drawList->AddText(ImVec2((src.x + dst.x) / 2, (src.y + dst.y) / 2), ImGui::ColorConvertFloat4ToU32(graphTextColor), buf);
            }
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
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromMemoryCompressedTTF(fa_data, fa_size, 10.f, &icons_config, icons_ranges);
    karlaRegular = io.Fonts->AddFontFromMemoryCompressedTTF((void*)karla_regular_data, karla_regular_size, 200.0f, NULL);
    cousineRegular = io.Fonts->AddFontFromMemoryCompressedTTF((void*)cousine_regular_data, cousine_regular_size, 200.0f, NULL);
    msyh = io.Fonts->AddFontFromMemoryCompressedTTF((void*)msyh_data, msyh_size, 20.f, NULL, io.Fonts->GetGlyphRangesChineseFull());

    //color styles
    ImGui::StyleColorsDark();
    this->railwayLineColors.resize(20, {1.f, 1.f, 1.f, 1.f});
    railwayLineColors[1] = { 0.81, 0.10, 0.10, 0.90 };  //line 1
    railwayLineColors[2] = { 0.10, 0.64, 0.10, 0.90 };  //line 2
    railwayLineColors[3] = { 0.98, 0.88, 0.01, 0.90 };  //line 3
    railwayLineColors[4] = { 0.46, 0.10, 0.46, 0.90 };  //line 4
    railwayLineColors[5] = { 0.82, 0.10, 0.82, 0.90 };  //line 5
    railwayLineColors[6] = { 1.00, 0.28, 0.46, 0.90 };  //line 6
    railwayLineColors[7] = { 1.00, 0.50, 0.00, 0.90 };  //line 7
    railwayLineColors[8] = { 0.00, 0.40, 0.80, 0.90 };  //line 8
    railwayLineColors[9] = { 0.58, 0.83, 0.86, 0.90 };  //line 9
    railwayLineColors[10] = { 0.79, 0.65, 0.84, 0.90 };  //line 10
    railwayLineColors[11] = { 0.50, 0.00, 0.00, 0.90 };  //line 11
    railwayLineColors[12] = { 0.05, 0.47, 0.37, 0.90 };  //line 12
    railwayLineColors[13] = { 0.91, 0.59, 0.76, 0.90 };  //line 13
    this->isRailwayLineIgnored.resize(20, false);
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
    else if (textLinesSize < totalLines) {
        const char** p = (const char**)realloc(textLines, sizeof(const char*) * totalLines);
        if (p != nullptr) textLines = p;
        ZeroMemory(textLines, sizeof(const char*) * totalLines);
    }
    if (textLines == nullptr) return;
    textLinesSize = totalLines;
    for (int i = 0; i < totalLines; i++)
    {
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        sprintf_s(buffer, "%d����", i + 1);
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
    g_graph->insert("����·", { 1 }, 31.394206, 121.419948, {  }, { });
    g_graph->insert("������·", { 1 }, 31.383264, 121.423247, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("������·", { 1 }, 31.371644, 121.426297, { g_graph->indexOf("������·") }, { 1 });
    g_graph->insert("�����´�", { 1 }, 31.356997, 121.429383, { g_graph->indexOf("������·") }, { 1 });
    g_graph->insert("����·", { 1 }, 31.341434, 121.43311, { g_graph->indexOf("�����´�") }, { 1 });
    g_graph->insert("ͨ���´�", { 1 }, 31.333256, 121.436764, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����·", { 1 }, 31.320818, 121.44242, { g_graph->indexOf("ͨ���´�") }, { 1 });
    g_graph->insert("�����´�", { 1 }, 31.308528, 121.444073, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("��ˮ·", { 1 }, 31.294405, 121.445543, { g_graph->indexOf("�����´�") }, { 1 });
    g_graph->insert("�Ϻ���Ϸ��", { 1 }, 31.28144, 121.447488, { g_graph->indexOf("��ˮ·") }, { 1 });
    g_graph->insert("�ӳ�·", { 1 }, 31.273658, 121.450876, { g_graph->indexOf("�Ϻ���Ϸ��") }, { 1 });
    g_graph->insert("��ɽ��·", { 1 }, 31.261058, 121.454577, { g_graph->indexOf("�ӳ�·") }, { 1 });
    g_graph->insert("�Ϻ���վ", { 1,3,4 }, 31.250757, 121.452927, { g_graph->indexOf("��ɽ��·") }, { 1 });
    g_graph->insert("����·", { 1, 12, 13 }, 31.242947, 121.45418, { g_graph->indexOf("�Ϻ���վ") }, { 1 });
    g_graph->insert("��բ·", { 1 }, 31.240488, 121.46374, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����㳡", { 1, 2, 8 }, 31.234805, 121.469952, { g_graph->indexOf("��բ·") }, { 1 });
    g_graph->insert("������·", { 1 }, 31.22476, 121.468717, { g_graph->indexOf("����㳡") }, { 1 });
    g_graph->insert("������·", { 1 }, 31.22476, 121.468717, { g_graph->indexOf("����㳡") }, { 1 });
    g_graph->insert("������·", { 1, 10, 12 }, 31.217738, 121.454264, { g_graph->indexOf("������·") }, { 1 });
    g_graph->insert("����·", { 1, 7 }, 31.21558, 121.445584, { g_graph->indexOf("������·") }, { 1 });
    g_graph->insert("��ɽ·", { 1 }, 31.206849, 121.442126, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("��һ�", { 1, 9, 11 }, 31.196382, 121.432132, { g_graph->indexOf("��ɽ·") }, { 1 });
    g_graph->insert("�Ϻ�������", { 1, 4 }, 31.184332, 121.432371, { g_graph->indexOf("��һ�") }, { 1 });
    g_graph->insert("�·", { 1, 12 }, 31.170181, 121.43041, { g_graph->indexOf("�Ϻ�������") }, { 1 });
    g_graph->insert("�Ϻ���վ", { 1, 3 }, 31.155939, 121.425684, { g_graph->indexOf("�·") }, { 1 });
    g_graph->insert("������԰", { 1 }, 31.144096, 121.409563, { g_graph->indexOf("�Ϻ���վ") }, { 1 });
    g_graph->insert("����·", { 1 }, 31.132708, 121.398158, { g_graph->indexOf("������԰") }, { 1 });
    g_graph->insert("�⻷·", { 1 }, 31.123071, 121.388591, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("ݷׯ", { 1, 5 }, 31.112825, 121.38038, { g_graph->indexOf("�⻷·") }, { 1 });

    //railway line 2
    g_graph->insert("������", { 2 }, 31.191167, 121.296606, {  }, { });
    g_graph->insert("���Ż�վ", { 2, 10 }, 31.195913, 121.316973, { g_graph->indexOf("������") }, { 1 });
    g_graph->insert("���Ŷ��ź�վ¥", { 2, 10 }, 31.197857, 121.33034, { g_graph->indexOf("���Ż�վ") }, { 1 });
    g_graph->insert("����·", { 2 }, 31.220115, 121.354602, { g_graph->indexOf("���Ŷ��ź�վ¥") }, { 1 });
    g_graph->insert("������", { 2 }, 31.218304, 121.369403, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����·", { 2 }, 31.216678, 121.382421, { g_graph->indexOf("������") }, { 1 });
    g_graph->insert("¦ɽ��·", { 2 }, 31.212889, 121.399621, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("��ɽ��԰", { 2, 3, 4 }, 31.219906, 121.41183, { g_graph->indexOf("¦ɽ��·") }, { 1 });
    g_graph->insert("����·", { 2, 11 }, 31.222037, 121.426651, { g_graph->indexOf("��ɽ��԰") }, { 1 });
    g_graph->insert("������", { 2, 7 }, 31.224904, 121.442854, { g_graph->indexOf("����·"), g_graph->indexOf("����·") }, { 1, 1 });
    g_graph->insert("�Ͼ���·", { 2, 12, 13 }, 31.230765, 121.456158, { g_graph->indexOf("������"), g_graph->indexOf("����㳡"), g_graph->indexOf("������·"), g_graph->indexOf("����·")}, {1, 1, 1, 1});
    g_graph->insert("�Ͼ���·", { 2, 10 }, 31.239933, 121.479767, { g_graph->indexOf("����㳡") }, { 1 });
    g_graph->insert("½����", { 2 }, 31.239995, 121.497778, { g_graph->indexOf("�Ͼ���·") }, { 1 });
    g_graph->insert("����·", { 2 }, 31.235378, 121.511011, { g_graph->indexOf("½����") }, { 1 });
    g_graph->insert("���ʹ��", { 2, 4, 6, 9 }, 31.231022, 121.522523, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("�Ϻ��Ƽ���", { 2 }, 31.221395, 121.539865, { g_graph->indexOf("���ʹ��") }, { 1 });
    g_graph->insert("���͹�԰", { 2 }, 31.211731, 121.546477, { g_graph->indexOf("�Ϻ��Ƽ���") }, { 1 });
    g_graph->insert("����·", { 2, 7 }, 31.20496, 121.553223, { g_graph->indexOf("���͹�԰") }, { 1 });
    g_graph->insert("�Ž��߿�", { 2 }, 31.203967, 121.583365, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("���·", { 2 }, 31.206382, 121.597782, { g_graph->indexOf("�Ž��߿�") }, { 1 });
    g_graph->insert("����·", { 2 }, 31.213204, 121.616363, { g_graph->indexOf("���·") }, { 1 });
    g_graph->insert("����", { 2 }, 31.215893, 121.651877, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("������·", { 2 }, 31.215511, 121.66962, { g_graph->indexOf("����") }, { 1 });
    g_graph->insert("���Ķ�·", { 2 }, 31.198916, 121.67658, { g_graph->indexOf("������·") }, { 1 });
    g_graph->insert("��ɳ", { 2 }, 31.188711, 121.693753, { g_graph->indexOf("���Ķ�·") }, { 1 });
    g_graph->insert("���·", { 2 }, 31.194885, 121.719377, { g_graph->indexOf("��ɳ") }, { 1 });
    g_graph->insert("Զ�����", { 2 }, 31.201484, 121.751204, { g_graph->indexOf("���·") }, { 1 });
    g_graph->insert("������·", { 2 }, 31.170675, 121.792503, { g_graph->indexOf("Զ�����") }, { 1 });
    g_graph->insert("�ֶ����ʻ���", { 2 }, 31.151413, 121.802256, { g_graph->indexOf("������·") }, { 1 });

    //railway line 3
    g_graph->insert("���·", { 3 }, 31.409663, 121.435193, {  }, { });
    g_graph->insert("����·", { 3 }, 31.409999, 121.456811, { g_graph->indexOf("���·") }, { 1 });
    g_graph->insert("����·", { 3 }, 31.405881, 121.471481, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����·", { 3 }, 31.397432, 121.475055, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("ˮ��·", { 3 }, 31.383247, 121.48364, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����·", { 3 }, 31.372837, 121.48831, { g_graph->indexOf("ˮ��·") }, { 1 });
    g_graph->insert("�Ż��", { 3 }, 31.359973, 121.494349, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����·", { 3 }, 31.347118, 121.496083, { g_graph->indexOf("�Ż��") }, { 1 });
    g_graph->insert("������·", { 3 }, 31.333952, 121.487098, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("�����·", { 3 }, 31.321718, 121.48033, { g_graph->indexOf("������·") }, { 1 });
    g_graph->insert("������", { 3 }, 31.307326, 121.480569, { g_graph->indexOf("�����·") }, { 1 });
    g_graph->insert("�����", { 3 }, 31.291275, 121.478554, { g_graph->indexOf("������") }, { 1 });
    g_graph->insert("���·", { 3 }, 31.283221, 121.477932, { g_graph->indexOf("�����") }, { 1 });
    g_graph->insert("�������", { 3, 8 }, 31.272819, 121.474747, { g_graph->indexOf("���·") }, { 1 });
    g_graph->insert("������·", { 3 }, 31.261935, 121.475572, { g_graph->indexOf("�������") }, { 1 });
    g_graph->insert("��ɽ·", { 3, 4 }, 31.253462, 121.471965, { g_graph->indexOf("������·"), g_graph->indexOf("�Ϻ���վ") }, { 1, 1 });
    g_graph->insert("��̶·", { 3,4 }, 31.256515, 121.43642, { g_graph->indexOf("�Ϻ���վ") }, { 1 });
    g_graph->insert("��ƺ·", { 3,4, 7 }, 31.248484, 121.425746, { g_graph->indexOf("��̶·") }, { 1 });
    g_graph->insert("����·", { 3,4, 11 }, 31.240572, 121.413032, { g_graph->indexOf("��ƺ·") }, { 1 });
    g_graph->insert("��ɳ��·", { 3,4, 13 }, 31.233724, 121.408356, { g_graph->indexOf("����·"), g_graph->indexOf("��ɽ��԰") }, { 1, 1 });
    g_graph->insert("�Ӱ���·", { 3, 4 }, 31.211608, 121.412437, { g_graph->indexOf("��ɽ��԰") }, { 1 });
    g_graph->insert("����·", { 3, 4, 10 }, 31.198768, 121.416953, { g_graph->indexOf("�Ӱ���·") }, { 1 });
    g_graph->insert("��ɽ·", { 3, 4, 9 }, 31.188189, 121.42273, { g_graph->indexOf("����·"), g_graph->indexOf("�Ϻ�������"), g_graph->indexOf("��һ�") }, { 1, 1, 1 });
    g_graph->insert("��Ϫ·", { 3 }, 31.178503, 121.433715, { g_graph->indexOf("��ɽ·") }, { 1 });
    g_graph->insert("����·", { 3, 12 }, 31.171816, 121.439595, { g_graph->indexOf("��Ϫ·"), g_graph->indexOf("�·")}, {1, 1});
    g_graph->insert("ʯ��·", { 3 }, 31.159849, 121.438624, { g_graph->indexOf("����·"), g_graph->indexOf("�Ϻ���վ") }, { 1, 1 });

    //railway line 4
    g_graph->insert("����·", { 4, 10 }, 31.260967, 121.484378, { g_graph->indexOf("��ɽ·") }, { 1 });
    g_graph->insert("��ƽ·", { 4 }, 31.262932, 121.496581, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����·", { 4, 12 }, 31.260017, 121.508603, { g_graph->indexOf("��ƽ·") }, { 1 });
    g_graph->insert("������·", { 4 }, 31.253975, 121.512971, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("�ֶ����", { 4 }, 31.241961, 121.515107, { g_graph->indexOf("������·"), g_graph->indexOf("���ʹ��") }, { 1, 1 });
    g_graph->insert("�ֵ�·", { 4 }, 31.225019, 121.527683, { g_graph->indexOf("���ʹ��") }, { 1 });
    g_graph->insert("����·", { 4, 6 }, 31.213805, 121.523474, { g_graph->indexOf("�ֵ�·") }, { 1 });
    g_graph->insert("����", { 4 }, 31.211585, 121.514446, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("���ִ���", { 4 }, 31.21066, 121.495302, { g_graph->indexOf("����") }, { 1 });
    g_graph->insert("������·", { 4, 8 }, 31.204011, 121.485054, { g_graph->indexOf("���ִ���") }, { 1 });
    g_graph->insert("³��·", { 4 }, 31.200967, 121.46993, { g_graph->indexOf("������·") }, { 1 });
    g_graph->insert("��ľ��·", { 4, 12 }, 31.196182, 121.459064, { g_graph->indexOf("³��·") }, { 1 });
    g_graph->insert("����·", { 4, 7 }, 31.192935, 121.45013, { g_graph->indexOf("��ľ��·") }, { 1 });
    g_graph->insert("�Ϻ�������", { 4 }, 31.187709, 121.439235, { g_graph->indexOf("����·"), g_graph->indexOf("�Ϻ�������") }, { 1, 1 });

    //railway line 5
    g_graph->insert("����·", { 5 }, 31.100213, 121.381092, { g_graph->indexOf("ݷׯ") }, { 1 });
    g_graph->insert("����·", { 5 }, 31.09124, 121.385647, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("���", { 5 }, 31.068858, 121.397278, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����", { 5 }, 31.046977, 121.405387, { g_graph->indexOf("���") }, { 1 });
    g_graph->insert("����·", { 5 }, 31.028423, 121.411866, { g_graph->indexOf("����") }, { 1 });
    g_graph->insert("����·", { 5 }, 31.020163, 121.415259, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("��ƽ·", { 5 }, 31.012995, 121.405438, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����·", { 5 }, 31.009276, 121.390423, { g_graph->indexOf("��ƽ·") }, { 1 });
    g_graph->insert("�ľ�·", { 5 }, 31.005506, 121.376092, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("���п�����", { 5 }, 31.00263, 121.365281, { g_graph->indexOf("�ľ�·") }, { 1 });

    //railway line 6
    g_graph->insert("�۳�·", { 6 }, 31.355152, 121.570457, {}, {});
    g_graph->insert("����ű�˰����", { 6 }, 31.34991, 121.582726, { g_graph->indexOf("�۳�·") }, { 1 });
    g_graph->insert("����·", { 6 }, 31.337531, 121.589784, { g_graph->indexOf("����ű�˰����") }, { 1 });
    g_graph->insert("����ű�˰����", { 6 }, 31.323628, 121.597877, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("�޺�·", { 6 }, 31.314593, 121.585186, { g_graph->indexOf("����ű�˰����") }, { 1 });
    g_graph->insert("���޴��", { 6 }, 31.304869, 121.585121, { g_graph->indexOf("�޺�·") }, { 1 });
    g_graph->insert("����·", { 6 }, 31.293053, 121.584609, { g_graph->indexOf("���޴��") }, { 1 });
    g_graph->insert("�޷�·", { 6, 12 }, 31.282686, 121.584813, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����·", { 6 }, 31.27407, 121.583861, { g_graph->indexOf("�޷�·") }, { 1 });
    g_graph->insert("����·", { 6 }, 31.266123, 121.582728, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����·", { 6 }, 31.259359, 121.577679, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("��ɽ·", { 6 }, 31.252556, 121.568647, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("��ƽ·", { 6 }, 31.247456, 121.559901, { g_graph->indexOf("��ɽ·") }, { 1 });
    g_graph->insert("������·", { 6 }, 31.241374, 121.548532, { g_graph->indexOf("��ƽ·") }, { 1 });
    g_graph->insert("����·", { 6 }, 31.237982, 121.53917, { g_graph->indexOf("������·") }, { 1 });
    g_graph->insert("Դ����������", { 6 }, 31.235052, 121.530063, { g_graph->indexOf("����·"), g_graph->indexOf("���ʹ��") }, { 1, 1 });
    g_graph->insert("�ֵ�·(6����)", { 6 }, 31.223019, 121.517683, { g_graph->indexOf("���ʹ��"),g_graph->indexOf("����·") }, { 1, 1 });
    g_graph->insert("�Ϻ���ͯҽѧ����", { 6 }, 31.205584, 121.518892, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("�����´�", { 6 }, 31.195272, 121.512116, { g_graph->indexOf("�Ϻ���ͯҽѧ����") }, { 1 });
    g_graph->insert("�߿���·", { 6, 7 }, 31.187616, 121.505442, { g_graph->indexOf("�����´�") }, { 1 });
    g_graph->insert("����·", { 6, 13 }, 31.174587, 121.506541, { g_graph->indexOf("�߿���·") }, { 1 });
    g_graph->insert("����·", { 6 }, 31.161589, 121.511326, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("������·", { 6 }, 31.151917, 121.510198, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����·", { 6 }, 31.151197, 121.501943, { g_graph->indexOf("������·") }, { 1 });
    g_graph->insert("������·", { 6 }, 31.150741, 121.490633, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("������������", { 6, 8, 11 }, 31.155594, 121.475962, { g_graph->indexOf("������·") }, { 1 });

    //railway line 7
    g_graph->insert("������", { 7 }, 31.40376, 121.345311, {}, {});
    g_graph->insert("�����´�", { 7 }, 31.390292, 121.35306, { g_graph->indexOf("������") }, { 1 });
    g_graph->insert("�˹�·", { 7 }, 31.366106, 121.351151, { g_graph->indexOf("�����´�") }, { 1 });
    g_graph->insert("����", { 7 }, 31.359487, 121.357679, { g_graph->indexOf("�˹�·") }, { 1 });
    g_graph->insert("�˴幫԰", { 7 }, 31.346723, 121.368061, { g_graph->indexOf("����") }, { 1 });
    g_graph->insert("�·", { 7 }, 31.324265, 121.368921, { g_graph->indexOf("�˴幫԰") }, { 1 });
    g_graph->insert("�Ϻ���ѧ", { 7 }, 31.32231, 121.384059, { g_graph->indexOf("�·") }, { 1 });
    g_graph->insert("�ϳ�·", { 7 }, 31.323111, 121.394025, { g_graph->indexOf("�Ϻ���ѧ") }, { 1 });
    g_graph->insert("�ϴ�·", { 7 }, 31.31735, 121.403586, { g_graph->indexOf("�ϳ�·") }, { 1 });
    g_graph->insert("����·", { 7 }, 31.305674, 121.409034, { g_graph->indexOf("�ϴ�·") }, { 1 });
    g_graph->insert("����", { 7 }, 31.295427, 121.411826, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("��֪·", { 7 }, 31.286787, 121.41699, { g_graph->indexOf("����") }, { 1 });
    g_graph->insert("����·", { 7 }, 31.275751, 121.418416, { g_graph->indexOf("��֪·") }, { 1 });
    g_graph->insert("�´�·", { 7 }, 31.265724, 121.417955, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("᰸�·", { 7 }, 31.258152, 121.417252, { g_graph->indexOf("�´�·"), g_graph->indexOf("��ƺ·") }, { 1, 1 });
    g_graph->insert("����·", { 7, 13 }, 31.242202, 121.433703, { g_graph->indexOf("��ƺ·") }, { 1 });
    g_graph->insert("��ƽ·", { 7 }, 31.235973, 121.4377, { g_graph->indexOf("����·"), g_graph->indexOf("������") }, { 1, 1 });
    g_graph->insert("�ؼ��·", { 7, 9 }, 31.201015, 121.445428, { g_graph->indexOf("����·"), g_graph->indexOf("����·"), g_graph->indexOf("��һ�") }, { 1, 1, 1 });
    g_graph->insert("������·", { 7, 12 }, 31.18696, 121.452591, { g_graph->indexOf("����·"), g_graph->indexOf("��ľ��·")}, {1, 1});
    g_graph->insert("��̲", { 7 }, 31.173924, 121.468995, { g_graph->indexOf("������·") }, { 1 });
    g_graph->insert("����·", { 7, 13 }, 31.176563, 121.481596, { g_graph->indexOf("��̲") }, { 1 });
    g_graph->insert("ҫ��·", { 7, 8 }, 31.180446, 121.490084, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("��̨·", { 7 }, 31.184151, 121.496255, { g_graph->indexOf("ҫ��·"), g_graph->indexOf("�߿���·") }, { 1, 1 });
    g_graph->insert("�����·", { 7 }, 31.189653, 121.520775, { g_graph->indexOf("�߿���·") }, { 1 });
    g_graph->insert("����·", { 7 }, 31.189665, 121.535675, { g_graph->indexOf("�����·") }, { 1 });
    g_graph->insert("����·", { 7 }, 31.195344, 121.545798, { g_graph->indexOf("����·"), g_graph->indexOf("����·") }, { 1, 1 });
    g_graph->insert("��ľ·", { 7 }, 31.21338, 121.558553, { g_graph->indexOf("����·") }, { 1 });

    //railway line 8
    g_graph->insert("�й�·", { 8 }, 31.324489, 121.5277, {}, {});
    g_graph->insert("�۽�·", { 8 }, 31.316903, 121.527743, { g_graph->indexOf("�й�·") }, { 1 });
    g_graph->insert("����·", { 8 }, 31.30701, 121.527666, { g_graph->indexOf("�۽�·") }, { 1 });
    g_graph->insert("���˹�԰", { 8 }, 31.297925, 121.528878, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("�Ӽ���·", { 8 }, 31.29072, 121.530609, { g_graph->indexOf("���˹�԰") }, { 1 });
    g_graph->insert("����·", { 8 }, 31.280733, 121.52356, { g_graph->indexOf("�Ӽ���·") }, { 1 });
    g_graph->insert("����·", { 8 }, 31.277108, 121.513847, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("��ɽ�´�", { 8 }, 31.275213, 121.505109, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("��ƽ·", { 8, 10 }, 31.277128, 121.497154, { g_graph->indexOf("��ɽ�´�") }, { 1 });
    g_graph->insert("����·", { 8 }, 31.278436, 121.485988, { g_graph->indexOf("��ƽ·"), g_graph->indexOf("�������") }, { 1, 1 });
    g_graph->insert("���ر�·", { 8 }, 31.265396, 121.464192, { g_graph->indexOf("�������") }, { 1 });
    g_graph->insert("����·", { 8 }, 31.255033, 121.464344, { g_graph->indexOf("���ر�·") }, { 1 });
    g_graph->insert("����·", { 8, 12 }, 31.244303, 121.466936, { g_graph->indexOf("����·"), g_graph->indexOf("����㳡"), g_graph->indexOf("����·")}, {1, 1, 1});
    g_graph->insert("������", { 8 }, 31.229326, 121.474794, { g_graph->indexOf("����㳡") }, { 1 });
    g_graph->insert("������", { 8, 10 }, 31.220879, 121.478399, { g_graph->indexOf("������") }, { 1 });
    g_graph->insert("½���·", { 8, 9 }, 31.213618, 121.48163, { g_graph->indexOf("������"), g_graph->indexOf("������·") }, { 1, 1 });
    g_graph->insert("�л�������", { 8 }, 31.187694, 121.489702, { g_graph->indexOf("ҫ��·"), g_graph->indexOf("������·") }, { 1, 1 });
    g_graph->insert("��ɽ·", { 8, 13 }, 31.172839, 121.49169, { g_graph->indexOf("ҫ��·"), g_graph->indexOf("����·"), g_graph->indexOf("����·")}, {1, 1, 1});
    g_graph->insert("��˼", { 8 }, 31.163076, 121.489252, { g_graph->indexOf("��ɽ·"), g_graph->indexOf("������������") }, { 1, 1 });
    g_graph->insert("�����´�", { 8 }, 31.14307, 121.485232, { g_graph->indexOf("������������") }, { 1 });
    g_graph->insert("«��·", { 8 }, 31.12129, 121.493307, { g_graph->indexOf("�����´�") }, { 1 });
    g_graph->insert("�ֽ���", { 8 }, 31.098627, 121.501859, { g_graph->indexOf("«��·") }, { 1 });
    g_graph->insert("����·", { 8 }, 31.086367, 121.504169, { g_graph->indexOf("�ֽ���") }, { 1 });
    g_graph->insert("����·", { 8 }, 31.076042, 121.506176, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("��Ź�·", { 8 }, 31.063455, 121.50786, { g_graph->indexOf("����·") }, { 1 });

    //railway line 9
    g_graph->insert("�ɽ���վ", { 9 }, 30.987173, 121.22672, {}, {});
    g_graph->insert("��׳�", { 9 }, 31.003195, 121.224878, { g_graph->indexOf("�ɽ���վ") }, { 1 });
    g_graph->insert("�ɽ���������", { 9 }, 31.018253, 121.226183, { g_graph->indexOf("��׳�") }, { 1 });
    g_graph->insert("�ɽ��³�", { 9 }, 31.032406, 121.226332, { g_graph->indexOf("�ɽ���������") }, { 1 });
    g_graph->insert("�ɽ���ѧ��", { 9 }, 31.056227, 121.228275, { g_graph->indexOf("�ɽ��³�") }, { 1 });
    g_graph->insert("����", { 9 }, 31.086556, 121.22606, { g_graph->indexOf("�ɽ���ѧ��") }, { 1 });
    g_graph->insert("��ɽ", { 9 }, 31.106758, 121.225289, { g_graph->indexOf("����") }, { 1 });
    g_graph->insert("����", { 9 }, 31.120395, 121.256226, { g_graph->indexOf("��ɽ") }, { 1 });
    g_graph->insert("��ͤ", { 9 }, 31.139471, 121.314445, { g_graph->indexOf("����") }, { 1 });
    g_graph->insert("�д�·", { 9 }, 31.151647, 121.333779, { g_graph->indexOf("��ͤ") }, { 1 });
    g_graph->insert("�߱�", { 9 }, 31.157398, 121.344952, { g_graph->indexOf("�д�·") }, { 1 });
    g_graph->insert("����·", { 9 }, 31.1601, 121.364371, { g_graph->indexOf("�߱�") }, { 1 });
    g_graph->insert("�ϴ�·", { 9 }, 31.168453, 121.379958, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("�����������", { 9 }, 31.172343, 121.392966, { g_graph->indexOf("�ϴ�·") }, { 1 });
    g_graph->insert("����·", { 9 }, 31.176772, 121.413872, { g_graph->indexOf("�����������"), g_graph->indexOf("��ɽ·") }, { 1, 1 });
    g_graph->insert("����·", { 9, 12 }, 31.20481, 121.456213, { g_graph->indexOf("�ؼ��·"), g_graph->indexOf("��ľ��·"), g_graph->indexOf("������·")}, {1, 1, 1});
    g_graph->insert("����·", { 9 }, 31.208092, 121.464334, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("��·", { 9, 13 }, 31.211369, 121.472233, { g_graph->indexOf("����·"), g_graph->indexOf("½���·")}, { 1, 1 });
    g_graph->insert("С����", { 9 }, 31.218989, 121.493818, { g_graph->indexOf("½���·") }, { 1 });
    g_graph->insert("�̳�·", { 9 }, 31.232424, 121.511903, { g_graph->indexOf("С����"), g_graph->indexOf("���ʹ��")}, { 1, 1 });
    g_graph->insert("�����·", { 9 }, 31.229584, 121.544318, { g_graph->indexOf("���ʹ��") }, { 1 });

    //railway line 10
    g_graph->insert("����·", { 10 }, 31.16762, 121.350155, { }, { });
    g_graph->insert("����·", { 10 }, 31.171678, 121.359812, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("�����´�", { 10 }, 31.179115, 121.36579, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("��Ϫ·", { 10 }, 31.196086, 121.375236, { g_graph->indexOf("�����´�") }, { 1 });
    g_graph->insert("ˮ��·", { 10 }, 31.201175, 121.387217, { g_graph->indexOf("��Ϫ·") }, { 1 });
    g_graph->insert("����·", { 10 }, 31.200578, 121.399493, { g_graph->indexOf("ˮ��·") }, { 1 });
    g_graph->insert("��԰·", { 10 }, 31.19845, 121.407306, { g_graph->indexOf("����·"), g_graph->indexOf("����·")}, {1, 1});
    g_graph->insert("��ͨ��ѧ", { 10, 11 }, 31.204057, 121.429925, { g_graph->indexOf("����·"), g_graph->indexOf("��һ�"), g_graph->indexOf("����·")}, {1, 1, 1});
    g_graph->insert("�Ϻ�ͼ���", { 10 }, 31.209953, 121.439737, { g_graph->indexOf("��ͨ��ѧ"), g_graph->indexOf("������·")}, {1, 1});
    g_graph->insert("�����", { 10, 13 }, 31.21801, 121.470609, { g_graph->indexOf("������·"), g_graph->indexOf("������"), g_graph->indexOf("��·")}, {1, 1, 1});
    g_graph->insert("ԥ԰", { 10 }, 31.230204, 121.482809, { g_graph->indexOf("�Ͼ���·"), g_graph->indexOf("������"), g_graph->indexOf("��·")}, {1, 1, 1});
    g_graph->insert("����·", { 10, 12 }, 31.246281, 121.477698, { g_graph->indexOf("�Ͼ���·"), g_graph->indexOf("����·")}, {1, 1});
    g_graph->insert("�Ĵ���·", { 10 }, 31.253923, 121.479698, { g_graph->indexOf("����·"), g_graph->indexOf("����·")}, {1, 1});
    g_graph->insert("�ʵ��´�", { 10 }, 31.270503, 121.489801, { g_graph->indexOf("����·"), g_graph->indexOf("��ƽ·")}, {1, 1});
    g_graph->insert("ͬ�ô�ѧ", { 10 }, 31.28456, 121.502036, { g_graph->indexOf("��ƽ·") }, { 1 });
    g_graph->insert("��Ȩ·", { 10 }, 31.291634, 121.505786, { g_graph->indexOf("ͬ�ô�ѧ") }, { 1 });
    g_graph->insert("��ǳ�", { 10 }, 31.300087, 121.510234, { g_graph->indexOf("��Ȩ·") }, { 1 });
    g_graph->insert("����������", { 10 }, 31.305023, 121.509669, { g_graph->indexOf("��ǳ�") }, { 1 });
    g_graph->insert("����·", { 10 }, 31.314943, 121.503668, { g_graph->indexOf("����������") }, { 1 });
    g_graph->insert("��߶�·", { 10 }, 31.324067, 121.502133, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("�½����", { 10 }, 31.330775, 121.502144, { g_graph->indexOf("��߶�·") }, { 1 });
    g_graph->insert("����һ�ź�վ¥", { 10 }, 31.193287, 121.342856, { g_graph->indexOf("���Ŷ��ź�վ¥") }, { 1 });
    g_graph->insert("�Ϻ�����԰", { 10 }, 31.19218, 121.362421, { g_graph->indexOf("����һ�ź�վ¥"), g_graph->indexOf("��Ϫ·")}, {1, 1});

    //railway line 11
    g_graph->insert("��ʿ��", { 11 }, 31.143585, 121.664058, { }, { });
    g_graph->insert("���¹�·", { 11 }, 31.132619, 121.613039, { g_graph->indexOf("��ʿ��")}, {1});
    g_graph->insert("����·", { 11 }, 31.140102, 121.59458, { g_graph->indexOf("���¹�·") }, { 1 });
    g_graph->insert("��ɽ·", { 11 }, 31.155534, 121.589014, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����", { 11 }, 31.160612, 121.566723, { g_graph->indexOf("��ɽ·") }, { 1 });
    g_graph->insert("����·", { 11 }, 31.153062, 121.534824, { g_graph->indexOf("����") }, { 1 });
    g_graph->insert("���ֶ�", { 11 }, 31.148612, 121.518813, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����", { 11 }, 31.135038, 121.506602, { g_graph->indexOf("���ֶ�"), g_graph->indexOf("������������")}, {1, 1});
    g_graph->insert("��ҫ·", { 11 }, 31.16195, 121.454981, { g_graph->indexOf("������������") }, { 1 });
    g_graph->insert("�ƽ�·", { 11 }, 31.1683, 121.45391, { g_graph->indexOf("��ҫ·") }, { 1 });
    g_graph->insert("����", { 11, 12 }, 31.174903, 121.448222, { g_graph->indexOf("�ƽ�·"), g_graph->indexOf("����·"), g_graph->indexOf("������·")}, {1, 1, 1});
    g_graph->insert("�Ϻ���Ӿ��", { 11 }, 31.180894, 121.437013, { g_graph->indexOf("����"), g_graph->indexOf("��һ�") }, { 1, 1 });
    g_graph->insert("¡��·", { 11, 13 }, 31.232292, 121.418873, { g_graph->indexOf("����·"), g_graph->indexOf("����·"), g_graph->indexOf("��ɳ��·") }, {1, 1, 1});
    g_graph->insert("����·", { 11 }, 31.243798, 121.406395, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����", { 11 }, 31.252412, 121.402647, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("�Ϻ���վ", { 11 }, 31.264456, 121.396762, { g_graph->indexOf("����") }, { 1 });
    g_graph->insert("����԰", { 11 }, 31.270706, 121.385317, { g_graph->indexOf("�Ϻ���վ") }, { 1 });
    g_graph->insert("����ɽ·", { 11 }, 31.273387, 121.371288, { g_graph->indexOf("����԰") }, { 1 });
    g_graph->insert("����·", { 11 }, 31.278923, 121.360149, { g_graph->indexOf("����ɽ·") }, { 1 });
    g_graph->insert("�����´�", { 11 }, 31.283593, 121.344832, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����", { 11 }, 31.298914, 121.318821, { g_graph->indexOf("�����´�") }, { 1 });
    g_graph->insert("���蹫·", { 11 }, 31.30844, 121.30243, { g_graph->indexOf("����") }, { 1 });
    g_graph->insert("��½", { 11 }, 31.32134, 121.271772, { g_graph->indexOf("���蹫·") }, { 1 });
    g_graph->insert("�ζ��³�", { 11 }, 31.33206, 121.249994, { g_graph->indexOf("��½") }, { 1 });
    g_graph->insert("�Ϻ�������", { 11 }, 31.333791, 121.221681, { g_graph->indexOf("�ζ��³�") }, { 1 });
    g_graph->insert("������·", { 11 }, 31.295534, 121.195899, { g_graph->indexOf("�Ϻ�������") }, { 1 });
    g_graph->insert("�Ϻ�������", { 11 }, 31.287248, 121.176316, { g_graph->indexOf("������·") }, { 1 });
    g_graph->insert("��ͤ", { 11 }, 31.290317, 121.157537, { g_graph->indexOf("�Ϻ�������") }, { 1 });
    g_graph->insert("�׷�·", { 11 }, 31.290732, 121.145505, { g_graph->indexOf("��ͤ") }, { 1 });
    g_graph->insert("����·", { 11 }, 31.29754, 121.113012, { g_graph->indexOf("�׷�·") }, { 1 });
    g_graph->insert("����", { 11 }, 31.300411, 121.099522, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����·", { 11 }, 31.347582, 121.240775, { g_graph->indexOf("�ζ��³�") }, { 1 });
    g_graph->insert("�ζ���", { 11 }, 31.379074, 121.22347, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("�ζ���", { 11 }, 31.393442, 121.233052, { g_graph->indexOf("�ζ���") }, { 1 });

    //railway line 12
    g_graph->insert("��ݷ·", { 12 }, 31.133958, 121.358644, {}, {});
    g_graph->insert("��ݷ·", { 12 }, 31.13918, 121.374741, {g_graph->indexOf("��ݷ·")}, {1});
    g_graph->insert("�˴�·", { 12 }, 31.142806, 121.387743, { g_graph->indexOf("��ݷ·") }, { 1 });
    g_graph->insert("����·", { 12 }, 31.157041, 121.387535, { g_graph->indexOf("�˴�·") }, { 1 });
    g_graph->insert("��÷·", { 12 }, 31.16209, 121.3928, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("���ֹ�԰", { 12 }, 31.168655, 121.414034, { g_graph->indexOf("��÷·"), g_graph->indexOf("�·")}, {1,1});
    g_graph->insert("���ʿ�������", { 12 }, 31.252153, 121.493789, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("������", { 12 }, 31.255494, 121.502248, { g_graph->indexOf("����·"), g_graph->indexOf("����·")}, {1, 1});
    g_graph->insert("���ֹ�԰", { 12 }, 31.266669, 121.519398, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����·", { 12 }, 31.27077, 121.528263, { g_graph->indexOf("���ֹ�԰") }, { 1 });
    g_graph->insert("¡��·", { 12 }, 31.277205, 121.540105, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("����·", { 12 }, 31.282128, 121.548757, { g_graph->indexOf("¡��·") }, { 1 });
    g_graph->insert("���˵�", { 12 }, 31.283002, 121.557186, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("��½·", { 12 }, 31.284771, 121.574572, { g_graph->indexOf("���˵�"), g_graph->indexOf("�޷�·")}, {1, 1});
    g_graph->insert("��߱�·", { 12 }, 31.282274, 121.598668, { g_graph->indexOf("�޷�·") }, { 1 });
    g_graph->insert("��·", { 12 }, 31.281933, 121.611422, { g_graph->indexOf("��߱�·") }, { 1 });
    g_graph->insert("�꽭·", { 12 }, 31.282377, 121.623002, { g_graph->indexOf("��·")}, { 1 });
    g_graph->insert("��·", { 12 }, 31.265203, 121.634441, { g_graph->indexOf("�꽭·") }, { 1 });

    //railway line 13
    g_graph->insert("����·", { 13 }, 31.242938, 121.315083, {}, {});
    g_graph->insert("��ɳ����·", { 13 }, 31.243042, 121.330692, { g_graph->indexOf("����·") }, { 1 });
    g_graph->insert("��ׯ", { 13 }, 31.244322, 121.351051, { g_graph->indexOf("��ɳ����·") }, { 1 });
    g_graph->insert("����ɽ��·", { 13 }, 31.239276, 121.362944, { g_graph->indexOf("��ׯ") }, { 1 });
    g_graph->insert("�汱·", { 13 }, 31.234045, 121.377633, { g_graph->indexOf("����ɽ��·") }, { 1 });
    g_graph->insert("��ɺ�·", { 13 }, 31.233621, 121.390468, { g_graph->indexOf("�汱·"), g_graph->indexOf("��ɳ��·")}, {1, 1});
    g_graph->insert("����·", { 13 }, 31.235778, 121.425611, { g_graph->indexOf("¡��·"), g_graph->indexOf("����·")}, {1, 1});
    g_graph->insert("����·", { 13 }, 31.246048, 121.439528, { g_graph->indexOf("����·"), g_graph->indexOf("����·") }, { 1, 1 });
    g_graph->insert("��Ȼ�����", { 13 }, 31.237706, 121.458217, { g_graph->indexOf("����·"), g_graph->indexOf("�Ͼ���·") }, { 1, 1 });
    g_graph->insert("������·", { 13 }, 31.221992, 121.459614, { g_graph->indexOf("�����"), g_graph->indexOf("�Ͼ���·") }, { 1, 1 });
    g_graph->insert("�����Ჩ���", { 13 }, 31.198825, 121.477448, { g_graph->indexOf("��·") }, { 1 });
    g_graph->insert("�������", { 13 }, 31.184457, 121.47988, { g_graph->indexOf("�����Ჩ���"), g_graph->indexOf("����·")}, {1, 1});

}

inline auto g_menu = std::make_unique<Menu>();


