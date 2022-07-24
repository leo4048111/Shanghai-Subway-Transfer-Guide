#pragma once

#include <string>
#include "Vector.hpp"

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
		Vertex(std::string name, uint32_t lineNum, double x, double y, Vector<int> adjVexes, Vector<int> costs) : name(name), lineNum(lineNum), coord_x(x), coord_y(y) {
			Arc* pArc = nullptr;
			for (uint32_t i = 0; i < adjVexes.size(); i++) pArc = new Arc(adjVexes[i], costs[i], pArc);
			this->first = pArc;
		};

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
		uint32_t lineNum{ NULL };
		double coord_x{ 0.f };
		double coord_y{ 0.f };
		Arc* first{ nullptr };
	};

	class SubwayGraph
	{
	public:
		SubwayGraph() = default;
		~SubwayGraph() = default;

		bool insert(std::string name, uint32_t lineNum, double latitude, double longitude, Vector<int> adjVexes, Vector<int> costs) {
			if (indexOf(name) != -1) return false; //duplication check
			for (auto elem : adjVexes)
				if (elem >= vertexes.size()) return false;  //invalid idx check
			vertexes.push_back(Vertex(name, lineNum, longitude, latitude, adjVexes, costs));
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
			for (int i = 0; i < vertexes.size(); i++)
			{
				if (vertexes[i].name == name) return i;
			}

			return -1;
		}

		Vertex vexAt(const int idx) {
			return vertexes[idx];
		}

		const size_t asMat(int**& mat) const {
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
					mat[arc->adjVex][i] = arc->cost;
					mat[i][arc->adjVex] = arc->cost;
					arc = arc->next;
				}
			}

			return size;
		}

		int size() const {
			return this->vertexes.size();
		}

		int getTotalLines() const {
			return this->numLines;
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

		void printMat() {
			int** mat = nullptr;
			auto size = asMat(mat);
			if (size <= 0) return;
			for (uint32_t i = 0; i < size; i++)
			{
				for (uint32_t j = 0; j < size; j++)
				{
					std::cout << mat[i][j] << ' ';
				}
				std::cout << std::endl;
			}

			for (int i = 0; i < size; i++) free(mat[i]);
			free(mat);
		}
#endif

	private:
		Vector<Vertex> vertexes;

		int numLines{ 18 };
	};
}

inline auto g_graph = std::make_unique<ds::SubwayGraph>();