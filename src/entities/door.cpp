// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "collision_groups.h" // CG_WALLS
#include "entity.h"
#include "models.h" // MDL_DOOR
#include "sounds.h" // SND_DOOR
#include "toggle.h" // decrement
#include "trigger.h" // TriggerEvent

struct Door : Entity, IEventSink
{
  Door(int id_) : id(id_)
  {
    size = UnitSize * 2;
    solid = true;
  }

  void enter() override
  {
    pos -= UnitSize;
    subscription = game->subscribeForEvents(this);
  }

  void leave() override
  {
    subscription.reset();
  }

  virtual void tick() override
  {
  }

  virtual void onDraw(View* view) const override
  {
    if(!solid)
      return;

    auto r = Actor(pos, MDL_DOOR1 + id);
    r.action = 1;
    r.ratio = 0;
    r.scale = size;

    view->sendActor(r);
  }

  virtual void notify(const Event* evt) override
  {
    if(!solid)
      return;

    if(auto trg = evt->as<TriggerEvent>())
    {
      if(trg->idx != id)
        return;

      game->playSound(SND_DOOR);
      solid = false;
    }
  }

  bool state = false;
  int openingDelay = 0;
  const int id;
  unique_ptr<Handle> subscription;
};

unique_ptr<Entity> makeDoor(int id)
{
  return make_unique<Door>(id);
}

///////////////////////////////////////////////////////////////////////////////

struct AutoDoor : Entity, Switchable
{
  AutoDoor()
  {
    size = Size3f(1, 1, 1);
    solid = true;
  }

  virtual void enter() override
  {
    basePos = pos;
  }

  virtual void tick() override
  {
    switch(state)
    {
    case State::Closed:
      break;
    case State::Opening:
      {
        if(pos.z - basePos.z < 2.3)
          physics->moveBody(this, Up * 0.004);
        else
          state = State::Open;

        break;
      }
    case State::Open:
      {
        if(decrement(timer))
        {
          game->playSound(SND_DOOR);
          state = State::Closing;
        }

        break;
      }
    case State::Closing:
      {
        if(pos.z - basePos.z > 0.001)
          physics->moveBody(this, Down * 0.004);
        else
          state = State::Closed;
      }
    }
  }

  virtual void onDraw(View* view) const override
  {
    auto r = Actor(pos, MDL_DOOR);
    r.action = 1;
    r.ratio = 0;
    r.scale = size;
    view->sendActor(r);
  }

  virtual void onSwitch() override
  {
    if(state != State::Closed)
      return;

    game->playSound(SND_DOOR);
    state = State::Opening;
    timer = 1500;
  }

  enum class State
  {
    Closed,
    Opening,
    Open,
    Closing,
  };

  State state = State::Closed;
  Vector basePos;
  int timer = 0;
};

unique_ptr<Entity> makeAutoDoor()
{
  return make_unique<AutoDoor>();
}

///////////////////////////////////////////////////////////////////////////////

#include "entities/explosion.h"

struct BreakableDoor : Entity, Damageable
{
  BreakableDoor()
  {
    size = UnitSize;
    solid = true;
    collisionGroup = CG_WALLS;
  }

  virtual void onDraw(View* view) const override
  {
    auto r = Actor(pos, MDL_DOOR);
    r.scale = size;

    if(blinking)
      r.effect = Effect::Blinking;

    view->sendActor(r);
  }

  virtual void tick() override
  {
    decrement(blinking);
  }

  virtual void onDamage(int amount) override
  {
    blinking = 200;
    life -= amount;

    if(life < 0)
    {
      game->playSound(SND_EXPLODE);
      dead = true;

      auto explosion = makeExplosion();
      explosion->pos = getCenter();
      game->spawn(explosion.release());
    }
    else
      game->playSound(SND_DAMAGE);
  }

  int life = 130;
};

unique_ptr<Entity> makeBreakableDoor()
{
  return make_unique<BreakableDoor>();
}

#include "entity_factory.h"
static auto const reg1 = registerEntity("auto_door", [] (IEntityConfig*) { return makeAutoDoor(); });
static auto const reg2 = registerEntity("door", [] (IEntityConfig* args) { auto arg = args->getInt("0"); return makeDoor(arg); });

