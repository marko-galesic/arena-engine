#pragma once
#include <cstdint>
#include <vector>
#include <typeindex>
#include <unordered_map>
#include <memory>
#include <cassert>

namespace arena::ecs {

using Entity = uint32_t;
constexpr Entity kInvalid{0};

struct Handle {
  uint32_t index{0};
  uint32_t generation{0};
};

struct EntityMeta {
  uint32_t generation{1};
  bool alive{false};
};

class IStorage {
public:
  virtual ~IStorage() = default;
  virtual void onDestroy(Entity e) = 0;
};

template<typename T>
class SparseSet : public IStorage {
public:
  // Dense component array
  std::vector<T> data;
  std::vector<Entity> denseToEntity;
  // Sparse map: entity -> dense index (kInvalid = not present)
  std::vector<uint32_t> entityToDense;

  void ensureSize(uint32_t n) {
    if (entityToDense.size() < n) entityToDense.resize(n, 0);
  }

  bool has(Entity e) const {
    uint32_t idx = entityToDense[e];
    return idx && denseToEntity[idx-1] == e;
  }

  T& add(Entity e, const T& value) {
    uint32_t idx = (uint32_t)data.size();
    data.push_back(value);
    denseToEntity.push_back(e);
    ensureSize(e+1);
    entityToDense[e] = idx + 1; // +1 so 0 means "absent"
    return data.back();
  }

  void remove(Entity e) {
    uint32_t idx1 = entityToDense[e];
    if (!idx1) return;
    uint32_t idx = idx1 - 1;
    uint32_t last = (uint32_t)data.size() - 1;
    if (idx != last) {
      data[idx] = data[last];
      Entity movedE = denseToEntity[last];
      denseToEntity[idx] = movedE;
      entityToDense[movedE] = idx + 1;
    }
    data.pop_back();
    denseToEntity.pop_back();
    entityToDense[e] = 0;
  }

  T* get(Entity e) {
    uint32_t idx1 = entityToDense[e];
    if (!idx1) return nullptr;
    return &data[idx1 - 1];
  }

  void onDestroy(Entity e) override { remove(e); }
};

class Registry {
public:
  Entity create() {
    Entity e;
    if (!freeList.empty()) {
      e = freeList.back(); freeList.pop_back();
      meta[e].alive = true;
    } else {
      e = (Entity)meta.size();
      meta.push_back({1, true});
      // grow storages' sparse arrays on demand
    }
    return e;
  }

  void destroy(Entity e) {
    if (e >= meta.size() || !meta[e].alive) return;
    // notify storages
    for (auto& [_, storage] : storages) storage->onDestroy(e);
    meta[e].alive = false;
    meta[e].generation++;
    freeList.push_back(e);
  }

  bool alive(Entity e) const { return e < meta.size() && meta[e].alive; }
  uint32_t generation(Entity e) const { return meta[e].generation; }

  template<typename T>
  SparseSet<T>& storage() {
    auto key = std::type_index(typeid(T));
    auto it = storages.find(key);
    if (it == storages.end()) {
      auto* s = new SparseSet<T>();
      storages.emplace(key, std::unique_ptr<IStorage>(s));
      return *s;
    }
    return *static_cast<SparseSet<T>*>(it->second.get());
  }

  template<typename T>
  T& add(Entity e, const T& value) { return storage<T>().add(e, value); }

  template<typename T>
  void remove(Entity e) { storage<T>().remove(e); }

  template<typename T>
  T* get(Entity e) { return storage<T>().get(e); }

  template<typename T>
  bool has(Entity e) const {
    auto it = storages.find(std::type_index(typeid(T)));
    if (it == storages.end()) return false;
    auto* s = static_cast<SparseSet<T>*>(it->second.get());
    return s->has(e);
  }

  template<typename T1, typename T2, typename Fn>
  void view(Fn&& fn) {
    auto& a = storage<T1>();
    for (size_t i = 0; i < a.data.size(); ++i) {
      Entity e = a.denseToEntity[i];
      if (storage<T2>().has(e)) {
        fn(e, a.data[i], *storage<T2>().get(e));
      }
    }
  }

private:
  std::vector<EntityMeta> meta;
  std::vector<Entity> freeList;
  std::unordered_map<std::type_index, std::unique_ptr<IStorage>> storages;
};

} // namespace arena::ecs
