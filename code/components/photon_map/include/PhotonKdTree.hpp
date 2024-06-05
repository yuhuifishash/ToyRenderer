#pragma once
#ifndef __PHOTON_KD_TREE_HPP__
#define __PHOTON_KD_TREE_HPP__
#include <queue>

namespace PhotonMap
{
	class Photon;

	struct PhotonKdTreeNode
	{
		int axis = 3; //���ֵ�Ϊx��->0, y��->1, z��->2  Ҷ�ӽڵ�->3
		int ls = -1, rs = -1;
		Photon* P = nullptr;
	};

	struct NearestPhotonsHandler {
		Vec3 X;
		float max_r2;
		priority_queue<Photon*> q;
	};

	//ֱ����������洢��ȫ����������ȫ����������������1��ʼ����������������0��ʼ
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