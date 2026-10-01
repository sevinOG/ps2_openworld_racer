#include "racer_game.hpp"

#include <math.h>


namespace Racer {

using Tyra::CameraInfo3D;
using Tyra::Color;
using Tyra::Engine;
using Tyra::Math;
using Tyra::Vec4;
constexpr float kWorld = 180.0F;
constexpr float kWalk = 9.0F;
constexpr float kLookYaw = 2.6F;
constexpr float kLookPitch = 2.0F;
constexpr float kCarAccel = 20.0F;
constexpr float kCarBrake = 28.0F;
constexpr float kCarReverse = 10.0F;
constexpr float kCarDrag = 1.2F;
constexpr float kSteer = 2.1F;
constexpr float kEnterDist = 3.6F;
constexpr float kGravity = 15.0F;
constexpr float kGroundFric = 5.5F;
constexpr float kAirDrag = 0.4F;
constexpr float kWheelRadius = 0.42F;
constexpr float kStrut = 1.7F;  // ride height; shocks themselves are not drawn
constexpr float kWaxDraw = 1.0F;
constexpr float kWaxCopy = 2.0F;
constexpr float kWaxWeld = 1.0F;
constexpr float kBenchHalf = 2.0F;  // 4x4 table, larger than four wheel diameters

void RacerGame::carBasis(Vec4* fwd, Vec4* right, Vec4* up) const {
  float cy = Math::cos(carYaw);
  float sy = Math::sin(carYaw);
  float cp = Math::cos(carPitch);
  float sp = Math::sin(carPitch);
  float cr = Math::cos(carRoll);
  float sr = Math::sin(carRoll);
  Vec4 f(sy, 0.0F, cy);
  Vec4 r(cy, 0.0F, -sy);
  Vec4 u(0.0F, 1.0F, 0.0F);
  Vec4 f2(f.x * cp + u.x * sp, f.y * cp + u.y * sp, f.z * cp + u.z * sp);
  Vec4 u2(u.x * cp - f.x * sp, u.y * cp - f.y * sp, u.z * cp - f.z * sp);
  Vec4 r2(r.x * cr - u2.x * sr, r.y * cr - u2.y * sr, r.z * cr - u2.z * sr);
  // forward x right = up. The other cross flipped the whole buggy.
  Vec4 u3 = f2.cross(r2);
  u3.normalize();
  f2.normalize();
  r2 = u3.cross(f2);
  r2.normalize();
  *fwd = f2;
  *right = r2;
  *up = u3;
}

float terrainHeight(float x, float z) {
  float h = 0.0F;
  h += 4.2F * sinf(x * 0.045F) * cosf(z * 0.038F);
  h += 6.5F * sinf(x * 0.017F + 0.6F) * sinf(z * 0.015F);
  h += 2.0F * sinf(x * 0.09F + z * 0.06F);
  float dx = x - 95.0F;
  float dz = z - 70.0F;
  h += 46.0F * expf(-(dx * dx + dz * dz) / 2400.0F);
  float dx2 = x + 120.0F;
  float dz2 = z - 30.0F;
  h += 24.0F * expf(-(dx2 * dx2 + dz2 * dz2) / 3800.0F);
  float dx3 = x - 20.0F;
  float dz3 = z + 110.0F;
  h += 16.0F * expf(-(dx3 * dx3 + dz3 * dz3) / 2800.0F);
  return h;
}

RacerGame::RacerGame(Engine* t_engine)
    : engine(t_engine),
      inCar(false),
      playerYaw(0.0F),
      carYaw(0.0F),
      carSpeed(0.0F),
      camYaw(0.0F),
      camPitch(0.42F),
      steerAngle(0.0F),
      wheelSpin(0.0F),
      carPitch(0.0F),
      carRoll(0.0F),
      carVelY(0.0F),
      pitchVel(0.0F),
      rollVel(0.0F),
      playerPos(0.0F, 0.0F, -20.0F),
      carPos(5.0F, 0.0F, 3.0F),
      camPos(0.0F, 12.0F, -40.0F),
      camLook(0.0F, 2.0F, 0.0F) {}

void RacerGame::initCubes() {
  const float s = 1.5F;
  const float gap = 0.1F;
  const float ox = 18.0F;
  const float oz = 14.0F;
  const float step = s * 2.0F + gap;
  const float base = terrainHeight(ox, oz);
  int n = 0;
  for (int iz = 0; iz < 2; iz++) {
    for (int ix = 0; ix < 2; ix++) {
      cubes[n].pos = Vec4(ox + (ix - 0.5F) * step, base,
                          oz + (iz - 0.5F) * step);
      cubes[n].vel = Vec4(0.0F, 0.0F, 0.0F);
      cubes[n].size = s;
      n++;
    }
  }
  for (int i = 0; i < 2; i++) {
    cubes[n].pos = Vec4(ox + (i - 0.5F) * step, base + s * 2.0F, oz);
    cubes[n].vel = Vec4(0.0F, 0.0F, 0.0F);
    cubes[n].size = s;
    n++;
  }
  cubes[n].pos = Vec4(ox, base + s * 4.0F, oz);
  cubes[n].vel = Vec4(0.0F, 0.0F, 0.0F);
  cubes[n].size = s;
}

void RacerGame::init() {
  engine->renderer.setClearScreenColor(Color(206.0F, 186.0F, 222.0F));
  engine->renderer.core.renderer3D.setFov(62.0F);
  // usePipeline() uploads VU1 programs on the first frame. Without this
  // the static pipeline's DMA path is uninitialized and the GS never
  // presents a frame after the Tyra splash.
  stapip.setRenderer(&engine->renderer.core);
  equipStockParts();
  benchPos = Vec4(-8.0F, 0.0F, -18.0F);
  benchPos.y = terrainHeight(benchPos.x, benchPos.z);
  carPos.y = terrainHeight(carPos.x, carPos.z) + kWheelRadius + kStrut - 0.12F;
  playerPos.y = terrainHeight(playerPos.x, playerPos.z);
  initCubes();
}



float RacerGame::analog(unsigned char raw) const {
  float a = (static_cast<float>(raw) - 127.0F) / 127.0F;
  if (a > -0.18F && a < 0.18F) return 0.0F;
  return a;
}

void RacerGame::equipStockParts() {
  // Front: softer spring, soft knobby. Rear: stiffer spring, medium compound.
  ShockSpec frontShock{54.0F, 34.0F, 62.0F, 0.62F};
  ShockSpec rearShock{72.0F, 40.0F, 72.0F, 0.55F};
  TireSpec frontTire{0.42F, 16.0F, 18.0F, 1.18F, 420.0F, TireSoft};
  TireSpec rearTire{0.42F, 18.0F, 18.0F, 1.02F, 460.0F, TireMedium};
  corners[0] = {rearShock, rearTire};
  corners[1] = {rearShock, rearTire};
  corners[2] = {frontShock, frontTire};
  corners[3] = {frontShock, frontTire};
}

float RacerGame::tireGrip(const TireSpec& tire) const {
  float p = tire.pressure / tire.ratedPressure;
  float off = p - 1.0F;
  float curve = 1.0F - off * off * 0.45F;
  if (curve < 0.45F) curve = 0.45F;
  return tire.grip * curve;
}

float RacerGame::cornerSpring(const CornerPart& corner) const {
  float p = corner.tire.pressure / corner.tire.ratedPressure;
  if (p < 0.35F) p = 0.35F;
  float tireK = corner.tire.sidewall * p;
  float shockK = corner.shock.spring;
  return (shockK * tireK) / (shockK + tireK);
}

void RacerGame::clampToWorld(Vec4* p) const {
  if (p->x < -kWorld) p->x = -kWorld;
  if (p->x > kWorld) p->x = kWorld;
  if (p->z < -kWorld) p->z = -kWorld;
  if (p->z > kWorld) p->z = kWorld;
}

Tyra::Vec4 RacerGame::rotateYaw(const Vec4& local, float yaw,
                               const Vec4& origin) const {
  float c = Math::cos(yaw);
  float s = Math::sin(yaw);
  return Vec4(origin.x + local.x * c + local.z * s, origin.y + local.y,
              origin.z + -local.x * s + local.z * c);
}

void RacerGame::line(const Vec4& a, const Vec4& b, const Color& c) {
  engine->renderer.renderer3D.utility.pushLine(a, b, c);
}

void RacerGame::ink(const Vec4& a, const Vec4& b, const Color& c) {
  line(a, b, c);
  float o = ((glitchPhase & 16) != 0) ? 0.05F : -0.035F;
  Color ghost((glitchPhase & 32) != 0 ? 255.0F : 130.0F,
              (glitchPhase & 32) != 0 ? 150.0F : 230.0F,
              (glitchPhase & 32) != 0 ? 214.0F : 255.0F);
  line(Vec4(a.x + o, a.y, a.z - o), Vec4(b.x + o, b.y, b.z - o), ghost);
}

void RacerGame::drawBoxYaw(const Vec4& center, float yaw, float hx, float hy,
                           float hz, const Color& color) {
  Vec4 cnr[8];
  int n = 0;
  for (int y = 0; y < 2; y++) {
    float yy = (y == 0) ? 0.0F : hy * 2.0F;
    for (int z = -1; z <= 1; z += 2) {
      for (int x = -1; x <= 1; x += 2) {
        cnr[n++] = rotateYaw(Vec4(hx * static_cast<float>(x), yy,
                                  hz * static_cast<float>(z)),
                             yaw, Vec4(center.x, center.y, center.z));
      }
    }
  }
  auto L = [&](int a, int b) { line(cnr[a], cnr[b], color); };
  L(0, 1);
  L(1, 3);
  L(3, 2);
  L(2, 0);
  L(4, 5);
  L(5, 7);
  L(7, 6);
  L(6, 4);
  L(0, 4);
  L(1, 5);
  L(2, 6);
  L(3, 7);
}

void RacerGame::tryEnterExit() {
  if (!engine->pad.getClicked().Triangle) return;

  if (inCar) {
    float c = Math::cos(carYaw);
    float s = Math::sin(carYaw);
    playerPos = Vec4(carPos.x - c * 2.4F, 0.0F, carPos.z + s * 2.4F);
    playerPos.y = terrainHeight(playerPos.x, playerPos.z);
    playerVelY = 0.0F;
    playerYaw = carYaw;
    camYaw = playerYaw;
    camPitch = 0.42F;
    lookHeld = false;
    lookWasHeld = false;
    lookReturning = false;
    inCar = false;
    return;
  }

  if (atBench) {
    atBench = false;
    helpHold = 0.0F;
    camYaw = playerYaw;
    camPitch = 0.42F;
    lookHeld = false;
    lookWasHeld = false;
    lookReturning = false;
    engine->renderer.core.renderer3D.setFov(62.0F);
    return;
  }

  if (nearBench()) {
    atBench = true;
    helpVisible = true;
    helpHold = 0.0F;
    cursorX = 0.0F;
    cursorZ = 0.0F;
    benchYaw = 0.0F;
    benchPitch = 1.05F;
    traceBits = 0;
    traceLatched = false;
    playerPos = Vec4(benchPos.x, 0.0F, benchPos.z - 2.65F);
    playerPos.y = terrainHeight(playerPos.x, playerPos.z);
    playerVelY = 0.0F;
    playerYaw = 0.0F;
    lookHeld = false;
    lookWasHeld = false;
    lookReturning = false;
    engine->renderer.core.renderer3D.setFov(36.0F);
    return;
  }

  float dx = playerPos.x - carPos.x;
  float dz = playerPos.z - carPos.z;
  if (dx * dx + dz * dz < kEnterDist * kEnterDist) {
    inCar = true;
    carSpeed = 0.0F;
    yawRate = 0.0F;
    camYaw = carYaw;
    camPitch = 0.42F;
    lookHeld = false;
    lookWasHeld = false;
    lookReturning = false;
  }
}

void RacerGame::updateLook(float dt) {
  const auto& r = engine->pad.getRightJoyPad();
  float rx = analog(r.h);
  float ry = analog(r.v);
  lookHeld = (rx != 0.0F || ry != 0.0F);
  if (lookHeld) {
    lookReturning = false;
    // Stick right is +rx. Decreasing yaw looks right.
    camYaw -= rx * kLookYaw * dt;
    camPitch += ry * kLookPitch * dt;
  } else if (lookWasHeld) {
    lookReturning = true;
  }
  lookWasHeld = lookHeld;
  if (camPitch > 0.85F) camPitch = 0.85F;
  if (camPitch < -0.12F) camPitch = -0.12F;
}

void RacerGame::updateOnFoot(float dt) {
  const auto& l = engine->pad.getLeftJoyPad();
  float lx = analog(l.h);
  float ly = analog(l.v);

  float c = Math::cos(camYaw);
  float s = Math::sin(camYaw);
  Vec4 fwd(s, 0.0F, c);
  Vec4 right(c, 0.0F, -s);
  Vec4 move = fwd * (-ly) + right * (-lx);
  playerPos += move * kWalk * dt;
  clampToWorld(&playerPos);

  float ground = terrainHeight(playerPos.x, playerPos.z);
  bool grounded = playerVelY <= 0.0F && playerPos.y <= ground + 0.12F;
  if (grounded && engine->pad.getClicked().Cross) {
    playerVelY = 8.6F;
    grounded = false;
  }
  if (grounded) {
    playerPos.y = ground;
    playerVelY = 0.0F;
  } else {
    playerVelY -= kGravity * dt;
    playerPos.y += playerVelY * dt;
    if (playerPos.y <= ground) {
      playerPos.y = ground;
      if (playerVelY < 0.0F) playerVelY = 0.0F;
    }
  }

  if (lx != 0.0F || ly != 0.0F) {
    playerYaw = Math::atan2(move.x, move.z);
  }
}

void RacerGame::updateInCar(float dt) {
  const auto& l = engine->pad.getLeftJoyPad();
  const auto& held = engine->pad.getPressed();
  float lx = analog(l.h);

  float frontG =
      0.5F * (tireGrip(corners[2].tire) + tireGrip(corners[3].tire));
  float rearG =
      0.5F * (tireGrip(corners[0].tire) + tireGrip(corners[1].tire));
  float mu = 0.5F * (frontG + rearG);
  float driveScale = 0.72F + 0.28F * mu;
  if (held.R2) {
    carSpeed += kCarAccel * driveScale * dt;
  }
  if (held.L2) {
    if (carSpeed > 0.4F) {
      carSpeed -= kCarBrake * driveScale * dt;
    } else {
      carSpeed -= kCarReverse * driveScale * dt;
    }
  }
  float pressureDrag = 0.0F;
  for (int i = 0; i < 4; i++) {
    float rated = corners[i].tire.ratedPressure;
    pressureDrag += 0.12F * fabsf(corners[i].tire.pressure - rated) / rated;
  }
  float drag = kCarDrag + pressureDrag;
  float dragKeep = 1.0F - drag * dt;
  if (dragKeep < 0.0F) dragKeep = 0.0F;
  carSpeed *= dragKeep;
  if (carSpeed > 24.0F) carSpeed = 24.0F;
  if (carSpeed < -8.0F) carSpeed = -8.0F;

  float steerScale = carSpeed / 8.0F;
  if (steerScale > 1.0F) steerScale = 1.0F;
  if (steerScale < -1.0F) steerScale = -1.0F;
  // Same sign as the old carYaw -= lx. Front grip sets how hard the nose
  // comes around; rear grip damps the yaw so a loose rear can slide.
  float gripScale = frontG / 1.10F;
  float targetRate = -lx * kSteer * steerScale * gripScale;
  yawRate += (targetRate - yawRate) * 10.0F * dt;
  yawRate -= yawRate * (0.65F * rearG) * dt;
  carYaw += yawRate * dt;
  float wantSteer = -lx * 0.55F;
  steerAngle += (wantSteer - steerAngle) * 0.35F;
  float spinR = corners[2].tire.radius;
  if (spinR < 0.05F) spinR = 0.05F;
  wheelSpin += carSpeed / spinR * dt;
  carSpeed -= fabsf(steerAngle) * fabsf(carSpeed) * 0.35F * dt;
  // nose-up costs speed, downhill gives it back
  carSpeed -= Math::sin(carPitch) * 14.0F * dt;

  float c = Math::cos(carYaw);
  float s = Math::sin(carYaw);
  carPos.x += s * carSpeed * dt;
  carPos.z += c * carSpeed * dt;
  clampToWorld(&carPos);

  const float wx[4] = {-1.2F, 1.2F, -1.2F, 1.2F};
  const float wz[4] = {-1.55F, -1.55F, 1.55F, 1.55F};
  // Shock travel is a few tenths. A mountainside is not. Plant the chassis
  // on the plane through the four contact patches so the body climbs and
  // tilts with the tires instead of letting the wheels walk up through it.
  float groundY[4];
  float kSum = 0.0F;
  for (int i = 0; i < 4; i++) {
    float x = carPos.x + c * wx[i] + s * wz[i];
    float z = carPos.z - s * wx[i] + c * wz[i];
    groundY[i] = terrainHeight(x, z);
    kSum += cornerSpring(corners[i]);
  }
  float sag = 0.12F;
  if (kSum > 1.0F) sag = 26.0F / kSum;
  if (sag < 0.04F) sag = 0.04F;
  if (sag > 0.35F) sag = 0.35F;

  float hFront = 0.5F * (groundY[2] + groundY[3]);
  float hRear = 0.5F * (groundY[0] + groundY[1]);
  float hLeft = 0.5F * (groundY[0] + groundY[2]);
  float hRight = 0.5F * (groundY[1] + groundY[3]);
  float targetPitch = atan2f(hFront - hRear, 3.1F);
  float targetRoll = atan2f(hLeft - hRight, 2.4F);
  if (targetPitch > 1.05F) targetPitch = 1.05F;
  if (targetPitch < -1.05F) targetPitch = -1.05F;
  if (targetRoll > 1.05F) targetRoll = 1.05F;
  if (targetRoll < -1.05F) targetRoll = -1.05F;

  float sp = sinf(targetPitch);
  float sr = sinf(targetRoll);
  float ySum = 0.0F;
  for (int i = 0; i < 4; i++) {
    float want = groundY[i] + corners[i].tire.radius + (kStrut - sag);
    ySum += want - sp * wz[i] + sr * wx[i];
  }
  carPos.y = ySum * 0.25F;
  carPitch = targetPitch;
  carRoll = targetRoll;
  carVelY = 0.0F;
  pitchVel = 0.0F;
  rollVel = 0.0F;
}

void RacerGame::updateCamera(float dt) {
  if (atBench) {
    const float dist = 14.0F;
    float cp = Math::cos(benchPitch);
    float sp = Math::sin(benchPitch);
    float cy = Math::cos(benchYaw);
    float sy = Math::sin(benchYaw);
    camLook = Vec4(benchPos.x, benchPos.y + 0.95F, benchPos.z);
    camPos = Vec4(camLook.x - sy * cp * dist, camLook.y + sp * dist,
                  camLook.z - cy * cp * dist);
    return;
  }

  if (!lookHeld) {
    float facing = inCar ? carYaw : playerYaw;
    float d = facing - camYaw;
    while (d > Math::PI) d -= Math::PI * 2.0F;
    while (d < -Math::PI) d += Math::PI * 2.0F;
    // Glued behind the body while driving. After a right-stick look, swing
    // back slowly until the view is centered again.
    float rate = lookReturning ? 1.35F : 16.0F;
    float blend = 1.0F - expf(-rate * dt);
    camYaw += d * blend;
    if (lookReturning) {
      camPitch += (0.42F - camPitch) * blend;
      if (fabsf(d) < 0.02F && fabsf(camPitch - 0.42F) < 0.02F) {
        camYaw = facing;
        camPitch = 0.42F;
        lookReturning = false;
      }
    }
  }

  Vec4 target = inCar ? carPos : playerPos;
  float dist = inCar ? 22.0F : 16.0F;
  float lookY = inCar ? 1.15F : 1.2F;
  float cp = Math::cos(camPitch);
  float sp = Math::sin(camPitch);
  float cy = Math::cos(camYaw);
  float sy = Math::sin(camYaw);

  float height = target.y + 4.5F + sp * dist * 0.35F;
  if (height < target.y + 1.6F) height = target.y + 1.6F;

  Vec4 desired(target.x - sy * cp * dist, height,
               target.z - cy * cp * dist);

  camPos += (desired - camPos) * 0.22F;
  camLook = Vec4(target.x, target.y + lookY, target.z);
}

void RacerGame::pushCubesFromCircle(float cx, float cz, float radius, float vx,
                                    float vz, float strength) {
  for (int i = 0; i < kCubeCount; i++) {
    Cube& c = cubes[i];
    float minx = c.pos.x - c.size;
    float maxx = c.pos.x + c.size;
    float minz = c.pos.z - c.size;
    float maxz = c.pos.z + c.size;
    float qx = cx;
    if (qx < minx) qx = minx;
    if (qx > maxx) qx = maxx;
    float qz = cz;
    if (qz < minz) qz = minz;
    if (qz > maxz) qz = maxz;
    float dx = cx - qx;
    float dz = cz - qz;
    float d2 = dx * dx + dz * dz;
    if (d2 > radius * radius) continue;

    float ground = terrainHeight(cx, cz);
    if (c.pos.y > ground + 2.4F) continue;

    float dist = sqrtf(d2);
    float nx;
    float nz;
    if (dist < 0.0001F) {
      nx = c.pos.x - cx;
      nz = c.pos.z - cz;
      float n2 = nx * nx + nz * nz;
      if (n2 < 0.0001F) {
        nx = 1.0F;
        nz = 0.0F;
      } else {
        float inv = 1.0F / sqrtf(n2);
        nx *= inv;
        nz *= inv;
      }
      dist = 0.0F;
    } else {
      nx = dx / dist;
      nz = dz / dist;
    }

    float pen = radius - dist;
    c.pos.x -= nx * pen;
    c.pos.z -= nz * pen;

    float rel = vx * (-nx) + vz * (-nz);
    if (rel < 0.0F) rel = 0.0F;
    float kick = rel * strength + 3.0F * strength;
    c.vel.x -= nx * kick;
    c.vel.z -= nz * kick;
    if (rel > 4.0F) c.vel.y += rel * 0.22F;
  }
}

void RacerGame::updateCubes(float dt) {
  if (dt < 0.001F) dt = 0.001F;
  if (dt > 0.05F) dt = 0.05F;

  for (int i = 0; i < kCubeCount; i++) {
    Cube& c = cubes[i];
    c.vel.y -= kGravity * dt;
    c.vel.x *= (1.0F - kAirDrag * dt);
    c.vel.z *= (1.0F - kAirDrag * dt);
    c.pos.x += c.vel.x * dt;
    c.pos.y += c.vel.y * dt;
    c.pos.z += c.vel.z * dt;

    float ground = terrainHeight(c.pos.x, c.pos.z);
    if (c.pos.y <= ground) {
      c.pos.y = ground;
      if (c.vel.y < 0.0F) c.vel.y = 0.0F;
      float damp = 1.0F - kGroundFric * dt;
      if (damp < 0.0F) damp = 0.0F;
      c.vel.x *= damp;
      c.vel.z *= damp;
      float hx = terrainHeight(c.pos.x + 1.2F, c.pos.z);
      float hz = terrainHeight(c.pos.x, c.pos.z + 1.2F);
      c.vel.x += (ground - hx) * 14.0F * dt;
      c.vel.z += (ground - hz) * 14.0F * dt;
    }

    float limit = kWorld - c.size;
    if (c.pos.x < -limit) {
      c.pos.x = -limit;
      if (c.vel.x < 0.0F) c.vel.x *= -0.3F;
    }
    if (c.pos.x > limit) {
      c.pos.x = limit;
      if (c.vel.x > 0.0F) c.vel.x *= -0.3F;
    }
    if (c.pos.z < -limit) {
      c.pos.z = -limit;
      if (c.vel.z < 0.0F) c.vel.z *= -0.3F;
    }
    if (c.pos.z > limit) {
      c.pos.z = limit;
      if (c.vel.z > 0.0F) c.vel.z *= -0.3F;
    }
  }

  for (int pass = 0; pass < 4; pass++) {
    for (int i = 0; i < kCubeCount; i++) {
      for (int j = i + 1; j < kCubeCount; j++) {
        Cube& a = cubes[i];
        Cube& b = cubes[j];
        float ox = (a.size + b.size) - fabsf(a.pos.x - b.pos.x);
        float oz = (a.size + b.size) - fabsf(a.pos.z - b.pos.z);
        float ay0 = a.pos.y;
        float ay1 = a.pos.y + a.size * 2.0F;
        float by0 = b.pos.y;
        float by1 = b.pos.y + b.size * 2.0F;
        float oy = (ay1 < by1 ? ay1 : by1) - (ay0 > by0 ? ay0 : by0);
        if (ox <= 0.0F || oz <= 0.0F || oy <= 0.0F) continue;

        if (oy <= ox && oy <= oz) {
          if (a.pos.y < b.pos.y) {
            b.pos.y += oy;
            if (b.vel.y < 0.0F) b.vel.y = 0.0F;
          } else {
            a.pos.y += oy;
            if (a.vel.y < 0.0F) a.vel.y = 0.0F;
          }
        } else if (ox < oz) {
          float dir = (a.pos.x < b.pos.x) ? -1.0F : 1.0F;
          a.pos.x += dir * ox * 0.5F;
          b.pos.x -= dir * ox * 0.5F;
          float rv = a.vel.x - b.vel.x;
          a.vel.x -= rv * 0.5F;
          b.vel.x += rv * 0.5F;
        } else {
          float dir = (a.pos.z < b.pos.z) ? -1.0F : 1.0F;
          a.pos.z += dir * oz * 0.5F;
          b.pos.z -= dir * oz * 0.5F;
          float rv = a.vel.z - b.vel.z;
          a.vel.z -= rv * 0.5F;
          b.vel.z += rv * 0.5F;
        }
      }
    }
  }

  float c = Math::cos(carYaw);
  float s = Math::sin(carYaw);
  pushCubesFromCircle(carPos.x, carPos.z, 2.4F, s * carSpeed, c * carSpeed,
                      1.15F);
  if (!inCar) {
    float pc = Math::cos(playerYaw);
    float ps = Math::sin(playerYaw);
    pushCubesFromCircle(playerPos.x, playerPos.z, 0.7F, ps * 6.0F, pc * 6.0F,
                        0.55F);
  }
}

void RacerGame::drawWorld() {
  Vec4 focus = inCar ? carPos : playerPos;
  // Tiles stay 12 units so the checker border and the snap-flicker remain.
  // Each tile edge is split so the line follows the hills instead of one chord.
  const float tile = 12.0F;
  const int n = 5;
  const int sub = 3;
  const float seg = tile / static_cast<float>(sub);
  Color snow(248.0F, 236.0F, 228.0F);
  Color shade(186.0F, 170.0F, 214.0F);
  float x0 = floorf(focus.x / tile) * tile;
  float z0 = floorf(focus.z / tile) * tile;
  for (int i = -n; i <= n; i++) {
    for (int j = -n; j < n; j++) {
      float x = x0 + static_cast<float>(i) * tile;
      float z = z0 + static_cast<float>(j) * tile;
      const Color& col = ((i + j) & 1) ? snow : shade;
      for (int s = 0; s < sub; s++) {
        float zA = z + seg * static_cast<float>(s);
        float zB = zA + seg;
        line(Vec4(x, terrainHeight(x, zA) + 0.04F, zA),
             Vec4(x, terrainHeight(x, zB) + 0.04F, zB), col);
      }
    }
  }
  for (int j = -n; j <= n; j++) {
    for (int i = -n; i < n; i++) {
      float z = z0 + static_cast<float>(j) * tile;
      float x = x0 + static_cast<float>(i) * tile;
      const Color& col = ((i + j) & 1) ? snow : shade;
      for (int s = 0; s < sub; s++) {
        float xA = x + seg * static_cast<float>(s);
        float xB = xA + seg;
        line(Vec4(xA, terrainHeight(xA, z) + 0.04F, z),
             Vec4(xB, terrainHeight(xB, z) + 0.04F, z), col);
      }
    }
  }
}

void RacerGame::drawCubes() {
  Color ice(214.0F, 196.0F, 232.0F);
  Color ghost(255.0F, 176.0F, 214.0F);
  float g = ((glitchPhase & 16) != 0) ? 0.07F : -0.04F;
  for (int i = 0; i < kCubeCount; i++) {
    drawBoxYaw(cubes[i].pos, 0.0F, cubes[i].size, cubes[i].size, cubes[i].size,
               ice);
    Vec4 echo = cubes[i].pos;
    echo.x += g;
    echo.z -= g;
    drawBoxYaw(echo, 0.0F, cubes[i].size, cubes[i].size, cubes[i].size, ghost);
  }
}

void RacerGame::drawBoxLocal(const Vec4& origin, const Vec4& right,
                             const Vec4& up, const Vec4& fwd, float hx,
                             float hy, float hz, const Color& color) {
  Vec4 cnr[8];
  int n = 0;
  for (int y = 0; y < 2; y++) {
    float yy = (y == 0) ? 0.0F : hy * 2.0F;
    for (int z = -1; z <= 1; z += 2) {
      for (int x = -1; x <= 1; x += 2) {
        cnr[n++] = origin + right * (hx * static_cast<float>(x)) + up * yy +
                   fwd * (hz * static_cast<float>(z));
      }
    }
  }
  auto L = [&](int a, int b) { line(cnr[a], cnr[b], color); };
  L(0, 1);
  L(1, 3);
  L(3, 2);
  L(2, 0);
  L(4, 5);
  L(5, 7);
  L(7, 6);
  L(6, 4);
  L(0, 4);
  L(1, 5);
  L(2, 6);
  L(3, 7);
}

void RacerGame::drawWheel(const Vec4& center, const Vec4& axleIn, float spin,
                          float radius, const Color& rubber, const Color& hub) {
  Vec4 axle = axleIn;
  axle.normalize();
  Vec4 tmp(0.0F, 1.0F, 0.0F);
  if (fabsf(axle.y) > 0.85F) tmp = Vec4(0.0F, 0.0F, 1.0F);
  Vec4 u = tmp.cross(axle);
  u.normalize();
  Vec4 v = axle.cross(u);
  v.normalize();
  const int N = 14;
  const float halfW = 0.16F;
  for (int side = -1; side <= 1; side += 2) {
    Vec4 c = center + axle * (halfW * static_cast<float>(side));
    Vec4 prev;
    for (int i = 0; i <= N; i++) {
      float a = spin + (static_cast<float>(i % N) / static_cast<float>(N)) *
                           Math::PI * 2.0F;
      Vec4 p = c + u * (Math::cos(a) * radius) + v * (Math::sin(a) * radius);
      if (i > 0) line(prev, p, rubber);
      prev = p;
    }
  }
  for (int i = 0; i < N; i += 2) {
    float a = spin + (static_cast<float>(i) / static_cast<float>(N)) * Math::PI * 2.0F;
    Vec4 inner = center + u * (Math::cos(a) * radius) + v * (Math::sin(a) * radius);
    Vec4 outer = inner + axle * halfW - axle * halfW;
    (void)outer;
    line(center + axle * (-halfW) + u * (Math::cos(a) * radius) +
             v * (Math::sin(a) * radius),
         center + axle * halfW + u * (Math::cos(a) * radius) +
             v * (Math::sin(a) * radius),
         rubber);
  }
  Color spoke(255.0F, 206.0F, 156.0F);
  for (int i = 0; i < 4; i++) {
    float a = spin + static_cast<float>(i) * Math::PI * 0.5F;
    line(center, center + u * (Math::cos(a) * radius * 0.82F) +
                     v * (Math::sin(a) * radius * 0.82F),
         spoke);
  }
  line(center - axle * halfW, center + axle * halfW, hub);
}

void RacerGame::drawPenguin(const Vec4& feet, const Vec4& fwd, const Vec4& right,
                            const Vec4& up) {
  Color black(92.0F, 52.0F, 118.0F);
  Color white(255.0F, 214.0F, 226.0F);
  Color orange(255.0F, 156.0F, 104.0F);
  Color eye(64.0F, 36.0F, 82.0F);
  auto ring = [&](const Vec4& c, float rx, float ry, const Vec4& axisA,
                  const Vec4& axisB, const Color& col) {
    const int N = 10;
    Vec4 prev;
    for (int i = 0; i <= N; i++) {
      float a = (static_cast<float>(i % N) / static_cast<float>(N)) * Math::PI * 2.0F;
      Vec4 p = c + axisA * (Math::cos(a) * rx) + axisB * (Math::sin(a) * ry);
      if (i > 0) line(prev, p, col);
      prev = p;
    }
  };
  Vec4 hip = feet + up * 0.15F;
  Vec4 belly = feet + up * 0.55F + fwd * 0.06F;
  Vec4 chest = feet + up * 0.95F;
  Vec4 neck = feet + up * 1.25F;
  ring(hip, 0.28F, 0.22F, right, fwd, black);
  ring(belly, 0.40F, 0.30F, right, fwd, black);
  ring(chest, 0.34F, 0.26F, right, fwd, black);
  ring(belly + fwd * 0.08F, 0.22F, 0.16F, right, up, white);
  ring(neck, 0.16F, 0.14F, right, fwd, black);
  Vec4 head = feet + up * 1.55F;
  ring(head, 0.24F, 0.22F, right, fwd, black);
  ring(head, 0.24F, 0.22F, right, up, black);
  float go = ((glitchPhase & 16) != 0) ? 0.05F : -0.04F;
  ring(head + right * go, 0.24F, 0.22F, right, fwd, Color(130.0F, 230.0F, 255.0F));
  Vec4 beak = head + fwd * 0.22F + up * 0.02F;
  line(head + fwd * 0.08F + up * 0.06F, beak + fwd * 0.28F, orange);
  line(head + fwd * 0.08F - up * 0.04F, beak + fwd * 0.28F, orange);
  line(head + right * 0.08F + fwd * 0.1F, beak + fwd * 0.28F, orange);
  line(head - right * 0.08F + fwd * 0.1F, beak + fwd * 0.28F, orange);
  line(head + right * 0.1F + fwd * 0.16F + up * 0.06F,
       head + right * 0.1F + fwd * 0.22F + up * 0.06F, eye);
  line(head - right * 0.1F + fwd * 0.16F + up * 0.06F,
       head - right * 0.1F + fwd * 0.22F + up * 0.06F, eye);
  Vec4 shL = chest - right * 0.28F;
  Vec4 shR = chest + right * 0.28F;
  line(shL, shL - right * 0.28F - up * 0.45F + fwd * 0.05F, black);
  line(shR, shR + right * 0.28F - up * 0.45F + fwd * 0.05F, black);
  line(shL - right * 0.28F - up * 0.45F, shL - right * 0.05F - up * 0.55F, black);
  line(shR + right * 0.28F - up * 0.45F, shR + right * 0.05F - up * 0.55F, black);
  Vec4 footL = feet - right * 0.12F;
  Vec4 footR = feet + right * 0.12F;
  line(footL, footL + fwd * 0.28F - right * 0.08F, orange);
  line(footL, footL + fwd * 0.28F + right * 0.02F, orange);
  line(footL + fwd * 0.28F - right * 0.08F, footL + fwd * 0.28F + right * 0.02F, orange);
  line(footR, footR + fwd * 0.28F - right * 0.02F, orange);
  line(footR, footR + fwd * 0.28F + right * 0.08F, orange);
  line(footR + fwd * 0.28F - right * 0.02F, footR + fwd * 0.28F + right * 0.08F, orange);
}

void RacerGame::drawWaxPips(const Vec4& head, const Vec4& right, const Vec4& up) {
  Color pip(255.0F, 186.0F, 96.0F);
  for (int i = 0; i < invCount; i++) {
    float x = (static_cast<float>(i) - 0.5F * static_cast<float>(invCount - 1)) *
              0.14F;
    Vec4 a = head + right * x + up * 0.38F;
    line(a, a + up * 0.16F, pip);
  }
}

void RacerGame::drawPlayer() {
  if (inCar) {
    Vec4 fwd, right, up;
    carBasis(&fwd, &right, &up);
    Vec4 seat(carPos.x, carPos.y + 0.2F, carPos.z);
    seat = seat + up * 0.05F - fwd * 0.15F;
    drawPenguin(seat, fwd, right, up);
    drawWaxPips(seat + up * 1.7F, right, up);
    return;
  }
  float cy = Math::cos(playerYaw);
  float sy = Math::sin(playerYaw);
  Vec4 fwd(sy, 0.0F, cy);
  Vec4 right(cy, 0.0F, -sy);
  Vec4 up(0.0F, 1.0F, 0.0F);
  drawPenguin(playerPos, fwd, right, up);
  drawWaxPips(playerPos + up * 1.85F, right, up);
}

void RacerGame::drawCar() {
  Color body(240.0F, 112.0F, 146.0F);
  Color hood(255.0F, 214.0F, 186.0F);
  Color cabin(168.0F, 176.0F, 232.0F);
  Color rubber(48.0F, 32.0F, 64.0F);
  Color hubc(255.0F, 228.0F, 196.0F);
  Color bar(78.0F, 48.0F, 104.0F);
  Color hint = deny > 0.0F ? Color(255.0F, 120.0F, 168.0F)
                            : Color(255.0F, 210.0F, 140.0F);
  Color ghost((glitchPhase & 32) != 0 ? 255.0F : 130.0F,
              (glitchPhase & 32) != 0 ? 160.0F : 230.0F,
              (glitchPhase & 32) != 0 ? 214.0F : 255.0F);

  Vec4 fwd, right, up;
  carBasis(&fwd, &right, &up);
  Vec4 origin(carPos.x, carPos.y, carPos.z);

  drawBoxLocal(origin, right, up, fwd, 1.05F, 0.48F, 2.05F, body);
  float go = ((glitchPhase & 16) != 0) ? 0.06F : -0.04F;
  drawBoxLocal(origin + right * go, right, up, fwd, 1.05F, 0.48F, 2.05F, ghost);
  drawBoxLocal(origin + fwd * 1.15F + up * 0.15F, right, up, fwd, 0.85F, 0.28F,
               0.7F, hood);
  drawBoxLocal(origin - fwd * 1.15F + up * 0.05F, right, up, fwd, 0.9F, 0.32F,
               0.55F, body);
  drawBoxLocal(origin + up * 0.95F - fwd * 0.15F, right, up, fwd, 0.82F, 0.38F,
               0.75F, cabin);

  Vec4 cageL = origin + up * 0.95F - right * 0.7F - fwd * 0.15F;
  Vec4 cageR = origin + up * 0.95F + right * 0.7F - fwd * 0.15F;
  Vec4 cageT = origin + up * 1.85F - fwd * 0.15F;
  line(cageL, cageT - right * 0.15F, bar);
  line(cageR, cageT + right * 0.15F, bar);
  line(cageT - right * 0.55F, cageT + right * 0.55F, bar);
  line(cageT - fwd * 0.45F, cageT + fwd * 0.35F, bar);

  const float wx[4] = {-1.2F, 1.2F, -1.2F, 1.2F};
  const float wz[4] = {-1.55F, -1.55F, 1.55F, 1.55F};
  float cs = Math::cos(steerAngle);
  float ss = Math::sin(steerAngle);
  for (int i = 0; i < 4; i++) {
    float x = carPos.x + Math::cos(carYaw) * wx[i] + Math::sin(carYaw) * wz[i];
    float z = carPos.z - Math::sin(carYaw) * wx[i] + Math::cos(carYaw) * wz[i];
    float gy = terrainHeight(x, z);
    float radius = corners[i].tire.radius;
    Vec4 mount = origin + right * wx[i] + fwd * wz[i];
    float travel = corners[i].shock.travel;
    float hubY = mount.y - kStrut;
    float planted = gy + radius;
    float maxHub = mount.y - (kStrut - travel);
    if (hubY < planted) hubY = planted;
    if (hubY > maxHub) hubY = maxHub;
    Vec4 hub(x, hubY, z);
    Vec4 axle = right;
    if (wz[i] > 0.0F) {
      // steerAngle uses the same sign as carYaw. The axle is the wheel's
      // right, so a positive angle swings the nose toward +right.
      axle = right * cs - fwd * ss;
      axle.normalize();
    }
    drawWheel(hub, axle, wheelSpin, radius, rubber, hubc);
  }

  drawMountedArms(origin, right, up, fwd);

  if (!inCar) {
    float dx = playerPos.x - carPos.x;
    float dz = playerPos.z - carPos.z;
    if (dx * dx + dz * dz < kEnterDist * kEnterDist) {
      for (int i = 0; i < 16; i++) {
        float a0 = (static_cast<float>(i) / 16.0F) * Math::PI * 2.0F;
        float a1 = (static_cast<float>(i + 1) / 16.0F) * Math::PI * 2.0F;
        line(Vec4(carPos.x + Math::cos(a0) * 2.6F, carPos.y + 0.08F,
                  carPos.z + Math::sin(a0) * 2.6F),
             Vec4(carPos.x + Math::cos(a1) * 2.6F, carPos.y + 0.08F,
                  carPos.z + Math::sin(a1) * 2.6F),
             hint);
      }
    }
  }
}

bool RacerGame::nearBench() const {
  float dx = playerPos.x - benchPos.x;
  float dz = playerPos.z - benchPos.z;
  return dx * dx + dz * dz < 3.6F * 3.6F;
}

bool RacerGame::nearCarCraft() const {
  float dx = playerPos.x - carPos.x;
  float dz = playerPos.z - carPos.z;
  return dx * dx + dz * dz < 4.2F * 4.2F;
}

bool RacerGame::socketTaken(int socket) const {
  for (int i = 0; i < mountCount; i++) {
    if (mounts[i].socket == socket) return true;
  }
  return false;
}

void RacerGame::drawArmShape(const Vec4& origin, const Vec4& right,
                             const Vec4& up, const Vec4& fwd, float x0,
                             float x1, float y0, float y1, float spread,
                             const Color& color, bool glow) {
  Vec4 a = origin + right * x0 + up * y0 + fwd * (-spread);
  Vec4 b = origin + right * x0 + up * y0 + fwd * spread;
  Vec4 o = origin + right * x1 + up * y1;
  if (glow) {
    ink(a, o, color);
    ink(b, o, color);
    ink(a, b, color);
  } else {
    line(a, o, color);
    line(b, o, color);
    line(a, b, color);
  }
}

void RacerGame::drawMountedArms(const Vec4& origin, const Vec4& right,
                                const Vec4& up, const Vec4& fwd) {
  const float wx[4] = {-1.2F, 1.2F, -1.2F, 1.2F};
  const float wz[4] = {-1.55F, -1.55F, 1.55F, 1.55F};
  Color waxc(255.0F, 196.0F, 96.0F);
  Color test(186.0F, 160.0F, 255.0F);
  Color blob(255.0F, 232.0F, 170.0F);
  for (int i = 0; i < mountCount; i++) {
    int corner = mounts[i].socket % 4;
    bool upper = mounts[i].socket >= 4;
    float tweak = mounts[i].part.tweak;
    if (tweak < 0.5F) tweak = 1.0F;
    float sx = wx[corner] > 0.0F ? 1.0F : -1.0F;
    Vec4 outward = right * sx;
    Vec4 at = origin + fwd * wz[corner];
    float spread = (upper ? 0.22F : 0.34F) * tweak;
    float x0 = upper ? 0.40F : 0.48F;
    float x1 = fabsf(wx[corner]) * (upper ? 0.86F : 0.98F);
    float y0 = upper ? 0.62F : 0.08F;
    float y1 = upper ? -0.75F : -(kStrut - 0.22F);
    const Color& col = mounts[i].welded ? waxc : test;
    drawArmShape(at, outward, up, fwd, x0, x1, y0, y1, spread, col, true);
    if (!mounts[i].welded) continue;
    Vec4 a = at + outward * x0 + up * y0 + fwd * (-spread);
    Vec4 b = at + outward * x0 + up * y0 + fwd * spread;
    Vec4 o = at + outward * x1 + up * y1;
    line(a, a + up * 0.08F, blob);
    line(b, b + up * 0.08F, blob);
    line(o, o + up * 0.08F, blob);
  }
}

void RacerGame::drawBench() {
  Vec4 up(0.0F, 1.0F, 0.0F);
  Vec4 right(1.0F, 0.0F, 0.0F);
  Vec4 fwd(0.0F, 0.0F, 1.0F);
  float top = benchPos.y + 0.92F;
  float h = kBenchHalf;
  Color rim = deny > 0.0F ? Color(255.0F, 120.0F, 168.0F)
                          : (nearBench() ? Color(255.0F, 210.0F, 150.0F)
                                         : Color(186.0F, 160.0F, 220.0F));
  Color grid(214.0F, 196.0F, 232.0F);
  Vec4 c00(benchPos.x - h, top, benchPos.z - h);
  Vec4 c10(benchPos.x + h, top, benchPos.z - h);
  Vec4 c11(benchPos.x + h, top, benchPos.z + h);
  Vec4 c01(benchPos.x - h, top, benchPos.z + h);
  ink(c00, c10, rim);
  ink(c10, c11, rim);
  ink(c11, c01, rim);
  ink(c01, c00, rim);
  line(c00, Vec4(c00.x, benchPos.y, c00.z), rim);
  line(c10, Vec4(c10.x, benchPos.y, c10.z), rim);
  line(c11, Vec4(c11.x, benchPos.y, c11.z), rim);
  line(c01, Vec4(c01.x, benchPos.y, c01.z), rim);
  for (int i = 1; i < 4; i++) {
    float t = -h + (h * 2.0F) * (static_cast<float>(i) / 4.0F);
    line(Vec4(benchPos.x - h, top + 0.02F, benchPos.z + t),
         Vec4(benchPos.x + h, top + 0.02F, benchPos.z + t), grid);
    line(Vec4(benchPos.x + t, top + 0.02F, benchPos.z - h),
         Vec4(benchPos.x + t, top + 0.02F, benchPos.z + h), grid);
  }

  if (tableHas) {
    bool upper = tablePart.kind == PartUpperArm;
    float tweak = tablePart.tweak > 0.5F ? tablePart.tweak : 1.0F;
    float x0 = upper ? -0.55F : -0.85F;
    float x1 = upper ? 0.85F : 1.15F;
    float spread = (upper ? 0.55F : 0.72F) * tweak;
    drawArmShape(Vec4(benchPos.x, top + 0.02F, benchPos.z), right, up, fwd, x0,
                 x1, 0.42F, 0.08F, spread, Color(255.0F, 196.0F, 96.0F), true);
  }
  if (atBench) {
    Vec4 ga, gb, gt;
    armGuide(&ga, &gb, &gt);
    const Vec4* ends[3][2] = {{&ga, &gt}, {&gb, &gt}, {&ga, &gb}};
    Color guide(186.0F, 166.0F, 214.0F);
    Color inked(255.0F, 196.0F, 96.0F);
    const int samples = 6;
    for (int e = 0; e < 3; e++) {
      line(*ends[e][0], *ends[e][1], guide);
      for (int s = 0; s < samples; s++) {
        if ((traceBits & (1u << (e * samples + s))) == 0) continue;
        float t0 = static_cast<float>(s) / static_cast<float>(samples);
        float t1 = static_cast<float>(s + 1) / static_cast<float>(samples);
        Vec4 p0 = *ends[e][0] + (*ends[e][1] - *ends[e][0]) * t0;
        Vec4 p1 = *ends[e][0] + (*ends[e][1] - *ends[e][0]) * t1;
        p0.y += 0.03F;
        p1.y += 0.03F;
        line(p0, p1, inked);
      }
    }
    float cy = top + 0.12F;
    Vec4 cur(benchPos.x + cursorX, cy, benchPos.z + cursorZ);
    Color mark = engine->pad.getPressed().Square ? Color(255.0F, 120.0F, 168.0F)
                                                 : Color(255.0F, 236.0F, 214.0F);
    float m = 0.28F;
    line(cur + Vec4(-m, 0.0F, 0.0F), cur + Vec4(m, 0.0F, 0.0F), mark);
    line(cur + Vec4(0.0F, 0.0F, -m), cur + Vec4(0.0F, 0.0F, m), mark);
    line(cur, cur + up * 0.35F, mark);
  }

  for (int cup = 0; cup < 2; cup++) {
    float x = benchPos.x + (cup == 0 ? -1.35F : 1.35F);
    float z = benchPos.z + 1.45F;
    Color cupCol = (recipe == cup) ? Color(255.0F, 196.0F, 96.0F) : grid;
    line(Vec4(x - 0.16F, top + 0.03F, z - 0.16F),
         Vec4(x + 0.16F, top + 0.03F, z - 0.16F), cupCol);
    line(Vec4(x + 0.16F, top + 0.03F, z - 0.16F),
         Vec4(x + 0.16F, top + 0.03F, z + 0.16F), cupCol);
    line(Vec4(x + 0.16F, top + 0.03F, z + 0.16F),
         Vec4(x - 0.16F, top + 0.03F, z + 0.16F), cupCol);
    line(Vec4(x - 0.16F, top + 0.03F, z + 0.16F),
         Vec4(x - 0.16F, top + 0.03F, z - 0.16F), cupCol);
  }

  int marks = static_cast<int>(wax);
  if (marks > 12) marks = 12;
  for (int i = 0; i < marks; i++) {
    float x = benchPos.x - h + 0.28F + static_cast<float>(i) * 0.30F;
    Vec4 base(x, top + 0.03F, benchPos.z - h + 0.22F);
    Color waxc = (i % 3 == 0) ? Color(240.0F, 112.0F, 146.0F)
                              : ((i % 3 == 1) ? Color(168.0F, 176.0F, 232.0F)
                                              : Color(255.0F, 196.0F, 96.0F));
    line(base, base + up * 0.26F, waxc);
    line(base + up * 0.26F, base + up * 0.34F, Color(255.0F, 236.0F, 214.0F));
  }

  Color zone = deny > 0.0F ? Color(255.0F, 120.0F, 168.0F)
                           : (nearBench() ? Color(255.0F, 210.0F, 150.0F)
                                          : Color(186.0F, 160.0F, 220.0F));
  for (int i = 0; i < 16; i++) {
    float a0 = (static_cast<float>(i) / 16.0F) * Math::PI * 2.0F;
    float a1 = (static_cast<float>(i + 1) / 16.0F) * Math::PI * 2.0F;
    line(Vec4(benchPos.x + Math::cos(a0) * 3.4F, benchPos.y + 0.08F,
              benchPos.z + Math::sin(a0) * 3.4F),
         Vec4(benchPos.x + Math::cos(a1) * 3.4F, benchPos.y + 0.08F,
              benchPos.z + Math::sin(a1) * 3.4F),
         zone);
  }
}

void RacerGame::armGuide(Vec4* a, Vec4* b, Vec4* tip) const {
  float top = benchPos.y + 0.98F;
  float tweak = draftTweak > 0.5F ? draftTweak : 1.0F;
  bool upper = recipe != 0;
  float x0 = upper ? -0.55F : -0.85F;
  float x1 = upper ? 0.85F : 1.15F;
  float spread = (upper ? 0.55F : 0.72F) * tweak;
  *a = Vec4(benchPos.x + x0, top, benchPos.z - spread);
  *b = Vec4(benchPos.x + x0, top, benchPos.z + spread);
  *tip = Vec4(benchPos.x + x1, top, benchPos.z);
}

void RacerGame::say(const char* text) {
  int i = 0;
  while (text[i] != '\0' && i < 31) {
    helpMsg[i] = text[i];
    i++;
  }
  helpMsg[i] = '\0';
  helpHold = 2.2F;
}

void RacerGame::updateCraft(float dt) {
  if (deny > 0.0F) deny -= dt;
  if (helpHold > 0.0F) {
    helpHold -= dt;
    if (helpHold < 0.0F) helpHold = 0.0F;
  }
  const auto& click = engine->pad.getClicked();

  if (atBench) {
    const auto& move = engine->pad.getLeftJoyPad();
    const auto& look = engine->pad.getRightJoyPad();
    float lx = analog(move.h);
    float ly = analog(move.v);
    float rx = analog(look.h);
    float ry = analog(look.v);
    float sy = Math::sin(benchYaw);
    float cy = Math::cos(benchYaw);
    cursorX += (sy * (-ly) + cy * (-lx)) * 3.2F * dt;
    cursorZ += (cy * (-ly) - sy * (-lx)) * 3.2F * dt;
    if (cursorX < -1.9F) cursorX = -1.9F;
    if (cursorX > 1.9F) cursorX = 1.9F;
    if (cursorZ < -1.9F) cursorZ = -1.9F;
    if (cursorZ > 1.9F) cursorZ = 1.9F;
    benchYaw -= rx * 1.7F * dt;
    while (benchYaw > Math::PI) benchYaw -= Math::PI * 2.0F;
    while (benchYaw < -Math::PI) benchYaw += Math::PI * 2.0F;
    benchPitch += ry * 1.15F * dt;
    if (benchPitch < 0.42F) benchPitch = 0.42F;
    if (benchPitch > 1.22F) benchPitch = 1.22F;

    if (click.Select) helpVisible = !helpVisible;
    if (click.DpadUp || click.DpadDown) {
      recipe = 1 - recipe;
      traceBits = 0;
      traceLatched = false;
      say(recipe == 0 ? "LOWER ARM" : "UPPER ARM");
    }
    if (click.DpadLeft || click.DpadRight) {
      draftTweak += click.DpadRight ? 0.08F : -0.08F;
      if (draftTweak < 0.7F) draftTweak = 0.7F;
      if (draftTweak > 1.4F) draftTweak = 1.4F;
      if (tableHas) tablePart.tweak = draftTweak;
      traceBits = 0;
      traceLatched = false;
      say(click.DpadRight ? "WIDER" : "NARROWER");
    }
    bool drawing = engine->pad.getPressed().Square != 0;
    if (drawing) {
      if (wax < kWaxDraw) {
        if (!traceLatched) {
          deny = 0.7F;
          say("NEED 1 WAX");
          traceLatched = true;
        }
      } else {
        Vec4 ga, gb, gt;
        armGuide(&ga, &gb, &gt);
        const Vec4 ends[3][2] = {{ga, gt}, {gb, gt}, {ga, gb}};
        const int samples = 6;
        float cx = benchPos.x + cursorX;
        float cz = benchPos.z + cursorZ;
        for (int e = 0; e < 3; e++) {
          for (int s = 0; s < samples; s++) {
            float t = (static_cast<float>(s) + 0.5F) / static_cast<float>(samples);
            Vec4 p = ends[e][0] + (ends[e][1] - ends[e][0]) * t;
            float dx = p.x - cx;
            float dz = p.z - cz;
            if (dx * dx + dz * dz < 0.34F * 0.34F) {
              traceBits |= 1u << (e * samples + s);
            }
          }
        }
        if (!traceLatched && traceBits == ((1u << (3 * samples)) - 1u)) {
          PartKind want = recipe == 0 ? PartLowerArm : PartUpperArm;
          tablePart.kind = want;
          tablePart.tweak = draftTweak;
          tableHas = true;
          wax -= kWaxDraw;
          traceBits = 0;
          traceLatched = true;
          say(want == PartLowerArm ? "DREW LOWER ARM" : "DREW UPPER ARM");
        }
      }
    } else {
      traceLatched = false;
    }
    if (click.Circle) {
      if (!tableHas) {
        deny = 0.7F;
        say("DRAW A PART FIRST");
      } else if (wax < kWaxCopy) {
        deny = 0.7F;
        say("NEED 2 WAX");
      } else if (invCount >= kInvCap) {
        deny = 0.7F;
        say("PACK IS FULL");
      } else {
        inv[invCount++] = tablePart;
        wax -= kWaxCopy;
        say("COPIED");
      }
    }
    if (click.L1) {
      if (invCount <= 0) {
        deny = 0.7F;
        say("NOTHING TO PUT BACK");
      } else {
        CraftPart incoming = inv[0];
        for (int i = 1; i < invCount; i++) inv[i - 1] = inv[i];
        invCount--;
        if (tableHas && invCount < kInvCap) inv[invCount++] = tablePart;
        tablePart = incoming;
        tableHas = incoming.kind != PartNone;
        if (tablePart.tweak < 0.5F) tablePart.tweak = 1.0F;
        if (tableHas) {
          recipe = tablePart.kind == PartUpperArm ? 1 : 0;
          draftTweak = tablePart.tweak;
        }
        traceBits = 0;
        say("ON THE TABLE");
      }
    }
    return;
  }

  if (!nearCarCraft()) return;

  if (click.Square) {
    int socket = -1;
    if (invCount > 0) {
      int begin = inv[0].kind == PartUpperArm ? 4 : 0;
      for (int i = 0; i < 4; i++) {
        if (!socketTaken(begin + i)) {
          socket = begin + i;
          break;
        }
      }
    }
    if (invCount <= 0 || socket < 0 || mountCount >= kSocketCount) {
      deny = 0.45F;
    } else {
      mounts[mountCount].part = inv[0];
      mounts[mountCount].socket = socket;
      mounts[mountCount].welded = false;
      mountCount++;
      for (int i = 1; i < invCount; i++) inv[i - 1] = inv[i];
      invCount--;
    }
  }

  if (click.R1) {
    int idx = -1;
    for (int i = mountCount - 1; i >= 0; i--) {
      if (!mounts[i].welded) {
        idx = i;
        break;
      }
    }
    if (idx >= 0) {
      int cur = mounts[idx].socket;
      int begin = cur >= 4 ? 4 : 0;
      int local = cur - begin;
      for (int step = 1; step <= 4; step++) {
        int next = begin + (local + step) % 4;
        if (next == cur || !socketTaken(next)) {
          mounts[idx].socket = next;
          break;
        }
      }
    }
  }

  if (click.Circle) {
    int idx = -1;
    for (int i = mountCount - 1; i >= 0; i--) {
      if (!mounts[i].welded) {
        idx = i;
        break;
      }
    }
    if (idx < 0 || wax < kWaxWeld) {
      deny = 0.45F;
    } else {
      mounts[idx].welded = true;
      wax -= kWaxWeld;
    }
  }

  if (click.L1 && mountCount > 0) {
    int best = 0;
    float bestD = 1.0e9F;
    const float wx[4] = {-1.2F, 1.2F, -1.2F, 1.2F};
    const float wz[4] = {-1.55F, -1.55F, 1.55F, 1.55F};
    float cy = Math::cos(carYaw);
    float sy = Math::sin(carYaw);
    for (int i = 0; i < mountCount; i++) {
      int corner = mounts[i].socket % 4;
      float x = carPos.x + cy * wx[corner] + sy * wz[corner];
      float z = carPos.z - sy * wx[corner] + cy * wz[corner];
      float dx = playerPos.x - x;
      float dz = playerPos.z - z;
      float d = dx * dx + dz * dz;
      if (d < bestD) {
        bestD = d;
        best = i;
      }
    }
    if (invCount >= kInvCap) {
      deny = 0.45F;
    } else {
      if (mounts[best].welded) {
        wax += kWaxWeld;
        if (wax > 48.0F) wax = 48.0F;
      }
      inv[invCount++] = mounts[best].part;
      for (int i = best + 1; i < mountCount; i++) mounts[i - 1] = mounts[i];
      mountCount--;
    }
  }
}

// 5x7, bit 4 is the left pixel. Unknown characters stay blank.
static void glyphRows(char ch, unsigned char out[7]) {
  const unsigned char* g = nullptr;
  switch (ch) {
    case 'A': {
      static const unsigned char r[7] = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
      g = r;
      break;
    }
    case 'B': {
      static const unsigned char r[7] = {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
      g = r;
      break;
    }
    case 'C': {
      static const unsigned char r[7] = {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E};
      g = r;
      break;
    }
    case 'D': {
      static const unsigned char r[7] = {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E};
      g = r;
      break;
    }
    case 'E': {
      static const unsigned char r[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
      g = r;
      break;
    }
    case 'F': {
      static const unsigned char r[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
      g = r;
      break;
    }
    case 'G': {
      static const unsigned char r[7] = {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E};
      g = r;
      break;
    }
    case 'H': {
      static const unsigned char r[7] = {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
      g = r;
      break;
    }
    case 'I': {
      static const unsigned char r[7] = {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E};
      g = r;
      break;
    }
    case 'K': {
      static const unsigned char r[7] = {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11};
      g = r;
      break;
    }
    case 'L': {
      static const unsigned char r[7] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
      g = r;
      break;
    }
    case 'M': {
      static const unsigned char r[7] = {0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x11};
      g = r;
      break;
    }
    case 'N': {
      static const unsigned char r[7] = {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11};
      g = r;
      break;
    }
    case 'O': {
      static const unsigned char r[7] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
      g = r;
      break;
    }
    case 'P': {
      static const unsigned char r[7] = {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
      g = r;
      break;
    }
    case 'Q': {
      static const unsigned char r[7] = {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D};
      g = r;
      break;
    }
    case 'R': {
      static const unsigned char r[7] = {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11};
      g = r;
      break;
    }
    case 'S': {
      static const unsigned char r[7] = {0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E};
      g = r;
      break;
    }
    case 'T': {
      static const unsigned char r[7] = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
      g = r;
      break;
    }
    case 'U': {
      static const unsigned char r[7] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
      g = r;
      break;
    }
    case 'W': {
      static const unsigned char r[7] = {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11};
      g = r;
      break;
    }
    case 'Y': {
      static const unsigned char r[7] = {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04};
      g = r;
      break;
    }
    case '1': {
      static const unsigned char r[7] = {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
      g = r;
      break;
    }
    case '2': {
      static const unsigned char r[7] = {0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F};
      g = r;
      break;
    }
    case '0': {
      static const unsigned char r[7] = {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E};
      g = r;
      break;
    }
    case '3': {
      static const unsigned char r[7] = {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E};
      g = r;
      break;
    }
    case '4': {
      static const unsigned char r[7] = {0x04, 0x0C, 0x14, 0x1F, 0x04, 0x04, 0x04};
      g = r;
      break;
    }
    case '5': {
      static const unsigned char r[7] = {0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E};
      g = r;
      break;
    }
    case '6': {
      static const unsigned char r[7] = {0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E};
      g = r;
      break;
    }
    case '7': {
      static const unsigned char r[7] = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08};
      g = r;
      break;
    }
    case '8': {
      static const unsigned char r[7] = {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E};
      g = r;
      break;
    }
    case '9': {
      static const unsigned char r[7] = {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E};
      g = r;
      break;
    }
    case 'X': {
      static const unsigned char r[7] = {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11};
      g = r;
      break;
    }
    default:
      break;
  }
  for (int i = 0; i < 7; i++) out[i] = (g != nullptr) ? g[i] : 0;
}

static int textLen(const char* s) {
  int n = 0;
  while (s[n] != '\0') n++;
  return n;
}

void RacerGame::drawHelp(float dt) {
  if (inCar || (!atBench && !nearBench())) return;
  if (atBench && !helpVisible && helpHold <= 0.0F) return;

  // Debug lines closer than about 10 units are rejected by Tyra's near clip.
  const float dist = 13.0F;
  float fov = engine->renderer.core.renderer3D.getFov();
  float halfH = dist * tanf(fov * 0.008726646F);
  float halfW = halfH * (512.0F / 448.0F);
  bool sheet = atBench && helpVisible;
  float barHalfW = halfW * (sheet ? 0.92F : 0.72F);
  float advance =
      sheet ? (barHalfW - 0.2F) / 18.0F : (barHalfW * 2.0F * 0.9F) / 20.0F;
  float cell = advance * (sheet ? 0.16F : 0.15F);
  char waxPage[8];
  int w = static_cast<int>(wax + 0.01F);
  if (w < 0) w = 0;
  if (w > 99) w = 99;
  waxPage[0] = 'W';
  waxPage[1] = 'A';
  waxPage[2] = 'X';
  waxPage[3] = ' ';
  int waxN = 4;
  if (w >= 10) waxPage[waxN++] = static_cast<char>('0' + (w / 10));
  waxPage[waxN++] = static_cast<char>('0' + (w % 10));
  waxPage[waxN] = '\0';

  // Same basis as Tyra's lookAt. Screen right is worldUp x back.
  Vec4 back = camPos - camLook;
  if (back.length() < 0.001F) return;
  back.normalize();
  Vec4 right = Vec4(0.0F, 1.0F, 0.0F).cross(back);
  if (right.length() < 0.001F) return;
  right.normalize();
  Vec4 up = back.cross(right);
  up.normalize();

  Color inkc(255.0F, 236.0F, 214.0F);
  auto paint = [&](const char* s, Vec4 pen) {
    int len = textLen(s);
    for (int i = 0; i < len; i++) {
      unsigned char rows[7];
      glyphRows(s[i], rows);
      Vec4 glyph = pen + right * (advance * static_cast<float>(i));
      glyph.w = 1.0F;
      for (int row = 0; row < 7; row++) {
        unsigned char bits = rows[row];
        int col = 0;
        while (col < 5) {
          if ((bits & static_cast<unsigned char>(0x10 >> col)) == 0) {
            col++;
            continue;
          }
          int run = col;
          while (run < 5 &&
                 (bits & static_cast<unsigned char>(0x10 >> run)) != 0) {
            run++;
          }
          float x0 = cell * static_cast<float>(col);
          float x1 = cell * static_cast<float>(run);
          if (x1 - x0 < cell * 0.85F) x1 = x0 + cell * 0.85F;
          float y = -cell * static_cast<float>(row);
          Vec4 a = glyph + right * x0 + up * y;
          Vec4 b = glyph + right * x1 + up * y;
          a.w = 1.0F;
          b.w = 1.0F;
          line(a, b, inkc);
          col = run;
        }
      }
    }
  };

  const char* lefts[4] = {"HOLD SQUARE TRACE", "CIRCLE COPIES 2", "L1 PUTS BACK",
                          "DPAD UP DOWN ARM"};
  const char* rights[4] = {"RIGHT STICK ORBITS", "TRIANGLE EXITS", "SELECT HIDES",
                           "LEFT RIGHT WIDTH"};
  bool flash = atBench && helpHold > 0.0F && helpMsg[0] != '\0';
  int total = sheet ? (5 + (flash ? 1 : 0)) : 1;
  float rowPitch = cell * 10.0F;
  float blockH = rowPitch * static_cast<float>(total);
  float barHalfH = blockH * 0.5F + cell * 1.2F;
  float yCenter = sheet ? -halfH * 0.42F : -halfH * 0.62F;
  Vec4 mid = camPos - back * dist + up * yCenter;
  mid.w = 1.0F;
  auto center = [&](const char* s, float y) {
    float textW = static_cast<float>(textLen(s)) * advance;
    Vec4 pen = mid - right * (textW * 0.5F) + up * y;
    pen.w = 1.0F;
    paint(s, pen);
  };

  Color band(42.0F, 20.0F, 58.0F);
  const int bandRows = 18;
  for (int i = 0; i < bandRows; i++) {
    float y = -barHalfH + (barHalfH * 2.0F) *
                               (static_cast<float>(i) + 0.5F) /
                               static_cast<float>(bandRows);
    Vec4 a = mid - right * barHalfW + up * y;
    Vec4 b = mid + right * barHalfW + up * y;
    a.w = 1.0F;
    b.w = 1.0F;
    line(a, b, band);
  }

  if (!sheet) {
    center(atBench ? helpMsg : "TRIANGLE ENTERS", blockH * 0.5F - rowPitch * 0.5F);
  } else {
    int row = 0;
    if (flash) {
      center(helpMsg, blockH * 0.5F - rowPitch * (static_cast<float>(row) + 0.5F));
      row++;
    }
    for (int i = 0; i < 4; i++) {
      float y = blockH * 0.5F - rowPitch * (static_cast<float>(row) + 0.5F);
      Vec4 leftPen = mid - right * (barHalfW - 0.15F) + up * y;
      Vec4 rightPen = mid + right * 0.18F + up * y;
      leftPen.w = 1.0F;
      rightPen.w = 1.0F;
      paint(lefts[i], leftPen);
      paint(rights[i], rightPen);
      row++;
    }
    center(waxPage, blockH * 0.5F - rowPitch * (static_cast<float>(row) + 0.5F));
  }

  Vec4 topA = mid - right * barHalfW + up * barHalfH;
  Vec4 topB = mid + right * barHalfW + up * barHalfH;
  Vec4 botA = mid - right * barHalfW - up * barHalfH;
  Vec4 botB = mid + right * barHalfW - up * barHalfH;
  topA.w = 1.0F;
  topB.w = 1.0F;
  botA.w = 1.0F;
  botB.w = 1.0F;
  line(topA, topB, inkc);
  line(botA, botB, inkc);
}

void RacerGame::loop() {
  float dt = 1.0F / 60.0F;
  unsigned int fps = engine->info.getFps();
  if (fps >= 20 && fps <= 120) dt = 1.0F / static_cast<float>(fps);
  glitchPhase++;

  if (!atBench) updateLook(dt);
  tryEnterExit();
  if (inCar) {
    updateInCar(dt);
  } else if (!atBench) {
    updateOnFoot(dt);
  }
  if (!inCar) updateCraft(dt);
  updateCubes(dt);
  updateCamera(dt);

  engine->renderer.beginFrame(CameraInfo3D(&camPos, &camLook));
  engine->renderer.renderer3D.usePipeline(stapip);
  drawWorld();
  drawBench();
  drawCubes();
  drawCar();
  drawPlayer();
  drawHelp(dt);
  engine->renderer.renderer3D.utility.flushLines();
  engine->renderer.endFrame();
}

}  // namespace Racer
