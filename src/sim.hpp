#pragma once
#include <raylib.h>
#include <raymath.h>
#include <vector>

struct Particle {
  Vector2 position{};
  Vector2 velocity{};
  Color color{RED};
  float radius{4.0f};
};

struct Simulation {
  std::vector<Particle> particles{};
};

struct InputState {
  bool is_rmb_pressed{};
  bool is_lmb_pressed{};
  bool should_reset{};
  Vector2 mouse_position{};
};

extern "C" {
Simulation create_simulation();
void draw_simulation(const Simulation &sim);
void update_simulation(Simulation &sim, const InputState &input_state,
                       float dt);
}
