#include "server/Server.hpp"
#include "PhotonMap.hpp"

namespace PhotonMap
{
	void PhotonMap::PrintPhotonMap() {
		for (int i = 0; i < PhotonNum; ++i) {
			auto p = PhotonM[i];
			cout << p.Pos << "\n";
		}
	}
    struct pdnode {
        int photon_index;
        float d2;
        bool operator<(const pdnode& x)const {
            return d2 < x.d2;
        }
    };

    //<photons, max_r2>
    tuple<std::vector<Photon*>, float> PhotonMap::GetNearestNPhotonsDebug(const Vec3& pos, int N, float R2) {
        //we use brute_force first
        std::vector<pdnode> V;
        for (int i = 0; i < PhotonNum; ++i) {
            auto p = PhotonM[i];
            float d2 = glm::dot(pos - p.Pos, pos - p.Pos);
            if (d2 <= R2) {
                V.push_back({ i,d2 });
            }
        }
        if (V.size() > N) {
            sort(V.begin(), V.end());
        }
        std::vector<Photon*> res;
        float max_r2;
        for (int i = 0; i < min((size_t)N, V.size()); ++i) {
            res.push_back(&(PhotonM[V[i].photon_index]));
            max_r2 = max(max_r2, V[i].d2);
        }
        return { res,max_r2 };
    }


    void PhotonKdTree::PrintKdTree(int idx, int space) {
        if (idx == -1 || idx >= PhotonNum) {
            return;
        }
        for (int i = 0; i < space; ++i) {
            cout << " ";
        }
        cout << T[idx].P->Pos << "  " << T[idx].axis << "\n";

        PrintKdTree(T[idx].ls, space + 2);
        PrintKdTree(T[idx].rs, space + 2);
    }

    void PhotonMap::KdTreeTest() {
        Photon P;
        P.Pos = { 3,4,5 };
        StorePhoton(P);
        P.Pos = { 5,6,7 };
        StorePhoton(P);
        P.Pos = { 1,2,3 };
        StorePhoton(P);
        P.Pos = { 1,2,2 };
        StorePhoton(P);
        P.Pos = { 1,2,1 };
        StorePhoton(P);

        PrintPhotonMap();

        BuildKdTree();
        KdTree->PrintKdTree(KdTree->root,0);

        auto [photons, max_r2] = GetNearestNPhotons({ 9,9,9 }, 4, 100);
        cout <<"KDTree: " << max_r2 << "   ";
        for (auto p : photons) {
            cout << p->Pos << " ";
        }cout << "\n";

        /*auto [tphotons, tmax_r2] = GetNearestNPhotonsDebug({ 9,9,9 }, 4, 100);
        cout <<"Debug:  " << tmax_r2 << "   ";
        for (auto p : tphotons) {
            cout << p->Pos << " ";
        }cout << "\n";*/
    }
}