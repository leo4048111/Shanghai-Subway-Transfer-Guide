#pragma once

#include <string>
#include "Vector.hpp"
#include "HashMap.hpp"

namespace ds
{
	struct Arc
	{
		Arc(int adjVex, int cost, Arc* next) :adjVex(adjVex), cost(cost), next(next) {};
		int adjVex;
		int cost;
		Arc* next{ nullptr };
	};

	struct Vertex
	{
		Vertex(std::string name, ds::Vector<int> lineNum, double x, double y, Vector<int> adjVexes, Vector<int> costs) : name(name), coord_x(x), coord_y(y) {
			this->lineNum = lineNum;
			Arc* pArc = nullptr;
			for (uint32_t i = 0; i < adjVexes.size(); i++) pArc = new Arc(adjVexes[i], costs[i], pArc);
			this->first = pArc;
		};

		inline Vertex& operator=(const Vertex& src) {
			this->name.resize(src.name.size());
			for (int i = 0; i < src.name.size(); i++) this->name[i] = src.name[i];
			this->lineNum = src.lineNum;
			this->coord_x = src.coord_x;
			this->coord_y = src.coord_y;
			this->first = src.first;
			return *this;
		}

		~Vertex() = default; //pls use destroy() instead so that arcs arent automatically deconstructed..

		void destroy() {
			auto arc = this->first;
			auto next = arc->next;
			while (arc != nullptr)
			{
				next = arc->next;
				free(arc);
				arc = next;
			}
		}

		std::string name{ "" };
		ds::Vector<int> lineNum;
		double coord_x{ 0.f };
		double coord_y{ 0.f };
		Arc* first{ nullptr };
	};

	class SubwayGraph
	{
	public:
		SubwayGraph() = default;
		~SubwayGraph() = default;

		bool insert(const char* name, ds::Vector<int> lineNum, double latitude, double longitude, Vector<int> adjVexes, Vector<int> costs) {
			if (indexOf(name) != -1) return false; //duplication check
			for (auto elem : adjVexes)
				if (elem >= vertexes.size()) return false;  //invalid idx check
			vertexes.push_back(Vertex(name, lineNum, longitude, latitude, adjVexes, costs));
			idxMap.insert(name, vertexes.size() - 1);
			return true;
		};

		//this function implementation is wrong, dont use!!!
		bool remove(std::string name) {
			int idx = indexOf(name);
			if (idx == -1) return false;
			vertexes[idx].destroy();
			vertexes.erase(&vertexes[idx]);
			return true;
		}

		int indexOf(const std::string name) {
			int ret = -1;
			if (idxMap.find(name, ret)) return ret;

			return -1;
		}

		Vertex vexAt(const int idx) {
			return vertexes[idx];
		}

		const bool updateArcCost(int i1, int i2, int newCost)
		{
			if (i1 == -1 || i2 == -1) return false;
			for (auto arc = vertexes[i1].first; arc != nullptr; arc = arc->next) {
				if (arc->adjVex == i2) arc->cost = newCost;
			}

			for (auto arc = vertexes[i2].first; arc != nullptr; arc = arc->next) {
				if (arc->adjVex == i1) arc->cost = newCost;
			}

			return true;
		}

		const size_t asMat(int**& mat, bool isWeighted) const {
			const int size = vertexes.size();
			if (size <= 0) return 0;
			if (mat == nullptr)
				mat = (int**)malloc(size * sizeof(int*));
			if (mat == nullptr) return 0;
			for (int i = 0; i < size; i++) {
				mat[i] = (int*)malloc(size * sizeof(int));
				if (mat[i] == nullptr) return 0;
				memset(mat[i], 0, size * sizeof(int));
			}
			for (int i = 0; i < size; i++)
			{
				auto arc = vertexes[i].first;
				while (arc != nullptr)
				{
					mat[arc->adjVex][i] = isWeighted ? arc->cost : 1;
					mat[i][arc->adjVex] = isWeighted ? arc->cost : 1;
					arc = arc->next;
				}
			}

			return size;
		}

		int size() const {
			return this->vertexes.size();
		}

		int getTotalLines() const {
			ds::Vector<int> vec;
			for (int i = 0; i < vertexes.size(); i++) {
				for(int j = 0; j < vertexes[i].lineNum.size(); j++)
				if (vec.find(vertexes[i].lineNum[j]) == vec.end()) vec.push_back(vertexes[i].lineNum[j]);
			}
			return vec.size();
		}

#ifdef _DEBUG
		void print() {
			for (int i = 0; i < this->vertexes.size(); i++)
			{
				std::cout << vertexes[i].name << ' ';
				auto arc = vertexes[i].first;
				while (arc != nullptr)
				{
					std::cout << arc->adjVex << ' ';
					arc = arc->next;
				}
				std::cout << std::endl;
			}
		}
#endif

	private:
		ds::Vector<Vertex> vertexes;
		ds::HashMap<std::string, int> idxMap;
	};
}

inline auto g_graph = std::make_unique<ds::SubwayGraph>();