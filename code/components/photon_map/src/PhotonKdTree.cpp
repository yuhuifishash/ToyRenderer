#include "PhotonMap.hpp"

namespace PhotonMap
{
	PhotonKdTree::PhotonKdTree(int pNum, Photon* Pns) {
		PhotonNum = pNum;
		T = new PhotonKdTreeNode[pNum + 5ll];
		for (int i = 0; i < pNum; ++i) {
			Vp.push_back(Pns + i);
		}
	}

	void PhotonMap::BuildKdTree() {
		KdTree = new PhotonKdTree(PhotonNum, PhotonM);
		int rt = KdTree->BuildKdTree(0, PhotonNum - 1, 0);
		KdTree->root = rt;
	}

	bool pqnode::operator < (const pqnode& x) const {
		return glm::dot(X - P->Pos, X - P->Pos) < glm::dot(X - x.P->Pos, X - x.P->Pos);
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
		/*cout << mid << "     ";
		for (auto x : Vp) {
			cout << x->Pos << " ";
		}cout << "\n";*/
		T[x].axis = axis;
		T[x].P = Vp[mid];

		T[x].ls = BuildKdTree(l, mid - 1, GetNextAxis(axis));
		T[x].rs = BuildKdTree(mid + 1, r, GetNextAxis(axis));

		return x;
	}

	float GetDistance(const Vec3& X, const Vec3& Ax, int axis) {
		return X[axis] - Ax[axis];
	}

	void PhotonKdTree::GetNearestNPhotons(NearestPhotonsHandler& handler, int idx) {
		if (idx >= PhotonNum || idx == -1) {
			return;
		}
		auto now = T[idx];
		float d_tmp = GetDistance(handler.X, now.P->Pos, now.axis);
		float d_tmp2 = d_tmp * d_tmp;
		if (d_tmp < 0) {//X在分界线左边
			GetNearestNPhotons(handler, now.ls);
			//判断是否有必要往右子树递归
			if (d_tmp2 < handler.max_r2) {
				GetNearestNPhotons(handler, now.rs);
			}

		}
		else {//X在分界线右边
			GetNearestNPhotons(handler, now.rs);
			if (d_tmp2 < handler.max_r2) {
				GetNearestNPhotons(handler, now.ls);
			}
		}

		//考虑当前节点的光子
		float dis = glm::dot(now.P->Pos - handler.X, now.P->Pos - handler.X);
		if (dis < handler.max_r2) {
			handler.q.push(pqnode(handler.X,now.P));
			if (handler.q.size() > handler.N) { // 光子数超过N
				handler.q.pop();
				const auto& t = handler.q.top();
				handler.max_r2 = glm::dot(t.X - t.P->Pos, t.X - t.P->Pos);
			}
		}

	}
}