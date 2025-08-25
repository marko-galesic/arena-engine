#include <catch2/catch_all.hpp>
#include <arena/contracts.hpp>
#include <arena/ecs/registry.hpp>
#include <arena/ecs/components.hpp>
#include <vector>
#include <cstring>

using namespace arena;
using namespace arena::ecs;

struct MockRenderer : IRenderer {
  int frames=0; MeshHandle last{};
  MeshHandle createMesh(std::span<const float>, std::span<const uint32_t>) override { return {123}; }
  void submitMesh(MeshHandle mh, const Mat4&, LightmapHandle) override { last = mh; }
  void beginFrame(int,int,float,float) override { frames++; }
  void endFrame() override {}
};

struct MockWorld : IWorld {
  std::vector<EntityId> statics;
  void registerStaticMesh(MeshHandle, const Mat4&) override { statics.push_back((EntityId)statics.size()+1); }
  RayHit raycast(const Vec3& o, const Vec3& d, float maxDist) const override {
    (void)o;(void)d;(void)maxDist; return RayHit{true, {0,0,1},{0,1,0}, 1.0f, statics.empty()?0:statics.back()};
  }
  bool sweepCapsule(const Capsule&, const Vec3& start, const Vec3& delta, Vec3& outPos) const override {
    outPos = {start.x+delta.x, start.y+delta.y, start.z+delta.z}; return true;
  }
};

struct MockNav : INav {
  bool baked=false;
  bool bakeFromWorld(const IWorld&) override { baked=true; return true; }
  Path findPath(const Vec3&, const Vec3&) const override { Path p; p.ok=true; p.count=2; p.points[0]={0,0,0}; p.points[1]={1,0,0}; return p; }
};

struct MockTransport : ITransport {
  std::vector<uint8_t> pipe;
  bool send(const void* data, size_t sz) override { const uint8_t* b=(const uint8_t*)data; pipe.assign(b,b+sz); return true; }
  int  receive(uint8_t* out, size_t cap) override { if(pipe.empty()) return 0; int n=(int)std::min(cap, pipe.size()); std::memcpy(out, pipe.data(), n); pipe.clear(); return n; }
  void setLossSim(float, int) override {}
};

struct MockReplicator : IReplicator {
  std::vector<uint8_t> last;
  void submitInput(uint32_t, ByteView input) override { last.assign(input.data, input.data+input.size); }
  bool pollSnapshot(Snapshot& out) override { if(last.empty()) return false; out.tick=1; out.bytes={last.data(), last.size()}; return true; }
};

TEST_CASE("contracts smoke across mocks + ECS") {
  // Setup
  MockRenderer gfx; MockWorld phys; MockNav nav; MockTransport net; MockReplicator repl;
  Registry r;
  auto e = r.create(); r.add<Renderable>(e, {.mesh=123, .lightmap=0});

  // gfx: create & submit
  auto mh = gfx.createMesh({}, {});
  gfx.beginFrame(1280,720,1280.f/720.f, 0.5f);
  gfx.submitMesh(mh, Mat4{}, LightmapHandle{0});
  gfx.endFrame();
  REQUIRE(gfx.frames==1);
  REQUIRE(gfx.last.id==123);

  // phys: register & raycast
  phys.registerStaticMesh(mh, Mat4{});
  auto hit = phys.raycast({0,0,0},{0,0,1}, 100.f);
  REQUIRE(hit.hit); REQUIRE(hit.entity != 0);

  // nav: bake + path
  REQUIRE(nav.bakeFromWorld(phys));
  auto path = nav.findPath({0,0,0},{10,0,0});
  REQUIRE(path.ok); REQUIRE(path.count >= 2);

  // net: send->receive; replicator echoes snapshot
  uint8_t in[3] = {1,2,3}; REQUIRE(net.send(in,3));
  uint8_t out[8]; int n = net.receive(out,8);
  REQUIRE(n==3); REQUIRE(out[2]==3);
  repl.submitInput(0, {out,(size_t)n});
  Snapshot s; REQUIRE(repl.pollSnapshot(s)); REQUIRE(s.bytes.size==3);

  // ecs view bridge (Renderable exists)
  int seen=0;
  r.view<Renderable,Renderable>([&](auto, const Renderable& a, const Renderable&){ ++seen; REQUIRE(a.mesh==123); });
  REQUIRE(seen==1);
}
