#include <catch2/catch_all.hpp>
#include <arena/ecs/registry.hpp>
#include <arena/ecs/components.hpp>
using namespace arena::ecs;

TEST_CASE("components attach and view") {
  Registry r; auto e = r.create();
  r.add<Transform>(e, {});
  r.add<Renderable>(e, {.mesh=42, .lightmap=7});
  int count=0;
  r.view<Transform,Renderable>([&](auto, const Transform&, const Renderable& rend){
    ++count; REQUIRE(rend.mesh==42); REQUIRE(rend.lightmap==7);
  });
  REQUIRE(count==1);
}
