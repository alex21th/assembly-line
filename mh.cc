/*
  ________________________
 /\                       \
 \_|     METAHEURISTIC    |
   |                      |
   |         mh.cc        |
   |   ___________________|_
    \_/_____________________/
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <time.h>
using namespace std;

// Data structure that sets the model, the number of cars, the number
// of upgrades and the assignment of improvements of a car model.
struct car_model {
  int model;
  int num_cars;
  int num_upgrades = 0;
  vector<bool> upgrades;
};

// Function that writes the penalty, the computation time
// and the sequence of a solution in a file 'argv'.
void write_to_file(const int penalty, const clock_t time,
    const vector<int>& solution, const string argv) {

  // We set an output stream with a precision of a decimal.
  ofstream out(argv);
  out.setf(ios::fixed); out.precision(1);

  // We write the penalty of the solution and the time to get it in seconds.
  double seconds = float(clock() - time)/CLOCKS_PER_SEC;
  out << penalty << ' ' << seconds << endl;

  // We write the sequence of the solution.
  for (int i = 0; i < solution.size(); ++i)
    (i == 0) ? out << solution[i] : out << ' ' << solution[i];
  out << endl;

  // We close the file output stream.
  out.close();
}

// Function that computes penalties of all windows of a partial
// solution that reach an index 'k' given a set of parameters.
int penalties(const vector<int>& solution, int k, int partial_penalty,
    const vector<car_model>& models, const vector<pair<int, int>>& resources) {

  // We set the penalties we will calculate to
  // zero and get the number of improvements.
  int new_penalties = 0;
  int improvements = models[0].upgrades.size();

  // We calculate the penalties for each station 's'.
  for (int s = 0; s < improvements; ++s) {

    // We calculate the incomplete windows at the beginning of the sequence.
    if (k < resources[s].second - 1) {
      int num_upgrades = 0;
      for (int p = 0; p <= k; ++p) // We sum the number of upgrades.
        if (models[solution[p]].upgrades[s]) ++num_upgrades;
      if (num_upgrades > resources[s].first) // We check and compute penalties.
        new_penalties += num_upgrades - resources[s].first;
    }

    // We calculate the complete windows that reach the index 'k'.
    if (k >= resources[s].second - 1) {
      int num_upgrades = 0;
      // We sum the number of upgrades.
      for (int p = k - resources[s].second + 1; p <= k; ++p)
        if (models[solution[p]].upgrades[s]) ++num_upgrades;
      if (num_upgrades > resources[s].first) // We check and compute penalties.
        new_penalties += num_upgrades - resources[s].first;
    }

    // We calculate the incomplete windows at the end of the sequence.
    int n = solution.size();
    if (k == n - 1) {
      int i = n - resources[s].second + 1;
      while (i < n - 1) { // We calculate windows up to that of mesure two.
        int num_upgrades = 0;
        for (int p = i; p <= k; ++p) // We sum the number of upgrades.
          if (models[solution[p]].upgrades[s]) ++num_upgrades;
        if (num_upgrades > resources[s].first) // We check and compute penalties.
          new_penalties += num_upgrades - resources[s].first;
        ++i;
      }
    }
  }
  // We sum the new penalties until index 'k' plus the previous penalty.
  return partial_penalty + new_penalties;
}

// Function that randomly picks two different indexes of a solution vector.
vector<int> random_pair(const vector<int>& solution) {
  int n = solution.size();
  int i = rand() % n, j = rand() % n;
  while (i == j) j = rand() % n;
  return {i, j};
}

// We define the parameters and constraints of the simulated annealing function.
const long double TEMPERATURE = 1000;
const double TERMINATION_CONDITIONS = 0.001;
const double ALPHA = 0.9999;

// Function that finds a solution with a random approach
// by following a metaheuristic given a set of parameters.
void simulated_annealing(vector<int>& solution, vector<int>& used,
    vector<car_model>& models, const vector<pair<int,int>>& resources,
    const string argv, const clock_t time) {

  // We store the size of the solution and the number of classes.
  int n = solution.size();
  int classes = models.size();

  // We fill the solution by putting a car of each
  // model one after another while supplies last.
  for (int i = 0; i < n; ++i) {
    // If we have not used all cars of a particular class, we put one.
    if (used[i%classes] < models[i%classes].num_cars) {
      solution[i] = models[i%classes].model;
      ++used[i%classes];
    }
    // Otherwise we put a car of the next model of which we still have stock.
    else {
      int j = i;
      while (used[j%classes] == models[j%classes].num_cars) ++j;
      solution[i] = models[j%classes].model;
      ++used[j%classes];
    }
  }

  // We shuffle the cars of the solution to get a random setting.
  random_shuffle(solution.begin(), solution.end());

  // We create a neighbor solution and set the penalty
  // of the solution and the neighbor to zero.
  vector<int> neighbor(n);
  int penalty_sol = 0, penalty_nei = 0;

  // We set an initial temperature value.
  long double temp = TEMPERATURE;
  // We iterate based on temperature.
  while (temp > TERMINATION_CONDITIONS) {
    // We compute the penalties for the entire solution.
    penalty_sol = 0;
    for (int k = 0; k < n; ++k)
      penalty_sol = penalties(solution, k, penalty_sol, models, resources);

    // We find a neighbor by choosing a random pair
    // of indexes of the solution and swapping them.
    neighbor = solution;
    vector<int> pair = random_pair(solution);
    swap(neighbor[pair[0]], neighbor[pair[1]]);

    // We compute the penalties for the entire neighbor solution.
    penalty_nei = 0;
    for (int k = 0; k < n; ++k)
      penalty_nei = penalties(neighbor, k, penalty_nei, models, resources);

    // If the neighbor solution is better than the previous
    // solution, we write it and update the solution.
    if (penalty_nei < penalty_sol) {
      write_to_file(penalty_nei, time, neighbor, argv);
      solution = neighbor;
    }
    // Otherwise we accept a worsening move with a probability
    // that is decreased during the search.
    else
      if (exp(-(penalty_nei-penalty_sol)/temp) > (double) rand() / (RAND_MAX))
        solution = neighbor;

    // We update the temperature by using a parameter alpha 'α'.
    temp *= ALPHA;
  }
}

int main(int argc, char** argv) {
  // We create an input stream class to operate on files.
  ifstream in(argv[1]);

  // We read the number of cars 'C', improvements 'M' and classes 'K'.
  int cars, improvements, classes;
  in >> cars >> improvements >> classes;
  // We read the number of cars 'ce' which can require an
  // improvement out of a window of 'ne' consecutive cars.
  vector<pair<int,int>> resources(improvements);
  for (int i = 0; i < improvements; ++i) in >> resources[i].first;
  for (int i = 0; i < improvements; ++i) in >> resources[i].second;
  // We read the number of cars of each class
  // and whether they need or not an upgrade.
  vector<car_model> models(classes);
  bool upgrade;
  for (int i = 0; i < classes; ++i) {
    in >> models[i].model >> models[i].num_cars;
    for (int j = 0; j < improvements; ++j) {
      in >> upgrade;
      if (upgrade) ++models[i].num_upgrades;
      models[i].upgrades.push_back(upgrade);
  } }

  // We close the file input stream.
  in.close();

  // We get the current time before executing
  // the metaheuristic of the solution.
  clock_t timer;
  timer = clock();
  // We set a seed for the random number generator.
  srand(time(NULL));

  // We create a void solution that will be filled and a
  // vector that will keep track of the used car classes.
  vector<int> solution(cars);
  vector<int> used(classes, 0);
  // We find a solution by following the simulated annealing metaheuristic.
  simulated_annealing(solution, used, models, resources, argv[2], timer);
}
