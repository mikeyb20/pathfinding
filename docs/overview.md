# Pathfinding Algorithm Visualizer & Benchmark Suite

## Project Overview

A real-time pathfinding visualizer that implements multiple pathfinding algorithms side by side, with interactive map editing, adversarial test scenarios, and performance benchmarking. The purpose is not just to implement the algorithms (Claude Code handles that quickly) but to develop deep intuition for **when each algorithm fails, why, and what the performance tradeoffs actually look like** on real maps with real constraints.

The project naturally extends into logistics and flow-based problems that are directly relevant to factory/logistics game development.

---

## Core Concepts

### What Makes This Educational
Most developers know A* and stop there. This project forces you to understand:

- **Why A* is slow on large open maps** and what hierarchical approaches fix
- **Why grid-based pathing looks unnatural** and how navigation meshes solve it
- **When flow fields beat individual paths** (many agents, same destination)
- **What "optimal" actually costs** and when "good enough" is better
- **How heuristics shape search behavior** and what happens when they're wrong

### The Algorithms (Ordered by Implementation Complexity)

**Tier 1: Foundations**
- Breadth-First Search (BFS) — Unweighted, guarantees shortest path
- Dijkstra's Algorithm — Weighted, guarantees shortest path
- A* — Weighted + heuristic, guarantees optimal path (with admissible heuristic)

**Tier 2: Optimizations**
- Jump Point Search (JPS) — A* on uniform grids, skips symmetrical paths. 10-50x faster than A* on open maps
- Bidirectional A* — Search from both ends, meet in the middle
- Theta* — Any-angle pathfinding, produces more natural paths than grid-restricted A*

**Tier 3: Multi-Agent / Mass Pathfinding**
- Flow Fields — Compute once, all agents use. Essential for RTS/factory games
- Dijkstra Maps (influence maps) — Precomputed distance fields for AI decision-making

**Tier 4: Hierarchical / Advanced**
- Hierarchical Pathfinding (HPA*) — Divide map into clusters, path between clusters first, then refine
- Navigation Meshes (NavMesh) — Decompose walkable space into convex polygons, path through polygon graph

**Tier 5: Graph Theory / Logistics**
- Minimum Spanning Tree (Prim's / Kruskal's)
- Maximum Flow (Ford-Fulkerson / Edmonds-Karp)
- Shortest paths all-pairs (Floyd-Warshall)
- Traveling Salesman heuristics (nearest neighbor, 2-opt)

---

## Architecture

```
Application
├── Core
│   ├── Grid (2D tile-based world representation)
│   ├── NavGraph (abstract graph for non-grid algorithms)
│   ├── NavMesh (polygon-based navigation mesh)
│   └── CoordinateSystem (world <-> grid <-> screen transforms)
│
├── Algorithms
│   ├── IPathfinder (interface: find_path, get_visited, get_frontier)
│   ├── BFS
│   ├── Dijkstra
│   ├── AStar
│   │   ├── Heuristics (Manhattan, Euclidean, Octile, Chebyshev)
│   │   └── TieBreaking strategies
│   ├── JumpPointSearch
│   ├── BidirectionalAStar
│   ├── ThetaStar
│   ├── FlowField
│   ├── HierarchicalAStar
│   └── NavMeshPathfinder
│
├── Visualization
│   ├── Renderer (grid, paths, visited sets, frontier)
│   ├── AlgorithmAnimator (step-by-step playback)
│   ├── SplitView (side-by-side algorithm comparison)
│   ├── HeatmapRenderer (visit frequency, cost maps)
│   └── StatisticsOverlay (nodes visited, path cost, time)
│
├── MapEditor
│   ├── TerrainPainter (walls, weighted terrain, one-way edges)
│   ├── AgentPlacer (start, goal, multiple agents)
│   ├── MapGenerator (random, maze, rooms, open field, bottleneck)
│   └── MapSerializer (save/load maps as JSON)
│
├── Benchmarking
│   ├── BenchmarkRunner (run all algorithms on same map)
│   ├── MetricsCollector (time, nodes expanded, path length, memory)
│   └── ResultsExporter (CSV/chart output)
│
└── Scenarios
    ├── AdversarialMaps (designed to break specific algorithms)
    ├── ScaleTests (increasing map sizes)
    └── MultiAgentScenarios (flow field vs individual pathing)
```

### Key Data Structures

```cpp
struct GridCell {
    int x, y;
    float movementCost;      // 1.0 = normal, higher = harder, INF = wall
    TerrainType terrain;     // OPEN, WALL, WATER, MUD, etc.
    bool walkable;
};

struct PathResult {
    std::vector<Vec2i> path;
    float totalCost;
    int nodesExpanded;
    int nodesInFrontier;
    float computeTimeMs;
    std::vector<Vec2i> visitedOrder;  // for visualization playback
};

// For flow fields
struct FlowFieldCell {
    float distanceToGoal;    // from Dijkstra pass
    Vec2 direction;          // normalized direction toward goal
    bool reachable;
};

// For NavMesh
struct NavPolygon {
    std::vector<Vec2> vertices;
    std::vector<int> neighborIndices;
    Vec2 centroid;
};

// Common interface
class IPathfinder {
public:
    virtual PathResult findPath(const Grid& grid, Vec2i start, Vec2i goal) = 0;
    virtual std::string getName() const = 0;
    // For step-by-step visualization
    virtual void initSearch(const Grid& grid, Vec2i start, Vec2i goal) = 0;
    virtual bool step() = 0;  // returns false when done
    virtual const SearchState& getCurrentState() const = 0;
};
```

---

## Phase Breakdown

### Phase 1: Grid, Rendering, and BFS (Hours 1–6)
**Goal:** A grid you can draw walls on, with BFS finding a path and animating the search.

- Implement `Grid` class with cell types
- Set up rendering (raylib recommended for simplicity)
- Mouse interaction: click to place walls, right-click to set start/goal
- Implement BFS with visited-order tracking
- Animate: step through BFS one node per frame, color visited nodes, highlight frontier, draw final path
- **Milestone:** Draw a maze, watch BFS flood-fill to find the exit

### Phase 2: Dijkstra and A* (Hours 6–12)
**Goal:** Weighted terrain and heuristic search.

- Add terrain types with movement costs (mud = 3x, water = 5x, etc.)
- Terrain painting in the editor
- Implement Dijkstra's (priority queue on cost-so-far)
- Implement A* with selectable heuristics:
  - Manhattan: `|dx| + |dy|` — best for 4-directional
  - Euclidean: `sqrt(dx² + dy²)` — admissible but slow to converge
  - Octile: `max(|dx|, |dy|) + (√2-1) * min(|dx|, |dy|)` — best for 8-directional
  - Chebyshev: `max(|dx|, |dy|)` — interesting failure cases
- Add side-by-side split view: run two algorithms on the same map simultaneously
- **Milestone:** Watch A* expand far fewer nodes than Dijkstra on the same map. Switch heuristics and see the search shape change.

**Key learning moment:** Watch what happens when you use Manhattan distance on an 8-directional grid. The heuristic is no longer admissible for diagonal movement and A* can find suboptimal paths. This is the kind of thing you only internalize by seeing it.

### Phase 3: Jump Point Search (Hours 12–18)
**Goal:** Understand symmetry breaking on uniform grids.

- Implement JPS (works only on uniform-cost grids with 8-directional movement)
- Core concept: instead of expanding every neighbor, "jump" in a direction until hitting a wall or a "forced neighbor"
- Visualize the jump points vs. the nodes A* would expand
- **Milestone:** On a large open map, JPS expands 10-50x fewer nodes than A*

**Key learning moment:** JPS is dramatically faster on open maps but gains nothing on dense mazes (where every cell is a jump point). Build a map that makes JPS slower than A* — it's a corridor maze where jumps terminate immediately. Understanding *why* builds intuition about algorithmic assumptions.

### Phase 4: Flow Fields (Hours 18–26)
**Goal:** Mass pathfinding for many agents to one destination.

- Implement Dijkstra-based distance field from goal
- Convert distance field to direction field (each cell points toward lowest-cost neighbor)
- Spawn 100-1000 agents, have them all follow the flow field
- Visualize: direction arrows in each cell, agents streaming along paths
- Compare: 100 individual A* calls vs. 1 flow field computation
- Add multiple goals (separate flow fields per goal, agents pick nearest)
- **Milestone:** 1000 agents smoothly navigate a complex map

**Key learning moment:** Flow fields have a high upfront cost but O(1) per-agent lookup. The crossover point where flow fields beat individual pathfinding depends on agent count and map size. Find that crossover empirically by benchmarking.

**Tuning parameters:** Agent steering (how closely agents follow the field vs. smoothing their motion), obstacle avoidance between agents (local avoidance layer on top of flow field), field resolution vs. map resolution.

### Phase 5: Theta* and Any-Angle Pathfinding (Hours 26–32)
**Goal:** Paths that don't look like they're on a grid.

- Implement Theta*: like A* but checks line-of-sight to parent's parent
- If line of sight exists, skip the intermediate node (produces straight-line segments)
- Implement line-of-sight check (Bresenham's line or raycast against grid)
- Compare path quality: A* produces staircase paths, Theta* produces natural diagonals
- **Milestone:** Side-by-side showing A*'s jagged path vs Theta*'s smooth path

**Key learning moment:** Theta* paths are shorter and more natural, but the line-of-sight checks are expensive. On dense maps with lots of obstacles, Theta* can be significantly slower than A* for marginal path improvement. Profile both and find the terrain density where Theta* stops being worth it.

### Phase 6: Hierarchical Pathfinding / HPA* (Hours 32–42)
**Goal:** Pathfinding on maps too large for A* to handle in real-time.

- Divide the grid into rectangular clusters (e.g., 16x16)
- Compute entrance/exit nodes at cluster borders
- Build an abstract graph connecting border nodes
- Path: A* on abstract graph for rough path, then A* within each cluster for refinement
- Visualize: cluster boundaries, abstract graph edges, two-level search
- **Milestone:** Pathfinding on a 1000x1000 grid in <1ms

**Tuning parameters:** Cluster size is the critical knob. Too small = too many abstract nodes, too large = intra-cluster A* is expensive. The optimal size depends on map density and average path length. Experiment with 8x8, 16x16, 32x32, 64x64 and profile.

**Key learning moment:** HPA* paths are not optimal — the cluster boundaries force suboptimal routing. Visualize the suboptimality by overlaying the HPA* path on the true A* optimal path. Understanding this tradeoff (speed vs. optimality) is fundamental to real-world pathfinding.

### Phase 7: Navigation Meshes (Hours 42–54)
**Goal:** Break free from grids entirely.

- Implement walkable space decomposition (start with simple polygon decomposition)
- Generate NavMesh from a grid (merge walkable cells into convex polygons)
- Build adjacency graph between polygons
- A* through polygon graph (using centroid or edge midpoint distances)
- Funnel algorithm for path smoothing (find shortest path through portal edges)
- **Milestone:** Smooth, natural-looking paths through complex environments without grid artifacts

**Key learning moment:** NavMesh generation is the hard part. Getting a clean mesh from an obstacle map requires understanding constrained Delaunay triangulation or similar decomposition. The pathfinding on the mesh itself is straightforward — it's the mesh construction that teaches you computational geometry.

### Phase 8: Benchmarking Suite (Hours 54–62)
**Goal:** Rigorous performance comparison across algorithms and map types.

- Implement automated benchmark runner
- Map categories:
  - **Open field** — tests raw algorithm speed, JPS dominance
  - **Dense maze** — tests exploration efficiency, JPS weakness
  - **Rooms and corridors** — realistic game map, tests hierarchical approaches
  - **Bottleneck map** — single narrow passage, tests worst-case behavior
  - **Large scale** — 1000x1000+, tests scalability
  - **Weighted terrain** — varied costs, tests algorithms that only work on uniform grids
- Metrics per run: computation time, nodes expanded, peak memory, path length, path optimality ratio
- Generate comparison charts (path cost vs compute time scatter plot)
- Export results to CSV for analysis
- **Milestone:** A comprehensive report showing which algorithm wins on which map type and why

### Phase 9: Adversarial Maps and Edge Cases (Hours 62–72)
**Goal:** Build maps designed to break each algorithm, deepening understanding.

Adversarial scenarios to implement:

| Map | Target Algorithm | What Breaks |
|---|---|---|
| Long spiral maze | A* with Euclidean heuristic | Heuristic says goal is close, but path is extremely long. A* explores nearly everything. |
| Open field with single wall | JPS | JPS skips over the wall and has to backtrack. Overhead of forced neighbors negates jump savings. |
| Many agents, many goals | Flow fields | Need multiple flow fields, memory explodes. When does it stop being cheaper than individual A*? |
| Narrow corridor grid | HPA* | Cluster boundaries in the corridor force suboptimal abstract paths. Path quality degrades badly. |
| Dynamic obstacles | All pre-computed approaches | Obstacles move, invalidating cached paths. Tests replanning strategies. |
| Non-uniform costs everywhere | JPS, flow fields | JPS only works on uniform grids. Flow fields need recomputation for cost changes. |

---

## Map Types and Generators

Build procedural generators for each — essential for benchmarking and testing:

```
RandomWalls       — Scatter random wall cells at configurable density
RecursiveMaze     — Perfect maze via recursive backtracking
RoomsAndCorridors — BSP-based room placement connected by hallways
OpenWithIslands   — Large open space with scattered obstacle clusters
Bottleneck        — Two open areas connected by a narrow passage
WeightedTerrain   — Perlin noise-based movement cost variation
Spiral            — Adversarial spiral toward center goal
```

---

## Visualization Features

### Essential (Build First)
- Color-coded visited/frontier/path cells
- Step-by-step animation with speed control
- Side-by-side algorithm comparison (same map, same start/goal)
- Real-time stats overlay (nodes expanded, time, path cost)

### Advanced (Build Later)
- Heatmap mode: color cells by visit frequency across many random queries
- Flow field arrow overlay
- NavMesh polygon boundary rendering
- Abstract graph overlay for HPA*
- Path smoothing visualization (before/after)
- Agent simulation on top of pathfinding (steering, local avoidance)

---

## Relevance to Arcane Industry

Several of these algorithms directly apply to a factory/logistics game:

| Game System | Relevant Algorithm |
|---|---|
| Worker/vehicle routing | A*, Theta*, NavMesh |
| Mass unit movement | Flow fields |
| Conveyor belt / pipe routing | Dijkstra with custom cost functions |
| Supply chain optimization | Min-cost max-flow |
| Logistics network design | Minimum spanning tree, TSP heuristics |
| Large world pathfinding | HPA*, hierarchical approaches |
| AI decision-making | Dijkstra maps / influence maps |

The benchmarking work here directly informs which algorithm to use for which system in the game.

---

## Debugging Toolkit

- **Path validation** — Verify path is connected and doesn't pass through walls
- **Optimality checker** — Run Dijkstra to get true optimal cost, compare against algorithm output
- **Step counter** — Verify algorithm terminates (detect infinite loops)
- **Heuristic analyzer** — Visualize h(n) across the map as a gradient, check admissibility
- **Memory tracker** — Monitor peak allocation per algorithm per map

---

## Common Bugs and What They Look Like

| Symptom | Likely Cause |
|---|---|
| Path goes through walls | Line-of-sight check is wrong (Theta*), or grid neighbor check doesn't validate |
| Path is clearly suboptimal | Heuristic is inadmissible (overestimates), or tie-breaking is biased wrong |
| Algorithm runs forever | Open set comparison is wrong, or goal check is missing |
| JPS finds different path than A* | JPS assumes uniform costs — using it on weighted terrain silently breaks |
| Flow field agents oscillate | Direction field has saddle points, need smoothing or tie-breaking |
| HPA* path has weird detours | Cluster border nodes don't capture all valid transitions |
| NavMesh path hugs walls | Funnel algorithm implementation error, or degenerate polygon in mesh |

---

## Recommended Libraries / Dependencies

- **Rendering:** raylib (simplest for 2D grid rendering)
- **GUI:** Dear ImGui (algorithm selection, parameter tweaking, speed control)
- **Benchmarking:** nanobench or manual high_resolution_clock timing
- **Serialization:** nlohmann/json for map save/load
- **Noise (for terrain gen):** FastNoiseLite

---

## Stretch Goals

- Dynamic replanning (D* Lite) — recalculate paths efficiently when map changes
- Multi-agent coordination (cooperative pathfinding, WHCA*)
- 3D extension — pathfinding on a voxel grid or 3D NavMesh
- GPU-accelerated flow field computation
- Integration with Arcane Industry as a drop-in pathfinding module