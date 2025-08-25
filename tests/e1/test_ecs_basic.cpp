#include <catch2/catch_all.hpp>
#include <arena/ecs/registry.hpp>
using namespace arena::ecs;

struct A{ int x; }; struct B{ int y; };

TEST_CASE("create/destroy with generation bump") {
  Registry r;
  auto e = r.create();
  REQUIRE(r.alive(e));
  auto gen = r.generation(e);
  r.destroy(e);
  REQUIRE_FALSE(r.alive(e));
  REQUIRE(r.generation(e) == gen + 1);
}

TEST_CASE("sparse set add/remove/get/view") {
  Registry r; auto e1 = r.create(); auto e2 = r.create();
  r.add<A>(e1, {1}); r.add<B>(e1, {10});
  r.add<A>(e2, {2});

  REQUIRE(r.has<A>(e1)); REQUIRE(r.has<B>(e1));
  REQUIRE(r.get<A>(e2)->x == 2);

  int sumX=0, sumY=0, count=0;
  r.view<A,B>([&](auto, const A& a, const B& b){ sumX+=a.x; sumY+=b.y; ++count; });
  REQUIRE(count == 1); REQUIRE(sumX == 1); REQUIRE(sumY == 10);

  r.remove<A>(e1);
  REQUIRE_FALSE(r.has<A>(e1));
}
