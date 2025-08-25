#pragma once
#include <cstdint>

namespace arena::ecs {

struct Transform {
  float pos[3]{0,0,0};
  float rotYawPitchRoll[3]{0,0,0}; // radians
  float scale[3]{1,1,1};
};

struct Renderable {
  uint32_t mesh{0};
  uint32_t lightmap{0};
};

struct Collider {
  // For MVP: 0=none,1=box,2=capsule,3=ramp
  uint8_t shape{0};
  float params[6]{0}; // interpretation depends on shape
  bool  isStatic{true};
};

struct CharacterController {
  float radius{0.4f};
  float height{1.8f};
  float speed{5.5f};
  bool  grounded{false};
};

struct NetworkReplicated {
  // bitset or mask of fields replicated; start simple:
  uint32_t mask{0};
  // network id for entities if needed:
  uint32_t netId{0};
};

struct CameraController {
  float moveSpeed{5.0f};
  float lookSensitivity{0.002f};
};

} // namespace arena::ecs
