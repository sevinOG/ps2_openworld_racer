#pragma once

#include <tyra>

namespace Racer {

struct Cube {
  Tyra::Vec4 pos;  // bottom-center (matches drawBoxYaw)
  Tyra::Vec4 vel;
  float size;  // half-extent
};

// Swappable buggy parts. Stock values live in equipStockParts(); a later
// shop can replace any corner's shock or tire without touching the solver.
enum TireCompound { TireSoft = 0, TireMedium = 1, TireHard = 2 };

struct ShockSpec {
  float spring;   // force per unit of compression
  float bump;     // damping while the shock is compressing
  float rebound;  // damping while the shock is extending
  float travel;   // max compression
};

struct TireSpec {
  float radius;
  float pressure;       // psi
  float ratedPressure;  // psi where this compound grips the most
  float grip;           // friction coefficient of the compound
  float sidewall;       // tire spring rate at rated pressure
  TireCompound compound;
};

struct CornerPart {
  ShockSpec shock;
  TireSpec tire;
};

// Drawn on the wax bench, then copied into inventory. Shocks are not
// drawable; those get found later and installed as real components.
enum PartKind { PartNone = 0, PartLowerArm = 1, PartUpperArm = 2 };

struct CraftPart {
  PartKind kind;
  float tweak;  // arm spread, modified on the bench
};

struct MountedPart {
  CraftPart part;
  int socket;  // 0-3 lower corners, 4-7 upper corners
  bool welded;
};

class RacerGame : public Tyra::Game {
 public:
  explicit RacerGame(Tyra::Engine* engine);
  ~RacerGame() {}

  void init() override;
  void loop() override;

 private:
  static const int kCubeCount = 7;
  static const int kInvCap = 8;
  static const int kSocketCount = 8;

  Tyra::Engine* engine;
  Tyra::StaticPipeline stapip;

  bool inCar;
  bool atBench = false;
  float playerYaw;
  float carYaw;
  float carPitch = 0.0F;
  float carRoll = 0.0F;
  float carVelY = 0.0F;
  float pitchVel = 0.0F;
  float rollVel = 0.0F;
  float wheelSpin = 0.0F;
  float steerAngle = 0.0F;
  float carSpeed;
  float camYaw;
  float camPitch;
  float playerVelY = 0.0F;
  float yawRate = 0.0F;
  bool lookHeld = false;
  bool lookWasHeld = false;
  bool lookReturning = false;
  Tyra::Vec4 playerPos;
  Tyra::Vec4 carPos;
  Tyra::Vec4 camPos;
  Tyra::Vec4 camLook;
  // 0 rear-left, 1 rear-right, 2 front-left, 3 front-right (+z is forward)
  CornerPart corners[4];
  Cube cubes[kCubeCount];

  float wax = 28.0F;
  int recipe = 0;
  int glitchPhase = 0;
  float deny = 0.0F;
  float helpHold = 0.0F;
  bool helpVisible = true;
  char helpMsg[32] = {};
  float cursorX = 0.0F;
  float cursorZ = 0.0F;
  float benchYaw = 0.0F;
  float benchPitch = 1.05F;
  float draftTweak = 1.0F;
  unsigned int traceBits = 0;
  bool traceLatched = false;
  bool tableHas = false;
  CraftPart tablePart{};
  CraftPart inv[kInvCap]{};
  int invCount = 0;
  MountedPart mounts[kSocketCount]{};
  int mountCount = 0;
  Tyra::Vec4 benchPos;

  float analog(unsigned char raw) const;
  void equipStockParts();
  float tireGrip(const TireSpec& tire) const;
  float cornerSpring(const CornerPart& corner) const;
  void clampToWorld(Tyra::Vec4* p) const;
  void updateLook(float dt);
  void updateOnFoot(float dt);
  void updateInCar(float dt);
  void tryEnterExit();
  void updateCamera(float dt);
  void initCubes();
  void updateCubes(float dt);
  void pushCubesFromCircle(float cx, float cz, float radius, float vx,
                           float vz, float strength);
  void updateCraft(float dt);
  bool nearBench() const;
  bool nearCarCraft() const;
  bool socketTaken(int socket) const;
  void drawWorld();
  void drawPlayer();
  void drawCar();
  void drawCubes();
  void drawBench();
  void drawHelp(float dt);
  void say(const char* text);
  void armGuide(Tyra::Vec4* a, Tyra::Vec4* b, Tyra::Vec4* tip) const;
  void drawWaxPips(const Tyra::Vec4& head, const Tyra::Vec4& right,
                   const Tyra::Vec4& up);
  void drawArmShape(const Tyra::Vec4& origin, const Tyra::Vec4& right,
                    const Tyra::Vec4& up, const Tyra::Vec4& fwd, float x0,
                    float x1, float y0, float y1, float spread,
                    const Tyra::Color& color, bool glow);
  void drawMountedArms(const Tyra::Vec4& origin, const Tyra::Vec4& right,
                       const Tyra::Vec4& up, const Tyra::Vec4& fwd);
  void ink(const Tyra::Vec4& a, const Tyra::Vec4& b, const Tyra::Color& c);
  void carBasis(Tyra::Vec4* fwd, Tyra::Vec4* right, Tyra::Vec4* up) const;
  void drawBoxLocal(const Tyra::Vec4& origin, const Tyra::Vec4& right,
                    const Tyra::Vec4& up, const Tyra::Vec4& fwd, float hx,
                    float hy, float hz, const Tyra::Color& color);
  void drawWheel(const Tyra::Vec4& center, const Tyra::Vec4& axle, float spin,
                 float radius, const Tyra::Color& rubber, const Tyra::Color& hub);
  void drawPenguin(const Tyra::Vec4& feet, const Tyra::Vec4& fwd,
                   const Tyra::Vec4& right, const Tyra::Vec4& up);
  void drawBoxYaw(const Tyra::Vec4& center, float yaw, float hx, float hy,
                  float hz, const Tyra::Color& color);
  void line(const Tyra::Vec4& a, const Tyra::Vec4& b, const Tyra::Color& c);
  Tyra::Vec4 rotateYaw(const Tyra::Vec4& local, float yaw,
                       const Tyra::Vec4& origin) const;
};

}  // namespace Racer
