// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// 'end-of-level' touch-trigger entity

#include "base/scene.h"
#include "entity.h"
#include "models.h"

namespace
{
struct Amulet : Entity
{
  Amulet()
  {
    size = Size(1, 1, 1) * 0.5;
    solid = false;
  }

  void onDraw(View* view) const override
  {
    auto r = Actor(pos, MDL_CRATE);
    r.scale = size;
    r.orientation = Quaternion::fromEuler(yaw, pitch, 0);
    view->sendActor(r);
  }

  void tick() override
  {
    yaw += 0.002 * 0.1;
    pitch += 0.003 * 0.1;
  }

  float yaw = 0;
  float pitch = 0;
};
}

#include "entity_factory.h"
static auto const reg = registerEntity("amulet", [] (IEntityConfig*) -> unique_ptr<Entity> { return make_unique<Amulet>(); });
static auto const reg2 = registerEntity("crate", [] (IEntityConfig*) -> unique_ptr<Entity> { return make_unique<Amulet>(); });

