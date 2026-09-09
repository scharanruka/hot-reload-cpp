#include <dlfcn.h>
#include <exception>
#include <filesystem>
#include <raylib.h>
#include <raymath.h>
#include <stdexcept>

#include "sim.hpp"
namespace fs = std::filesystem;

using CreateSimulationFn = Simulation (*)();
using DrawSimulationFn = void (*)(const Simulation &sim);
using UpdateSimulationFn = void (*)(Simulation &sim,
                                    const InputState &input_state, float dt);

const char *SOURCE_LIBRARY_PATH{"./build/libsim.so"};
const char *LIVE_LIBRARY_DIR{"./build/live/"};

struct SimulationFns {
  CreateSimulationFn create_simulation{};
  DrawSimulationFn draw_simulation{};
  UpdateSimulationFn update_simulation{};
};

struct SimulationLib {
  SimulationFns fns;
  void *handle{};
  fs::path path{};
};

SimulationFns load_fns(void *handle) {
  SimulationFns fns{};
  dlerror();

  fns.create_simulation =
      reinterpret_cast<CreateSimulationFn>(dlsym(handle, "create_simulation"));
  fns.draw_simulation =
      reinterpret_cast<DrawSimulationFn>(dlsym(handle, "draw_simulation"));
  fns.update_simulation =
      reinterpret_cast<UpdateSimulationFn>(dlsym(handle, "update_simulation"));

  if (const char *err{dlerror()}) {
    throw std::runtime_error(err);
  }

  return fns;
};

SimulationLib load_lib() {
  static std::size_t generation{};
  SimulationLib lib{};

  fs::create_directories(LIVE_LIBRARY_DIR);
  fs::path new_lib{fs::path{LIVE_LIBRARY_DIR} /
                   fs::path{std::format("libsim_{}.so", generation++)}};
  fs::copy_file(SOURCE_LIBRARY_PATH, new_lib,
                fs::copy_options::overwrite_existing);

  lib.handle = dlopen(new_lib.c_str(), RTLD_NOW);
  if (!lib.handle) {
    const char *err{dlerror()};
    fs::remove(new_lib);
    fs::remove(LIVE_LIBRARY_DIR);
    throw std::runtime_error(err);
  }

  try {
    lib.fns = load_fns(lib.handle);
    lib.path = new_lib;
  } catch (const std::exception &e) {
    dlclose(lib.handle);
    fs::remove(new_lib);
    fs::remove(LIVE_LIBRARY_DIR);
    throw e;
  }
  return lib;
};

void update_input_state(InputState &input_state) {
  input_state.is_rmb_pressed = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
  input_state.is_lmb_pressed = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
  input_state.should_reset = IsKeyPressed(KEY_R);
  input_state.mouse_position = GetMousePosition();
}

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(1000, 1000, "Particle Simulation");
  SetTargetFPS(60);

  auto active_library{load_lib()};

  Simulation simulation{active_library.fns.create_simulation()};
  InputState input_state{};

  while (!WindowShouldClose()) {
    update_input_state(input_state);

    if (input_state.should_reset) {
      simulation = active_library.fns.create_simulation();
    }

    active_library.fns.update_simulation(simulation, input_state,
                                         GetFrameTime());

    BeginDrawing();
    ClearBackground(BLACK);

    active_library.fns.draw_simulation(simulation);

    EndDrawing();
  }

  fs::remove(active_library.path);
  fs::remove(LIVE_LIBRARY_DIR);
  dlclose(active_library.handle);

  CloseWindow();

  return 0;
}
