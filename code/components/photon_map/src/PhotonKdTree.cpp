#include "PhotonMap.hpp"

namespace PhotonMap
{
	void PhotonMap::BuildKdTree() {
		KdTree = new PhotonKdTree(PhotonNum, PhotonM);
		int rt = KdTree->BuildKdTree(0, PhotonNum - 1, 0);
		KdTree->root = rt;
	}

	static int GetNextAxis(int axis) {
		if (axis <= 1) {
			return axis + 1;
		}
		else {
			return 0;
		}
	}
	int PhotonKdTree::BuildKdTree(int l, int r, int axis) {
		if (l > r) {
			return -1;
		}
		int x = ++NodeNum;
		int mid = (l + r) / 2;
		//[l,r]的中位数
		nth_element(Vp.begin() + l, Vp.begin() + mid, Vp.begin() + r + 1,[&](Photon* x, Photon* y) {
			return x->Pos[axis] < y->Pos[axis];
		});
		T[x].axis = axis;
		T[x].P = Vp[mid];

		T[x].ls = BuildKdTree(l, mid - 1, GetNextAxis(axis));
		T[x].rs = BuildKdTree(mid + 1, r, GetNextAxis(axis));

		return x;
	}
}