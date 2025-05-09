#pragma once
#include "thread_pool.h"

namespace Tmpl8 {
// forward declarations
class Tank;
class Rocket;
class Smoke;
class Particle_beam;

class Game {
public:
  // Add a default constructor
  Game() : thread_pool(std::thread::hardware_concurrency()) {}
  
  void set_target(Surface *surface) { screen = surface; }
  void init();
  void shutdown();
  void update(float deltaTime);
  void draw();
  void tick(float deltaTime);
  static void quick_sort_tanks_health(const std::vector<Tank> &original,
                                      std::vector<const Tank *> &sorted_tanks,
                                      int begin, int end);
  void draw_health_bars(const std::vector<const Tank *> &sorted_tanks,
                        const int team);
  void measure_performance();

  Tank &find_closest_enemy(Tank &current_tank);

private:
  Surface *screen;

  ThreadPool thread_pool;
  std::mutex game_mutex;

  vector<Tank> tanks;
  vector<Rocket> rockets;
  vector<Smoke> smokes;
  vector<Explosion> explosions;
  vector<Particle_beam> particle_beams;

  Terrain background_terrain;
  std::vector<vec2> forcefield_hull;

  Font *frame_count_font;
  long long frame_count = 0;

  bool lock_update = false;

  // Checks if a point lies on the left of an arbitrary angled line
  bool left_of_line(vec2 line_start, vec2 line_end, vec2 point);
  void check_tank_collision();
  void update_tanks();
  void find_first_and_most_left_tank(int &first_active, vec2 &point_on_hull);
  void calculate_convex_hull(int first_active, vec2 &point_on_hull);
  void merge_sort_tanks(const vector<Tank *> &tanks);
  void update_rockets();
  void merge_sort_tanks(std::vector<Tank *> &tanks);
  void disable_rockets_when_collide_forcefield();
  void update_particle_beams();
};

}; // namespace Tmpl8
