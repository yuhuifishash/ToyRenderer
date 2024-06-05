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
    static struct pdnode {
        int photon_index;
        float d2;
        bool operator<(const pdnode& x)const {
            return d2 < x.d2;
        }
    };

    //<photons, max_r2>
    tuple<std::vector<Photon>, float> PhotonMap::GetNearestNPhotonsDebug(const Vec3& pos, int N, float R) {
        //we use brute_force first
        std::vector<pdnode> V;
        for (int i = 0; i < PhotonNum; ++i) {
            auto p = PhotonM[i];
            float d2 = glm::dot(pos - p.Pos, pos - p.Pos);
            if (d2 <= R) {
                V.push_back({ i,d2 });
            }
        }
        if (V.size() > N) {
            sort(V.begin(), V.end());
        }
        std::vector<Photon> res;
        float max_r2;
        for (int i = 0; i < min((size_t)N, V.size()); ++i) {
            res.push_back(PhotonM[V[i].photon_index]);
            max_r2 = max(max_r2, V[i].d2);
        }
        return { res,max_r2 };
    }
}