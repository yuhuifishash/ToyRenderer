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
}