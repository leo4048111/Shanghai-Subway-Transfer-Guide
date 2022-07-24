#pragma once

#include <iostream>
#include "MinHeap.hpp"

namespace Dijkstra
{
	struct HeapElemWrapper
	{
		HeapElemWrapper(int idx, int cost) : idx(idx), cost(cost) {};
		int idx;
		int cost;

		bool operator < (const HeapElemWrapper& hew) const {
			return this->cost < hew.cost;
		}

		bool operator > (const HeapElemWrapper& hew) const {
			return this->cost > hew.cost;
		}
	};

	class Helper
	{
	private:
		Helper() = delete; //INCONSTRUCTIBLE
		Helper(Helper&&) = delete;
		Helper(const Helper&) = delete;
		Helper& operator =(const Helper&) = delete;

	public:
		static int calculate(const int** mat, const size_t size, const int origin, const int dst, int*& route) //returns route length and route
		{
			if (size <= 0) return 0; //size check

			ds::MinHeap<HeapElemWrapper> minHeap;
			minHeap.insert(HeapElemWrapper(origin, 0));

			bool* isVisited = (bool*)malloc((size + 1) * sizeof(bool));
			if (isVisited != nullptr)
				memset(isVisited, 0, (size + 1) * sizeof(bool));
			else return 0;

			int* minDis = (int*)malloc((size + 1) * sizeof(int));
			if (minDis != nullptr)
				for (uint32_t i = 0; i < size; i++) minDis[i] = INT_MAX;
			else return 0;
			minDis[origin] = 0;

			int** routes = (int**)malloc((size + 1) * sizeof(int*));
			if (routes != nullptr)
				for (uint32_t i = 0; i <= size; i++)
				{
					routes[i] = (int*)malloc((size + 1) * sizeof(int));
					if (routes[i] != nullptr)
						memset(routes[i], -1, (size + 1) * sizeof(int));
					else return 0;
				}
			else return 0;

			int* routeLength = (int*)malloc((size + 1) * sizeof(int));
			memset(routeLength, 0, (size + 1) * sizeof(int));
			routeLength[origin] = 1;
			routes[origin][0] = origin;

			while (!minHeap.empty()) {
				auto curNode = minHeap.front(); 
				minHeap.pop();
				isVisited[curNode.idx] = true;
				for (uint32_t i = 0; i < size; i++)
				{
					if ((mat[curNode.idx][i] > 0) && !isVisited[i])  //is connected and not visited
					{
						int dis = curNode.cost + mat[curNode.idx][i];
						if (dis < minDis[i])
						{
							minDis[i] = dis;
							memcpy_s(routes[i], routeLength[curNode.idx] * sizeof(int), routes[curNode.idx], routeLength[curNode.idx] * sizeof(int)); //update route
							routeLength[i] = routeLength[curNode.idx] + 1;
							routes[i][routeLength[i] - 1] = i;
						}
						minHeap.insert(HeapElemWrapper(i, minDis[i]));
					}
				}
			}

			int routeLen = routeLength[dst];

			if (route == nullptr)
				route = (int*)malloc(routeLen * sizeof(int));

			memcpy_s(route, routeLen * sizeof(int), routes[dst], routeLen * sizeof(int));  //save route
			//some cleanup
			for (uint32_t i = 0; i <= size; i++) free(routes[i]);
			free(routes);
			free(routeLength);
			free(isVisited);
			free(minDis);

			return routeLen;
		}
	};
}