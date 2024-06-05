#pragma once
#ifndef __PHOTON_KD_TREE_HPP__
#define __PHOTON_KD_TREE_HPP__
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

	struct NearestPhotonsHandler {
		Vec3 X;
		float max_r2;
		priority_queue<Photon*> q;
	};

	//直接利用数组存储完全二叉树，完全二叉树数组索引从1开始，光子数组索引从0开始
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
		}

		int BuildKdTree(int l, int r, int axis);

		void GetNearestNPhotons(NearestPhotonsHandler& handler);

	};
}
#endif