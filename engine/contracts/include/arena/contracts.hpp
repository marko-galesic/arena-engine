#pragma once
#include <cstdint>
#include <span>

namespace arena {

using EntityId = uint32_t;

struct Mat4 { float m[16]; };
struct Vec3 { float x,y,z; };

struct MeshHandle { uint32_t id{0}; };
struct LightmapHandle { uint32_t id{0}; };

// -------- gfx::IRenderer --------
struct IRenderer {
  virtual ~IRenderer() = default;
  virtual MeshHandle createMesh(std::span<const float> vertices, std::span<const uint32_t> indices) = 0;
  virtual void submitMesh(MeshHandle mh, const Mat4& world, LightmapHandle lm) = 0;
  virtual void beginFrame(int w, int h, float aspect, float alpha) = 0;
  virtual void endFrame() = 0;
};

// -------- phys::IWorld --------
struct RayHit { bool hit{false}; Vec3 pos{}, normal{}; float t{0}; EntityId entity{0}; };
struct Capsule { float radius, halfHeight; };

struct IWorld {
  virtual ~IWorld() = default;
  virtual void registerStaticMesh(MeshHandle mh, const Mat4& world) = 0;
  virtual RayHit raycast(const Vec3& origin, const Vec3& dir, float maxDist) const = 0;
  virtual bool sweepCapsule(const Capsule& cap, const Vec3& start, const Vec3& delta, Vec3& outPos) const = 0;
};

// -------- nav::INav --------
struct Cell { int x,y,level; };
struct Path { bool ok{false}; Cell points[256]; int count{0}; };

struct INav {
  virtual ~INav() = default;
  virtual bool bakeFromWorld(const IWorld& world) = 0;
  virtual Path findPath(const Vec3& start, const Vec3& goal) const = 0;
};

// -------- net::ITransport / IReplicator --------
struct ByteView { const uint8_t* data; size_t size; };

struct ITransport {
  virtual ~ITransport() = default;
  virtual bool send(const void* data, size_t sz) = 0;
  virtual int  receive(uint8_t* out, size_t cap) = 0;
  virtual void setLossSim(float pctLoss, int msJitter) = 0;
};

struct Snapshot { uint32_t tick; ByteView bytes; };

struct IReplicator {
  virtual ~IReplicator() = default;
  virtual void submitInput(uint32_t tick, ByteView input) = 0;
  virtual bool pollSnapshot(Snapshot& out) = 0;
};

// -------- editor::IEditMode --------
struct IEditMode {
  virtual ~IEditMode() = default;
  virtual void placeWall(const Vec3& p0, const Vec3& p1, float height, int level) = 0;
  virtual void markDirty() = 0;
};

} // namespace arena
