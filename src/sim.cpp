#include "sim.hpp"
#include <array>
#include <random>

const std::size_t AMOUNT{200};
const float GRAVITY{1020.0f};
const float MOUSE_FORCE{5950.0f};
const float MOUSE_RADIUS{400.0f};
const float EDGE_BOUNCE{0.86f};
const float AIR_DRAG{0.995f};

const float MAX_VELOCITY_X{160.0f};
const float MAX_VELOCITY_Y{130.0f};

float random_float(float min, float max) {
  static std::mt19937 generator{std::random_device{}()};
  std::uniform_real_distribution<float> range{min, max};
  return range(generator);
}

extern "C" {
Simulation create_simulation() {
  Simulation sim{};
  sim.particles.reserve(AMOUNT);

  const std::array<Color, 7> palette{RED,       BLUE, GREEN, YELLOW,
                                     LIGHTGRAY, LIME, PINK};

  const float width{static_cast<float>(GetScreenWidth())};
  const float height{static_cast<float>(GetScreenHeight())};

  for (std::size_t i{}; i < AMOUNT; i++) {
    const float radius{random_float(3.0f, 7.0f)};
    const Vector2 position{
        random_float(radius, width - radius),
        random_float(radius, height - radius),
    };
    const Vector2 velocity{
        random_float(-MAX_VELOCITY_X, MAX_VELOCITY_X),
        random_float(-MAX_VELOCITY_Y, MAX_VELOCITY_Y),
    };
    const Color color{palette.at(i % palette.size())};

    sim.particles.push_back(Particle{position, velocity, color, radius});
  }

  return sim;
}

void draw_simulation(const Simulation &sim) {

  for (const auto &particle : sim.particles) {
    DrawCircleV(particle.position, particle.radius, particle.color);
  }
}

void update_simulation(Simulation &sim, const InputState &input_state,
                       float dt) {
  const float width{static_cast<float>(GetScreenWidth())};
  const float height{static_cast<float>(GetScreenHeight())};

  for (auto &particle : sim.particles) {
    particle.velocity.y += GRAVITY * dt;

    if (input_state.is_lmb_pressed || input_state.is_rmb_pressed) {
      Vector2 to_mouse{input_state.mouse_position - particle.position};
      const float distance{Vector2Length(to_mouse)};

      if (distance > 0.001f && distance < MOUSE_RADIUS) {
        const float direction{input_state.is_lmb_pressed ? 1.0f : -1.0f};
        const float falloff{1.0f - distance / MOUSE_RADIUS};

        const Vector2 force{
            to_mouse.x / distance * MOUSE_FORCE * falloff * direction,
            to_mouse.y / distance * MOUSE_FORCE * falloff * direction,
        };

        particle.velocity.x += force.x * dt;
        particle.velocity.y += force.y * dt;
      }
    }

    particle.velocity.x *= std::pow(AIR_DRAG, dt * 60.0f);
    particle.velocity.y *= std::pow(AIR_DRAG, dt * 60.0f);

    particle.position.x += particle.velocity.x * dt;
    particle.position.y += particle.velocity.y * dt;

    if (particle.position.x - particle.radius < 0.0f) {
      particle.position.x = particle.radius;
      particle.velocity.x = std::abs(particle.velocity.x) * EDGE_BOUNCE;
    } else if (particle.position.x + particle.radius > width) {
      particle.position.x = width - particle.radius;
      particle.velocity.x = -std::abs(particle.velocity.x) * EDGE_BOUNCE;
    }

    if (particle.position.y - particle.radius < 0.0f) {
      particle.position.y = particle.radius;
      particle.velocity.y = std::abs(particle.velocity.y) * EDGE_BOUNCE;
    } else if (particle.position.y + particle.radius > height) {
      particle.position.y = height - particle.radius;
      particle.velocity.y = -std::abs(particle.velocity.y) * EDGE_BOUNCE;
    }
  }
}
}
