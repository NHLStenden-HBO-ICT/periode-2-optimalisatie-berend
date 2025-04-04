# Optimalisatie_Project

<!--toc:start-->
- [Optimalisatie_Project](#optimalisatieproject)
  - [Inleiding](#inleiding)
  - [Bottlenecks](#bottlenecks)
  - [Big-O](#big-o)
    - [Eerste big-o](#eerste-big-o)
  - [Algoritmes](#algoritmes)
    - [Eerste algoritme](#eerste-algoritme)
  - [Threading](#threading)
  - [Resultaat](#resultaat)
  - [Bronnen](#bronnen)
<!--toc:end-->

## Inleiding

Met dit document wordt duidelijk gemaakt welke algoritmes er zijn gebruikt en waarom.
Ook wordt er toegelicht waar en waarom threading is toegepast, er wordt gebruikt gemaakt van de meegeleverde threadpool.
Aan het einde komt er resultaat te zien van deze twee verbeteringen.

Vooraf zal er nog kort worden toegelicht welke big O er speelt binnen het code omdat dit in de rubric staat, dit heeft echter geen resultaat op code score.
Ook wordt er via CLion profiler gekeken naar Bottlenecks (tijd die het duurt om een bepaalde actie uit te voeren).

## Bottlenecks

Met profilling gezien dat paar functies zijn die veel tijd in beslag nemen.
Deze kwamen in de volgende volgorde in beeld.

- Game::tick
- Game::update
- Game::update_rockets
- Game::draw
- Game::insertion_sort_tanks_health
- Game::draw
- Rocket::intersects
- Tank::compare_health

Mijn focus is op algoritmes dus ik heb heb eerst gefocust op "insertion_sort_tanks_health"

## Big-O

### Eerste big-o

Big-O van insertion_sort_tanks_health = O(n^2)
Big-O van quicksort = O(n log n)
Ik heb dus voor quicksort gekozen omdat insertion sort met groote van de tanks trager wordt.

## Algoritmes

### Eerste algoritme

Ik heb de insertion_sort_tanks_health aangepast naar een quicksort algoritme.
Dit heb ik verwerk in game.cpp

## Threading

## Resultaat

## Bronnen

- [Dijkstra Algorithm Geek for geeks]("https://www.geeksforgeeks.org/introduction-to-dijkstras-shortest-path-algorithm/")
- [Learn Quick Sort in 13 minutes ⚡]("https://www.youtube.com/watch?v=Vtckgz38QHs")

