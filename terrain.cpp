#include "precomp.h"
#include "terrain.h"

namespace fs = std::filesystem;
namespace Tmpl8 {

using TileMap = vector<vector<TerrainTile>>;
using TileDistanceMap = unordered_map<TerrainTile *, vec2>;

Terrain::Terrain() {
  // Load in terrain sprites
  grass_img = std::make_unique<Surface>("assets/tile_grass.png");
  forest_img = std::make_unique<Surface>("assets/tile_forest.png");
  rocks_img = std::make_unique<Surface>("assets/tile_rocks.png");
  mountains_img = std::make_unique<Surface>("assets/tile_mountains.png");
  water_img = std::make_unique<Surface>("assets/tile_water.png");

  tile_grass = std::make_unique<Sprite>(grass_img.get(), 1);
  tile_forest = std::make_unique<Sprite>(forest_img.get(), 1);
  tile_rocks = std::make_unique<Sprite>(rocks_img.get(), 1);
  tile_water = std::make_unique<Sprite>(water_img.get(), 1);
  tile_mountains = std::make_unique<Sprite>(mountains_img.get(), 1);

  // Load terrain layout file and fill grid based on tiletypes
  fs::path terrain_file_path{"assets/terrain.txt"};
  std::ifstream terrain_file(terrain_file_path);

  if (terrain_file.is_open()) {
    std::string terrain_line;

    std::getline(terrain_file, terrain_line);
    std::istringstream lineStream(terrain_line);

    int rows;

    lineStream >> rows;

    for (size_t row = 0; row < rows; row++) {
      std::getline(terrain_file, terrain_line);

      for (size_t collumn = 0; collumn < terrain_line.size(); collumn++) {
        switch (std::toupper(terrain_line.at(collumn))) {
        case 'G':
          tiles.at(row).at(collumn).tile_type = TileType::GRASS;
          break;
        case 'F':
          tiles.at(row).at(collumn).tile_type = TileType::FORREST;
          break;
        case 'R':
          tiles.at(row).at(collumn).tile_type = TileType::ROCKS;
          break;
        case 'M':
          tiles.at(row).at(collumn).tile_type = TileType::MOUNTAINS;
          break;
        case 'W':
          tiles.at(row).at(collumn).tile_type = TileType::WATER;
          break;
        default:
          tiles.at(row).at(collumn).tile_type = TileType::GRASS;
          break;
        }
      }
    }
  } else {
    std::cout << "Could not open terrain file! Is the path correct? Defaulting "
                 "to grass.."
              << std::endl;
    std::cout << "Path was: " << terrain_file_path << std::endl;
  }

  // Instantiate tiles for path planning
  for (size_t y = 0; y < tiles.size(); y++) {
    for (size_t x = 0; x < tiles.at(y).size(); x++) {
      tiles.at(y).at(x).position_x = x;
      tiles.at(y).at(x).position_y = y;

      if (is_accessible(y, x + 1)) {
        tiles.at(y).at(x).exits.push_back(&tiles.at(y).at(x + 1));
      }
      if (is_accessible(y, x - 1)) {
        tiles.at(y).at(x).exits.push_back(&tiles.at(y).at(x - 1));
      }
      if (is_accessible(y + 1, x)) {
        tiles.at(y).at(x).exits.push_back(&tiles.at(y + 1).at(x));
      }
      if (is_accessible(y - 1, x)) {
        tiles.at(y).at(x).exits.push_back(&tiles.at(y - 1).at(x));
      }
    }
  }
}

void Terrain::update() {
  // Pretend there is animation code here.. next year :)
}

void Terrain::draw(Surface *target) const {

  for (size_t y = 0; y < tiles.size(); y++) {
    for (size_t x = 0; x < tiles.at(y).size(); x++) {
      int posX = (x * sprite_size) + HEALTHBAR_OFFSET;
      int posY = y * sprite_size;

      switch (tiles.at(y).at(x).tile_type) {
      case TileType::GRASS:
        tile_grass->draw(target, posX, posY);
        break;
      case TileType::FORREST:
        tile_forest->draw(target, posX, posY);
        break;
      case TileType::ROCKS:
        tile_rocks->draw(target, posX, posY);
        break;
      case TileType::MOUNTAINS:
        tile_mountains->draw(target, posX, posY);
        break;
      case TileType::WATER:
        tile_water->draw(target, posX, posY);
        break;
      default:
        tile_grass->draw(target, posX, posY);
        break;
      }
    }
  }
}

/*
// 3 algorithmen:
// 1. BFS: snelheid 1.0
// 2. Dijkstra: snelheid 1.0
// 3. A*: snelheid 1.1
// De BFS was al gegeven en heb de dijkstra en a* toegevoegd.
// De A* bleek het snelste te zijn.
// 5 Release runs elk.
*/

// Resultaat: sneller dan BFS (speedup 1.1 vs 1.0 origineel)
// A* algorithm to find shortest route to the destination
// BIG O: O(n^2)
// vector<vec2> Terrain::get_route(const Tank &tank, const vec2 &target) {
//     // Convert positions to tile coordinates
//     const size_t start_x = tank.position.x / sprite_size;
//     const size_t start_y = tank.position.y / sprite_size;
//     const size_t target_x = target.x / sprite_size;
//     const size_t target_y = target.y / sprite_size;
//
//     TerrainTile *start_tile = &tiles.at(start_y).at(start_x);
//     TerrainTile *target_tile = &tiles.at(target_y).at(target_x);
//
//     // Define maps and priority queue
//     std::unordered_map<TerrainTile*, TerrainTile*> parent_map;
//     std::unordered_map<TerrainTile*, float> g_score;  // Cost from start to current
//     std::unordered_map<TerrainTile*, float> f_score;  // Estimated total cost (g_score + heuristic)
//
//     // Priority queue using f_score
//     std::priority_queue<std::pair<float, TerrainTile*>,
//                        std::vector<std::pair<float, TerrainTile*>>,
//                        std::greater<std::pair<float, TerrainTile*>>> open_set;
//
//     // Heuristic function (Euclidean distance)
//     auto heuristic = [target_tile](TerrainTile* tile) {
//         float dx = tile->position_x - target_tile->position_x;
//         float dy = tile->position_y - target_tile->position_y;
//         return std::sqrt(dx*dx + dy*dy);
//     };
//
//     // Initialize scores
//     for (auto &row : tiles) {
//         for (TerrainTile &tile : row) {
//             g_score[&tile] = std::numeric_limits<float>::max();
//             f_score[&tile] = std::numeric_limits<float>::max();
//         }
//     }
//
//     g_score[start_tile] = 0;
//     f_score[start_tile] = heuristic(start_tile);
//     open_set.push({f_score[start_tile], start_tile});
//
//     while (!open_set.empty()) {
//         auto [current_f, current_tile] = open_set.top();
//         open_set.pop();
//
//         if (current_tile == target_tile) break;
//
//         for (TerrainTile *neighbor : current_tile->exits) {
//             // Tentative g_score is the distance from start to the neighbor through current
//             float tentative_g_score = g_score[current_tile] + neighbor->cost;
//
//             if (tentative_g_score < g_score[neighbor]) {
//                 parent_map[neighbor] = current_tile;
//                 g_score[neighbor] = tentative_g_score;
//                 f_score[neighbor] = g_score[neighbor] + heuristic(neighbor);
//
//                 // Add to open set if not already there
//                 open_set.push({f_score[neighbor], neighbor});
//             }
//         }
//     }
//
//     // Reconstruct path
//     vector<vec2> route;
//     TerrainTile *tile = target_tile;
//
//     if (g_score[target_tile] != std::numeric_limits<float>::max()) {
//         while (tile != nullptr) {
//             route.push_back(vec2((float)tile->position_x * sprite_size,
//                                (float)tile->position_y * sprite_size));
//             tile = parent_map[tile];
//         }
//         std::reverse(route.begin(), route.end());
//     }
//
//     return route;
// }

// Resultaat: Zelfde snelheid als BFS (speedup 1.0 vs 1.0 origineel)
// // Use Dijkstra's algorithm to find shortest route to the destination
// // BIG O: O(n^2)
// vector<vec2> Terrain::get_route(const Tank &tank, const vec2 &target) {
//   // Convert positions to tile coordinates
//   const size_t start_x = tank.position.x / sprite_size;
//   const size_t start_y = tank.position.y / sprite_size;
//   const size_t target_x = target.x / sprite_size;
//   const size_t target_y = target.y / sprite_size;

//   TerrainTile *start_tile = &tiles.at(start_y).at(start_x);
//   TerrainTile *target_tile = &tiles.at(target_y).at(target_x);

//   // Define maps and priority queue
//   std::unordered_map<TerrainTile *, TerrainTile *> parent_map;
//   std::unordered_map<TerrainTile *, int> distance;
//   std::priority_queue<std::pair<int, TerrainTile *>,
//                       std::vector<std::pair<int, TerrainTile *>>,
//                       std::greater<std::pair<int, TerrainTile *>>>
//       pq;

//   // Initialize distances
//   for (auto &row : tiles) {
//     for (TerrainTile &tile : row) {
//       distance[&tile] = std::numeric_limits<int>::max();
//     }
//   }

//   distance[start_tile] = 0;
//   pq.push({0, start_tile});

//   while (!pq.empty()) {
//     auto [current_cost, current_tile] = pq.top();
//     pq.pop();

//     if (current_tile == target_tile)
//       break;

//     for (TerrainTile *neighbor : current_tile->exits) {
//       int new_cost = current_cost + neighbor->cost;
//       if (new_cost < distance[neighbor]) {
//         distance[neighbor] = new_cost;
//         parent_map[neighbor] = current_tile;
//         pq.push({new_cost, neighbor});
//       }
//     }
//   }

//   vector<vec2> route;
//   TerrainTile *tile = target_tile;

//   if (distance[target_tile] != std::numeric_limits<int>::max()) {
//     while (tile != nullptr) {
//       route.push_back(vec2((float)tile->position_x * sprite_size,
//                          (float)tile->position_y * sprite_size));
//       tile = parent_map[tile];
//     }
//     std::reverse(route.begin(), route.end());
//   }

//   return route;
// }

// Use Breadth-first search to find shortest route to the destination
vector<vec2> Terrain::get_route(const Tank &tank, const vec2 &target) {
  // Find start and target tile
  const size_t pos_x = tank.position.x / sprite_size;
  const size_t pos_y = tank.position.y / sprite_size;

  const size_t target_x = target.x / sprite_size;
  const size_t target_y = target.y / sprite_size;

  // Init queue with start tile
  std::queue<vector<TerrainTile *>> queue;
  queue.emplace();
  queue.back().push_back(&tiles.at(pos_y).at(pos_x));

  std::vector<TerrainTile *> visited;

  bool route_found = false;
  vector<TerrainTile *> current_route;
  while (!queue.empty() && !route_found) {
    current_route = queue.front();
    queue.pop();
    TerrainTile *current_tile = current_route.back();

    // Check all exits, if target then done, else if unvisited push a new
    // partial route
    for (TerrainTile *exit : current_tile->exits) {
      if (exit->position_x == target_x && exit->position_y == target_y) {
        current_route.push_back(exit);
        route_found = true;
        break;
      } else if (!exit->visited) {
        exit->visited = true;
        visited.push_back(exit);
        queue.push(current_route);
        queue.back().push_back(exit);
      }
    }
  }

  // Reset tiles
  for (TerrainTile *tile : visited) {
    tile->visited = false;
  }

  if (route_found) {
    // Convert route to vec2 to prevent dangling pointers
    std::vector<vec2> route;
    for (TerrainTile *tile : current_route) {
      route.push_back(vec2((float)tile->position_x * sprite_size,
                           (float)tile->position_y * sprite_size));
    }

    return route;
  }
  return std::vector<vec2>();

}

// TODO: See if I can delete this if not used ?? (Berend)
// TODO: Function not used, convert BFS to dijkstra and take speed into account
// next year :)
// float Terrain::get_speed_modifier(const vec2 &position) const {
//   const size_t pos_x = position.x / sprite_size;
//   const size_t pos_y = position.y / sprite_size;

//   switch (tiles.at(pos_y).at(pos_x).tile_type) {
//   case TileType::GRASS:
//     return 1.0f;
//     break;
//   case TileType::FORREST:
//     return 0.5f;
//     break;
//   case TileType::ROCKS:
//     return 0.75f;
//     break;
//   case TileType::MOUNTAINS:
//     return 0.0f;
//     break;
//   case TileType::WATER:
//     return 0.0f;
//     break;
//   default:
//     return 1.0f;
//     break;
//   }
// }

bool Terrain::is_accessible(int y, int x) {
  // Bounds check
  if ((x >= 0 && x < terrain_width) && (y >= 0 && y < terrain_height)) {
    // Inaccessible terrain check
    if (tiles.at(y).at(x).tile_type != TileType::MOUNTAINS &&
        tiles.at(y).at(x).tile_type != TileType::WATER) {
      return true;
    }
  }

  return false;
}
} // namespace Tmpl8
