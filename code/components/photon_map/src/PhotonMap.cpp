#include "server/Server.hpp"

#include "PhotonMap.hpp"

#include "VertexTransformer.hpp"
#include "intersections/intersections.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace PhotonMap
{
    auto PhotonMapRender::render() -> RenderResult {
        // shaders
        shaderPrograms.clear();
        ShaderCreator shaderCreator{};
        for (auto& m : scene.materials) {
            shaderPrograms.push_back(shaderCreator.create(m, scene.textures));
        }

        RGBA* pixels = new RGBA[1ll * width * height]{};

        // �ֲ�����ת������������
        VertexTransformer vertexTransformer{};
        vertexTransformer.exec(spScene);




        getServer().logger.log("Done...");
        return { pixels, width, height };
    }



}