#include "stdafx.h"
#include "collective_builder.h"
#include "collective.h"
#include "creature.h"
#include "creature_name.h"
#include "collective_name.h"
#include "creature_attributes.h"
#include "collective_config.h"
#include "tribe.h"
#include "collective_control.h"
#include "immigration.h"
#include "territory.h"

CollectiveBuilder::CollectiveBuilder(const CollectiveConfig& cfg, TribeId t)
    : config(cfg), tribe(t) {
}

CollectiveBuilder& CollectiveBuilder::setLevel(WLevel l) {
  level = l;
  return *this;
}

CollectiveBuilder& CollectiveBuilder::setLocationName(const string& n) {
  locationName = n;
  return *this;
}

CollectiveBuilder& CollectiveBuilder::setRaceName(const string& n) {
  raceName = n;
  return *this;
}

CollectiveBuilder& CollectiveBuilder::setDiscoverable() {
  discoverable = true;
  return *this;
}

CollectiveBuilder& CollectiveBuilder::addCreature(WCreature c) {
  if (!c->getAttributes().isInnocent() && (!creatures.empty() || config->isLeaderFighter()))
    creatures.push_back({c, {MinionTrait::FIGHTER}});
  else
    creatures.push_back({c, {}});
  return *this;
}

bool CollectiveBuilder::hasCentralPoint() {
  return !!centralPoint;
}

void CollectiveBuilder::setCentralPoint(Vec2 pos) {
  centralPoint = pos;
}

CollectiveBuilder& CollectiveBuilder::addArea(const vector<Vec2>& v) {
  append(squares, v);
  return *this;
}

optional<CollectiveName> CollectiveBuilder::generateName() {
  if (!creatures.empty()) {
    CollectiveName ret;
    auto leader = creatures[0].creature;
    if (locationName && raceName)
      ret.full = capitalFirst(*raceName) + " of " + *locationName;
    else if (auto first = leader->getName().first())
      ret.full = leader->getName().title();
    else if (raceName)
      ret.full = capitalFirst(*raceName);
    else
      ret.full = leader->getName().title();
    if (locationName)
      ret.shortened = *locationName;
    else
      ret.shortened = leader->getName().first().value_or(leader->getName().bare());
    if (raceName)
      ret.race = *raceName;
    else
      ret.race = leader->getName().stack();
    return ret;
  } else
    return none;
}

PCollective CollectiveBuilder::build() {
  CHECK(level);
  auto c = Collective::create(level, *tribe, generateName(), discoverable);
  Immigration im(c.get());
  c->init(std::move(*config), std::move(im));
  c->setControl(CollectiveControl::idle(c.get()));
  for (auto& elem : creatures)
    c->addCreature(elem.creature, elem.traits);
  for (Vec2 v : squares) {
    Position pos(v, level);
    c->addKnownTile(pos);
    //if (c->canClaimSquare(pos))
      c->claimSquare(pos);
  }
  if (centralPoint)
    c->getTerritory().setCentralPoint(Position(*centralPoint, level));
  return c;
}

bool CollectiveBuilder::hasCreatures() const {
  return !creatures.empty();
}
