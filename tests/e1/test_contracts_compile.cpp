#include <arena/contracts.hpp>
#include <type_traits>

using namespace arena;

struct DummyRenderer : IRenderer {
  MeshHandle createMesh(std::span<const float>, std::span<const uint32_t>) override { return {}; }
  void submitMesh(MeshHandle, const Mat4&, LightmapHandle) override {}
  void beginFrame(int,int,float,float) override {}
  void endFrame() override {}
};

static_assert(std::is_abstract_v<IWorld>);
static_assert(std::is_abstract_v<INav>);
static_assert(std::is_abstract_v<ITransport>);
static_assert(std::is_abstract_v<IReplicator>);
static_assert(std::is_abstract_v<IEditMode>);

int main() {
    return 0;
}
