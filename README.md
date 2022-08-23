# 上海地铁换乘指南(Shanghai-Subway-Transfer-Guide)
数据结构课程设计，实现上海地铁换乘指南(A transfer guide for Shanghai Subway, implemented with Dear ImGui.)
## 题目要求（Problem description）
**上海的地铁交通网路已基本成型，建成的地铁线十多条，站点上百个，现需建立
一个换乘指南打印系统，通过输入起点站和终点站，打印出地铁换乘指南，指南内容包
括起点站、换乘站、终点站。  
（1）图形化显示地铁网络结构，能动态添加地铁线路和地铁站点。  
（2）根据输入起点站和终点站，显示地铁换乘指南。  
（3）通过图形界面显示乘车路径。**
## 运行平台（Platform）
（1）操作系统（OS）：Windows11 Pro build 22000.795  
（2）开发语言（Language）：C++(ISO C++17 Standard)  
（3）集成开发环境（IDE）：Visual Studio 2022  
（4）Windows 软件开发包版本（Windows SDK Version）：10.0 (latest installed version)  
（6）图形框架（GUI）：Dear ImGui  
（5）渲染器（Renderer）：OpenGL2 + GLFW 平台  
（6）编译器（Compiler）：MSVC v143 32bit  
## 数据结构与算法（DS&Algorithm）
使用邻接表存储图，使用迪杰斯特拉(Dijkstra)算法查找最优换乘路径，使用回溯法查找长度相同路径中换乘最少的路径。
## 功能（Features）
+ 添加站点（Add stations）
+ 添加线路（Add railway lines）
+ 修改站间连接线信息（Modify arc）
+ 不同查询策略的最短换乘路径（Search for best transfer route）
+ 其它自定义功能（Miscellaneous features, find out urself）
## 截图（Screenshot）
## License
All 3rd-party assets and libraries used in this project retain all rights under their respective licenses.
