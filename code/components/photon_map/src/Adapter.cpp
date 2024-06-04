#include "component/RenderComponent.hpp"
#include "server/Server.hpp"
#include "PhotonMap.hpp"


namespace PhotonMap
{
    using namespace std;
    using namespace NRenderer;

    // 继承RenderComponent, 复写render接口
    class Adapter : public RenderComponent
    {
        void render(SharedScene spScene) {
            PhotonMapRender renderer{ spScene };
            auto renderResult = renderer.render();
            auto [pixels, width, height] = renderResult;
            getServer().screen.set(pixels, width, height);
        }
    };
}

// REGISTER_RENDERER(Name, Description, Class)
REGISTER_RENDERER(PhotonMap, "光子映射+路径追踪混合", PhotonMap::Adapter);