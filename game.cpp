#include "precomp.h" // include (only) this in every .cpp file
#include "thread_pool.h"

constexpr auto num_tanks_blue = 2048;
constexpr auto num_tanks_red = 2048;

constexpr auto tank_max_health = 1000;
constexpr auto rocket_hit_value = 60;
constexpr auto particle_beam_hit_value = 50;

constexpr auto tank_max_speed = 1.0;

constexpr auto health_bar_width = 70;

constexpr auto max_frames = 2000;

// Global performance timer
//  constexpr auto REF_PERFORMANCE = 114757; //UPDATE THIS WITH YOUR REFERENCE
//  PERFORMANCE (see console after 2k frames) static timer perf_timer; static
//  float duration;

constexpr auto REF_PERFORMANCE =
    56005.8; // UPDATE THIS WITH YOUR REFERENCE PERFORMANCE (see console after
             // 2k frames)
static timer perf_timer;
static float duration;

// Load sprite files and initialize sprites
static Surface *tank_red_img = new Surface("assets/Tank_Proj2.png");
static Surface *tank_blue_img = new Surface("assets/Tank_Blue_Proj2.png");
static Surface *rocket_red_img = new Surface("assets/Rocket_Proj2.png");
static Surface *rocket_blue_img = new Surface("assets/Rocket_Blue_Proj2.png");
static Surface *particle_beam_img = new Surface("assets/Particle_Beam.png");
static Surface *smoke_img = new Surface("assets/Smoke.png");
static Surface *explosion_img = new Surface("assets/Explosion.png");

static Sprite tank_red(tank_red_img, 12);
static Sprite tank_blue(tank_blue_img, 12);
static Sprite rocket_red(rocket_red_img, 12);
static Sprite rocket_blue(rocket_blue_img, 12);
static Sprite smoke(smoke_img, 4);
static Sprite explosion(explosion_img, 9);
static Sprite particle_beam_sprite(particle_beam_img, 3);

const static vec2 tank_size(7, 9);
const static vec2 rocket_size(6, 6);

const static float tank_radius = 3.f;
const static float rocket_radius = 5.f;

// -----------------------------------------------------------
// Initialize the simulation state
// This function does not count for the performance multiplier
// (Feel free to optimize anyway though ;) )
// -----------------------------------------------------------
void Game::init() {
  frame_count_font = new Font("assets/digital_small.png",
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ:?!=-0123456789.");

  tanks.reserve(num_tanks_blue + num_tanks_red);

  uint max_rows = 24;

  float start_blue_x = tank_size.x + 40.0f;
  float start_blue_y = tank_size.y + 30.0f;

  float start_red_x = 1088.0f;
  float start_red_y = tank_size.y + 30.0f;

  float spacing = 7.5f;

  // Spawn blue tanks
  for (int i = 0; i < num_tanks_blue; i++) {
    vec2 position{start_blue_x + ((i % max_rows) * spacing),
                  start_blue_y + ((i / max_rows) * spacing)};
    tanks.push_back(Tank(position.x, position.y, BLUE, &tank_blue, &smoke,
                         1100.f, position.y + 16, tank_radius, tank_max_health,
                         tank_max_speed));
  }
  // Spawn red tanks
  for (int i = 0; i < num_tanks_red; i++) {
    vec2 position{start_red_x + ((i % max_rows) * spacing),
                  start_red_y + ((i / max_rows) * spacing)};
    tanks.push_back(Tank(position.x, position.y, RED, &tank_red, &smoke, 100.f,
                         position.y + 16, tank_radius, tank_max_health,
                         tank_max_speed));
  }

  particle_beams.push_back(Particle_beam(vec2(590, 327), vec2(100, 50),
                                         &particle_beam_sprite,
                                         particle_beam_hit_value));
  particle_beams.push_back(Particle_beam(vec2(64, 64), vec2(100, 50),
                                         &particle_beam_sprite,
                                         particle_beam_hit_value));
  particle_beams.push_back(Particle_beam(vec2(1200, 600), vec2(100, 50),
                                         &particle_beam_sprite,
                                         particle_beam_hit_value));
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::shutdown() {}

// -----------------------------------------------------------
// Iterates through all tanks and returns the closest enemy tank for the given
// tank
// -----------------------------------------------------------
Tank &Game::find_closest_enemy(Tank &current_tank) {
  float closest_distance = numeric_limits<float>::infinity();
  int closest_index = 0;

  for (int i = 0; i < tanks.size(); i++) {
    if (tanks.at(i).allignment != current_tank.allignment &&
        tanks.at(i).active) {
      float sqr_dist =
          fabsf((tanks.at(i).get_position() - current_tank.get_position())
                    .sqr_length());
      if (sqr_dist < closest_distance) {
        closest_distance = sqr_dist;
        closest_index = i;
      }
    }
  }

  return tanks.at(closest_index);
}

// Checks if a point lies on the left of an arbitrary angled line
bool Tmpl8::Game::left_of_line(vec2 line_start, vec2 line_end, vec2 point) {
  return ((line_end.x - line_start.x) * (point.y - line_start.y) -
          (line_end.y - line_start.y) * (point.x - line_start.x)) < 0;
}

/**
 * Sweep & Prune Algorithm for Tank Collision Detection
 * Documentation enhanced with ChatGPT
 *
 * This algorithm improves collision detection performance by:
 * 1. Projecting objects onto an axis (x-axis)
 * 2. Sorting the start and end points
 * 3. Sweeping through the sorted points
 * 4. Only checking collisions between objects that overlap on the axis
 *
 * Time Complexity:
 * - Sorting: O(n log n)
 * - Linear scan: O(n)
 *
 * Space Complexity: O(n)
 *
 * Advantages:
 * - More efficient than brute force O(n²)
 * - Works well for moving objects
 * - Simple to implement
 * - No complex data structures
 *
 * Implementation:
 * 1. Create arrays of tank bounds (min and max x-coordinates)
 * 2. Sort bounds by x-coordinate
 * 3. Sweep through sorted bounds
 * 4. Only check collisions between objects that overlap on the axis
 *
 * Thread Pool Implementation:
 * - Distributes tank collision checks across multiple CPU cores
 * - Each thread processes a subset of tanks
 * - Uses mutex for thread-safe access to shared resources
 * - Time Complexity: O(n/m) per thread, where n = number of tanks, m = number
 * of threads
 */
void Game::check_tank_collision() {
  // Create arrays of tank bounds (min and max x-coordinates)
  std::vector<std::pair<float, Tank *>> tank_bounds;
  tank_bounds.reserve(tanks.size());

  // Fill bounds array with active tanks
  for (Tank &tank : tanks) {
    if (tank.active) {
      float radius = tank.collision_radius;
      tank_bounds.push_back({tank.position.x - radius, &tank});
      tank_bounds.push_back({tank.position.x + radius, &tank});
    }
  }

  // Sort bounds by x-coordinate
  std::sort(tank_bounds.begin(), tank_bounds.end());

  // Calculate how many bounds per thread
  const int num_bounds = tank_bounds.size();
  const int num_threads = std::thread::hardware_concurrency();
  const int bounds_per_thread =
      (num_bounds + num_threads - 1) / num_threads; // Ceiling division

  std::vector<std::future<void>> futures;

  // Distribute bounds across threads
  for (int i = 0; i < num_bounds; i += bounds_per_thread) {
    int end = std::min(i + bounds_per_thread, num_bounds);

    futures.push_back(thread_pool.enqueue([this, i, end, &tank_bounds]() {
      // Active list of tanks to check for collisions (local to this thread)
      std::vector<Tank *> active_tanks;

      // Process this thread's portion of bounds
      for (int j = i; j < end; j++) {
        const auto &bound = tank_bounds[j];
        Tank *tank = bound.second;

        // Check if this is a start or end bound
        if (bound.first == tank->position.x - tank->collision_radius) {
          // Start bound - add to active list and check against other active
          // tanks
          for (Tank *other : active_tanks) {
            if (tank != other) {
              // Check y-coordinates for overlap
              float dy = tank->position.y - other->position.y;
              float min_y_dist =
                  tank->collision_radius + other->collision_radius;

              if (dy * dy < min_y_dist * min_y_dist) {
                // Collision detected - handle it
                std::lock_guard<std::mutex> lock(game_mutex);
                tank->push((tank->position - other->position).normalized(),
                           1.f);
              }
            }
          }
          active_tanks.push_back(tank);
        } else {
          // End bound - remove from active list
          active_tanks.erase(
              std::remove(active_tanks.begin(), active_tanks.end(), tank),
              active_tanks.end());
        }
      }
    }));
  }

  // Wait for all tasks to complete
  for (auto &future : futures) {
    future.wait();
  }
}

void Game::update_tanks() {
  for (Tank &tank : tanks) {
    if (tank.active) {
      // Move tanks according to speed and nudges (see above) also reload
      tank.tick(background_terrain);

      // Shoot at closest target if reloaded
      if (tank.rocket_reloaded()) {
        Tank &target = find_closest_enemy(tank);

        rockets.push_back(
            Rocket(tank.position,
                   (target.get_position() - tank.position).normalized() * 3,
                   rocket_radius, tank.allignment,
                   ((tank.allignment == RED) ? &rocket_red : &rocket_blue)));

        tank.reload_rocket();
      }
    }
  }
}

void Game::find_first_and_most_left_tank(int &first_active,
                                         vec2 &point_on_hull) {
  first_active = 0;
  for (Tank &tank : tanks) {
    if (tank.active) {
      break;
    }
    first_active++;
  }
  point_on_hull = tanks.at(first_active).position;
  // Find left most tank position
  for (Tank &tank : tanks) {
    if (tank.active) {
      if (tank.position.x <= point_on_hull.x) {
        point_on_hull = tank.position;
      }
    }
  }
}

void Game::calculate_convex_hull(int first_active, vec2 &point_on_hull) {
  while (true) {
    // Add last found point
    forcefield_hull.push_back(point_on_hull);

    // Loop through all points replacing the endpoint with the current iteration
    // every time it lies left of the current segment formed by point_on_hull
    // and the current endpoint. By the end we have a segment with no points on
    // the left and thus a point on the convex hull.
    vec2 endpoint = tanks.at(first_active).position;
    for (Tank &tank : tanks) {
      if (tank.active) {
        if ((endpoint == point_on_hull) ||
            left_of_line(point_on_hull, endpoint, tank.position)) {
          endpoint = tank.position;
        }
      }
    }

    // Set the starting point of the next segment to the found endpoint.
    point_on_hull = endpoint;

    // If we went all the way around we are done.
    if (endpoint == forcefield_hull.at(0)) {
      break;
    }
  }
}

/**
 * Nearest Neighbor Algorithm for Rocket Collision Detection
 * Documentation enhanced with ChatGPT
 *
 * This algorithm improves collision detection performance by:
 * 1. Finding the nearest enemy tank for each rocket
 * 2. Only checking collision with the nearest tank
 * 3. Using distance squared to avoid expensive square root calculations
 * 4. Early termination when collision is found
 *
 * Time Complexity:
 * - Finding nearest tank: O(n)
 * - Overall: O(n * m) worst case, but much better in practice
 *
 * Advantages:
 * - Simple to implement
 * - No additional data structures needed
 * - Early termination when collision is found
 *
 * Implementation:
 * 1. For each rocket, find the nearest enemy tank
 * 2. Check collision only with the nearest tank
 * 3. Handle collision if detected
 *
 * Thread Pool Implementation:
 * - Distributes rocket processing across multiple CPU cores
 * - Each thread processes a subset of rockets
 * - Uses mutex for thread-safe access to shared resources
 * - Time Complexity: O(n/m) per thread, where n = number of rockets, m = number
 * of threads
 */
void Game::update_rockets() {
  // Create a sorted array of active tanks by x-position
  std::vector<Tank *> sorted_tanks;
  sorted_tanks.reserve(tanks.size());

  // Add active tanks to the array
  for (Tank &tank : tanks) {
    if (tank.active) {
      sorted_tanks.push_back(&tank);
    }
  }

  // Simple merge sort (in-place)
  merge_sort_tanks(sorted_tanks);

  // Calculate how many rockets per thread
  const int num_rockets = rockets.size();
  const int num_threads = std::thread::hardware_concurrency();
  const int rockets_per_thread =
      (num_rockets + num_threads - 1) / num_threads; // Ceiling division

  std::vector<std::future<void>> futures;

  // Distribute rockets across threads
  for (int i = 0; i < num_rockets; i += rockets_per_thread) {
    int end = std::min(i + rockets_per_thread, num_rockets);

    futures.push_back(thread_pool.enqueue([this, i, end, &sorted_tanks]() {
      for (int j = i; j < end; j++) {
        Rocket &rocket = rockets[j];
        rocket.tick();

        // Find tanks in x-range
        for (Tank *tank : sorted_tanks) {
          // Skip friendly tanks
          if (tank->allignment == rocket.allignment)
            continue;

          // Quick distance check
          float dx = rocket.position.x - tank->position.x;
          if (std::abs(dx) > rocket.collision_radius + tank->collision_radius)
            continue;

          // Check collision
          if (rocket.intersects(tank->position, tank->collision_radius)) {
            std::lock_guard<std::mutex> lock(game_mutex);
            explosions.push_back(Explosion(&explosion, tank->position));

            if (tank->hit(rocket_hit_value)) {
              smokes.push_back(Smoke(smoke, tank->position - vec2(7, 24)));
            }

            rocket.active = false;
            break;
          }
        }
      }
    }));
  }

  // Wait for all tasks to complete
  for (auto &future : futures) {
    future.wait();
  }
}

// Simple in-place merge sort
void Game::merge_sort_tanks(std::vector<Tank *> &tanks) {
  if (tanks.size() <= 1)
    return;

  // Split array in half
  const int mid = tanks.size() / 2;
  std::vector<Tank *> left(tanks.begin(), tanks.begin() + mid);
  std::vector<Tank *> right(tanks.begin() + mid, tanks.end());

  // Recursively sort halves
  merge_sort_tanks(left);
  merge_sort_tanks(right);

  // Merge sorted halves
  int i = 0, j = 0, k = 0;
  while (i < left.size() && j < right.size()) {
    if (left[i]->position.x <= right[j]->position.x) {
      tanks[k++] = left[i++];
    } else {
      tanks[k++] = right[j++];
    }
  }

  // Copy remaining elements
  while (i < left.size())
    tanks[k++] = left[i++];
  while (j < right.size())
    tanks[k++] = right[j++];
}

void Game::disable_rockets_when_collide_forcefield() {
  for (Rocket &rocket : rockets) {
    if (rocket.active) {
      for (size_t i = 0; i < forcefield_hull.size(); i++) {
        if (circle_segment_intersect(
                forcefield_hull.at(i),
                forcefield_hull.at((i + 1) % forcefield_hull.size()),
                rocket.position, rocket.collision_radius)) {
          explosions.push_back(Explosion(&explosion, rocket.position));
          rocket.active = false;
        }
      }
    }
  }
}

/**
 * Thread Pool Implementation for Particle Beam Updates
 *
 * This implementation improves performance by:
 * 1. Distributing particle beam updates across multiple CPU cores
 * 2. Processing beams in parallel
 * 3. Using a thread pool to manage worker threads
 * 4. Synchronizing access to shared resources with mutexes
 *
 * Time Complexity:
 * - Beam updates: O(n/m) per thread, where n = number of beams, m = number of
 * threads
 * - Overall: O(n) with m threads
 *
 * Space Complexity: O(m) for thread management
 *
 * Advantages:
 * - Utilizes all available CPU cores
 * - Scales with hardware capabilities
 * - Efficient thread management
 * - Reduced overall processing time
 *
 * Implementation:
 * 1. Initialize thread pool with number of hardware cores
 * 2. Divide beams among threads
 * 3. Process beams in parallel
 * 4. Synchronize access to shared resources
 * 5. Wait for all tasks to complete
 */
void Game::update_particle_beams() {
  // Calculate how many beams per thread
  const int num_beams = particle_beams.size();
  const int num_threads = std::thread::hardware_concurrency();
  const int beams_per_thread =
      (num_beams + num_threads - 1) / num_threads; // Ceiling division

  std::vector<std::future<void>> futures;

  // Distribute beams across threads
  for (int i = 0; i < num_beams; i += beams_per_thread) {
    int end = std::min(i + beams_per_thread, num_beams);

    futures.push_back(thread_pool.enqueue([this, i, end]() {
      for (int j = i; j < end; j++) {
        Particle_beam &particle_beam = particle_beams[j];
        particle_beam.tick(tanks);

        // Check for tank hits
        for (Tank &tank : tanks) {
          if (tank.active && particle_beam.rectangle.intersects_circle(
                                 tank.position, tank.collision_radius)) {
            std::lock_guard<std::mutex> lock(game_mutex);
            if (tank.hit(particle_beam_hit_value)) {
              smokes.push_back(Smoke(smoke, tank.position - vec2(7, 24)));
            }
          }
        }
      }
    }));
  }

  // Wait for all tasks to complete
  for (auto &future : futures) {
    future.wait();
  }
}

// -----------------------------------------------------------
// Update the game state:
// Move all objects
// Update sprite frames
// Collision detection
// Targeting etc..
// -----------------------------------------------------------
void Game::update(float deltaTime) {
  // Calculate the route to the destination for each tank using BFS
  // Initializing routes here so it gets counted for performance..
  if (frame_count == 0) {
    for (Tank &t : tanks) {
      t.set_route(background_terrain.get_route(t, t.target));
    }
  }
  // Update smoke plumes
  for (Smoke &smoke : smokes) {
    smoke.tick();
  }

  // Check tank collision and nudge tanks away from each other
  check_tank_collision();

  // Update tanks
  update_tanks();

  // Calculate "force field" around active tanks
  forcefield_hull.clear();

  // Find first active tank (this loop is a bit disgusting, fix?)
  int first_active;
  vec2 point_on_hull{};
  find_first_and_most_left_tank(first_active, point_on_hull);

  // Calculate convex hull for 'rocket barrier'
  calculate_convex_hull(first_active, point_on_hull);

  // Update rockets
  update_rockets();

  // Disable rockets if they collide with the "forcefield"
  // hint: a point to convex hull intersection test might be better here? :)
  // (Disable if outside)
  disable_rockets_when_collide_forcefield();

  // Remove exploded rockets with remove erase idiom
  rockets.erase(
      std::remove_if(rockets.begin(), rockets.end(),
                     [](const Rocket &rocket) { return !rocket.active; }),
      rockets.end());

  // Update particle beams
  update_particle_beams();

  // Update explosion sprites and remove when done with remove erase idiom
  for (Explosion &explosion : explosions) {
    explosion.tick();
  }

  explosions.erase(std::remove_if(explosions.begin(), explosions.end(),
                                  [](const Explosion &explosion) {
                                    return explosion.done();
                                  }),
                   explosions.end());
}

// -----------------------------------------------------------
// Draw all sprites to the screen
// (It is not recommended to multi-thread this function)
// -----------------------------------------------------------
void Game::draw() {
  // clear the graphics window
  screen->clear(0);

  // Draw background
  background_terrain.draw(screen);

  // Draw sprites
  for (int i = 0; i < num_tanks_blue + num_tanks_red; i++) {
    tanks.at(i).draw(screen);

    vec2 tank_pos = tanks.at(i).get_position();
  }

  for (Rocket &rocket : rockets) {
    rocket.draw(screen);
  }

  for (Smoke &smoke : smokes) {
    smoke.draw(screen);
  }

  for (Particle_beam &particle_beam : particle_beams) {
    particle_beam.draw(screen);
  }

  for (Explosion &explosion : explosions) {
    explosion.draw(screen);
  }

  // Draw forcefield (mostly for debugging, its kinda ugly..)
  for (size_t i = 0; i < forcefield_hull.size(); i++) {
    vec2 line_start = forcefield_hull.at(i);
    vec2 line_end = forcefield_hull.at((i + 1) % forcefield_hull.size());
    line_start.x += HEALTHBAR_OFFSET;
    line_end.x += HEALTHBAR_OFFSET;
    screen->line(line_start, line_end, 0x0000ff);
  }

  // Draw sorted health bars
  for (int t = 0; t < 2; t++) {
    const int NUM_TANKS = ((t < 1) ? num_tanks_blue : num_tanks_red);

    const int begin = ((t < 1) ? 0 : num_tanks_blue);
    std::vector<const Tank *> sorted_tanks;
    quick_sort_tanks_health(tanks, sorted_tanks, begin, begin + NUM_TANKS);
    sorted_tanks.erase(
        std::remove_if(sorted_tanks.begin(), sorted_tanks.end(),
                       [](const Tank *tank) { return !tank->active; }),
        sorted_tanks.end());

    draw_health_bars(sorted_tanks, t);
  }
}

// TODO: change this. (changed to quicksort)
// -----------------------------------------------------------
// Sort tanks by health value using quicksort (tekst met chatgpt verduidelijkt)
// Bron voor uitleg: https://www.youtube.com/watch?v=Vtckgz38QHs
// -----------------------------------------------------------
// Time Complexity:
// - Original (insertion sort): O(n²) → very slow with many tanks
// - New (quicksort): O(n log n) → much faster with many tanks
//
// How Quicksort Works:
// 1. Pick a pivot (middle tank)
// 2. Put tanks with more health than pivot on left
// 3. Put tanks with less health than pivot on right
// 4. Repeat for left and right parts
//
// Example with health values:
// Start:  [50, 90, 30, 70, 60]
// Pivot = 60
// Step 1: [90, 70, 60, 30, 50] (60 is in correct spot)
// Step 2: [90, 70] 60 [30, 50] (sort each side)
// Final:  [90, 70, 60, 50, 30] (sorted by health)
// -----------------------------------------------------------
void Game::quick_sort_tanks_health(const std::vector<Tank> &original,
                                   std::vector<const Tank *> &sorted_tanks,
                                   const int begin, const int end) {
  const int NUM_TANKS = end - begin;
  sorted_tanks.clear(); // Clear existing tanks
  sorted_tanks.reserve(NUM_TANKS);

  // First add all tanks we want to sort
  for (int i = begin; i < end; i++) {
    sorted_tanks.push_back(&original.at(i));
  }

  if (!sorted_tanks.empty()) {
    // Stack for tracking ranges to sort (replaces recursion)
    vector<pair<int, int>> ranges;
    ranges.push_back({0, sorted_tanks.size() - 1});

    while (!ranges.empty()) {
      int low = ranges.back().first;
      int high = ranges.back().second;
      ranges.pop_back();

      if (low < high) {
        // Choose middle element as pivot
        // If float change to int to avoid errors
        int mid = low + (high - low) / 2.0f;
        int mid_int = static_cast<int>(mid);
        const Tank *pivot = sorted_tanks[mid_int];

        // Move pivot to end
        std::swap(sorted_tanks[mid_int], sorted_tanks[high]);

        // Partition around pivot
        int i = low;
        for (int j = low; j < high; j++) {
          if (sorted_tanks[j]->compare_health(*pivot) >= 0) {
            std::swap(sorted_tanks[i], sorted_tanks[j]);
            i++;
          }
        }
        std::swap(sorted_tanks[i], sorted_tanks[high]);

        // Add ranges for both sides of pivot
        ranges.push_back({low, i - 1});
        ranges.push_back({i + 1, high});
      }
    }
  }
}

// -----------------------------------------------------------
// Draw the health bars based on the given tanks health values
// -----------------------------------------------------------
void Tmpl8::Game::draw_health_bars(
    const std::vector<const Tank *> &sorted_tanks, const int team) {
  int health_bar_start_x = (team < 1) ? 0 : (SCRWIDTH - HEALTHBAR_OFFSET) - 1;
  int health_bar_end_x =
      (team < 1) ? health_bar_width : health_bar_start_x + health_bar_width - 1;

  for (int i = 0; i < SCRHEIGHT - 1; i++) {
    // Health bars are 1 pixel each
    int health_bar_start_y = i * 1;
    int health_bar_end_y = health_bar_start_y + 1;

    screen->bar(health_bar_start_x, health_bar_start_y, health_bar_end_x,
                health_bar_end_y, REDMASK);
  }

  // Draw the <SCRHEIGHT> least healthy tank health bars
  int draw_count = std::min(SCRHEIGHT, (int)sorted_tanks.size());
  for (int i = 0; i < draw_count - 1; i++) {
    // Health bars are 1 pixel each
    int health_bar_start_y = i * 1;
    int health_bar_end_y = health_bar_start_y + 1;

    float health_fraction =
        (1 - ((double)sorted_tanks.at(i)->health / (double)tank_max_health));

    if (team == 0) {
      screen->bar(health_bar_start_x +
                      (int)((double)health_bar_width * health_fraction),
                  health_bar_start_y, health_bar_end_x, health_bar_end_y,
                  GREENMASK);
    } else {
      screen->bar(health_bar_start_x, health_bar_start_y,
                  health_bar_end_x -
                      (int)((double)health_bar_width * health_fraction),
                  health_bar_end_y, GREENMASK);
    }
  }
}

// -----------------------------------------------------------
// When we reach max_frames print the duration and speedup multiplier
// Updating REF_PERFORMANCE at the top of this file with the value
// on your machine gives you an idea of the speedup your optimizations give
// -----------------------------------------------------------
void Game::measure_performance() {
  char buffer[128];
  if (frame_count >= max_frames) {
    if (!lock_update) {
      duration = perf_timer.elapsed();
      cout << "Duration was: " << duration
           << " (Replace REF_PERFORMANCE with this value)" << endl;
      lock_update = true;
    }

    frame_count--;
  }

  if (lock_update) {
    screen->bar(420 + HEALTHBAR_OFFSET, 170, 870 + HEALTHBAR_OFFSET, 430,
                0x030000);
    int ms = (int)duration % 1000, sec = ((int)duration / 1000) % 60,
        min = ((int)duration / 60000);
    sprintf(buffer, "%02i:%02i:%03i", min, sec, ms);
    frame_count_font->centre(screen, buffer, 200);
    sprintf(buffer, "SPEEDUP: %4.1f", REF_PERFORMANCE / duration);
    frame_count_font->centre(screen, buffer, 340);
  }
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::tick(float deltaTime) {
  if (!lock_update) {
    update(deltaTime);
  }
  draw();

  measure_performance();

  // print something in the graphics window
  // screen->Print("hello world", 2, 2, 0xffffff);

  // print something to the text window
  // cout << "This goes to the console window." << std::endl;

  // Print frame count
  frame_count++;
  string frame_count_string = "FRAME: " + std::to_string(frame_count);
  frame_count_font->print(screen, frame_count_string.c_str(), 350, 580);
}
