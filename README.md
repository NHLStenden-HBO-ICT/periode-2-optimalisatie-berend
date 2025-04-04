# Optimalisatie_Project

<!--toc:start-->
- [Optimalisatie_Project](#optimalisatie_project)
  - [Inleiding](#inleiding)
  - [Bottlenecks](#bottlenecks)
  - [Big-O](#big-o)
    - [Eerste big-o](#eerste-big-o)
  - [Algoritmes](#algoritmes)
    - [Eerste algoritme](#eerste-algoritme)
  - [Threading](#threading)
  - [Resultaat](#resultaat)
  - [Bronnen](#bronnen)
  - [Build and run van het project](#build-and-run-van-het-project)
<!--toc:end-->

## Inleiding

Met dit document wordt duidelijk gemaakt welke algoritmes er zijn gebruikt en waarom.  
Ook wordt er toegelicht waar en waarom threading is toegepast — er wordt gebruikgemaakt van de meegeleverde threadpool.  
Aan het einde komt het resultaat te zien van deze twee verbeteringen.

Vooraf wordt er nog kort toegelicht welke Big-O er speelt binnen de code (zoals vereist in de rubric), al heeft dit geen invloed op de score.  
Ook wordt er via de CLion-profiler gekeken naar bottlenecks (de tijd die het kost om een bepaalde actie uit te voeren).

## Bottlenecks

Met profiling is gezien dat er een paar functies zijn die veel tijd in beslag nemen.  
Deze kwamen in de volgende volgorde in beeld:

- `Game::tick`
- `Game::update`
- `Game::update_rockets`
- `Game::draw`
- `Game::insertion_sort_tanks_health`
- `Game::draw`
- `Rocket::intersects`
- `Tank::compare_health`

Mijn focus ligt op algoritmes, dus ik heb eerst gefocust op `insertion_sort_tanks_health`.

## Big-O

### Eerste big-o

- **`insertion_sort_tanks_health`**
  - Was: `O(n^2)`
  - Nu: `O(n log n)` (Quicksort gekozen)

- **`check_tank_collision`**
  - Was: `O(n^2)`
  - Nu: `O(n log n)` (Sweep and prune gekozen — zag ik op YouTube, ging van 1,5 uur naar 3 minuten)

- **`update_rockets`**
  - Was: `O(n^2)`
  - Nu: `O(n log n)` (Merge sort — zelfde Big-O maar stopt eerder wanneer mogelijk)

## Algoritmes

### Eerste algoritme

Ik heb de `insertion_sort_tanks_health` aangepast naar een quicksort-algoritme.  
Dit heb ik verwerkt in `game.cpp`.

**Tweede algoritme:** *Sweep and prune*  
Deze gaf direct verbetering van score: **1.1 → 2.0**

**Derde algoritme:** *Merge sort*  
Gaf weinig tot geen verbetering qua score, maar de originele werd trager bij grotere datasets.  
Merge sort stopt eerder en geeft op termijn snellere code.

## Threading

Ik heb de meegeleverde threadpool gebruikt.  
Het aantal hardware threads wordt opgehaald en vervolgens gebruik ik multithreading bij de functies waarbij ik een algoritme heb aangepast in `game.cpp`.

## Resultaat

Resultaat van deze optimalisaties:

**Score:** `1.0` → `3.3`

## Bronnen

- [Dijkstra Algorithm - GeeksforGeeks](https://www.geeksforgeeks.org/introduction-to-dijkstras-shortest-path-algorithm/)
- [Learn Quick Sort in 13 minutes ⚡](https://www.youtube.com/watch?v=Vtckgz38QHs)
- [Building Collision Simulation (YT)](https://www.youtube.com/watch?v=eED4bSkYCB8&t=942s)
- [Learn Merge Sort in 13 minutes 🔪](https://www.youtube.com/watch?v=3j0SWDX4AtU)

## Build and run van het project

Gebruik:

- `Neovim`
- `CMakeBuild`
  - CMakeBuildType → `Debug` of `Release`
- CMakeRun

- Jetbrain kan ook.
- Die komt direct met pop-up wat je wild Debug is default in dropdown kan je soort kiezen (debug of release heb ik gebruikt)
