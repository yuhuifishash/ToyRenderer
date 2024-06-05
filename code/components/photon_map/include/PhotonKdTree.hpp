#pragma once
#ifndef __PHOTON_KD_TREE_HPP__
#define __PHOTON_KD_TREE_HPP__
#include "scene/Scene.hpp"
#include <queue>

namespace PhotonMap
{
	class Photon;

	struct PhotonKdTreeNode
	{
		int axis = 3; //划分的为x轴->0, y轴->1, z轴->2  叶子节点->3
		int ls = -1, rs = -1;
		Photon* P = nullptr;
	};

	struct pqnode {
		Vec3 X;
		Photon* P;
		pqnode(Vec3& x, Photon* p) {
			X = x;
			P = p;
		}
		bool operator < (const pqnode& x) const;
	};

	struct NearestPhotonsHandler {
		Vec3 X;
		int N;
		float max_r2;
		priority_queue<pqnode> q;
	};

	class PhotonKdTree 
	{
		int NodeNum = -1;
		int PhotonNum;
		PhotonKdTreeNode* T;
		vector<Photon*> Vp;
	public:
		int root = -1;
		PhotonKdTree(int pNum, Photon* Pns) {
			PhotonNum = pNum;
			T = new PhotonKdTreeNode[pNum+5ll];
			for (int i = 0; i < pNum; ++i) {
				Vp.push_back(Pns);
			}
		}
		~PhotonKdTree() {
			delete[]T;
			Vp.clear();
		}

		int BuildKdTree(int l, int r, int axis);

		void GetNearestNPhotons(NearestPhotonsHandler& handler, int idx);

		void PrintKdTree(int idx, int space);
	};
}
#endif