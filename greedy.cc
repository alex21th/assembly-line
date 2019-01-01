/*
  ________________________
 /\                       \
 \_|   GREEDY ALGORITHM   |
   |                      |
   |       greedy.cc      |
   |   ___________________|_
    \_/_____________________/
*/
#include <iostream>
#include <fstream>
#include <vector>
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
    const vector<car_model>& models, const vector<int>& legend,
    const vector<pair<int, int>>& resources) {

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
        if (models[legend[solution[p]]].upgrades[s]) ++num_upgrades;
      if (num_upgrades > resources[s].first) // We check and compute penalties.
        new_penalties += num_upgrades - resources[s].first;
    }

    // We calculate the complete windows that reach the index 'k'.
    if (k >= resources[s].second - 1) {
      int num_upgrades = 0;
      // We sum the number of upgrades.
      for (int p = k - resources[s].second + 1; p <= k; ++p)
        if (models[legend[solution[p]]].upgrades[s]) ++num_upgrades;
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
          if (models[legend[solution[p]]].upgrades[s]) ++num_upgrades;
        if (num_upgrades > resources[s].first) // We check and compute penalties.
          new_penalties += num_upgrades - resources[s].first;
        ++i;
      }
    }
  }
  // We sum the new penalties until index 'k' plus the previous penalty.
  return partial_penalty + new_penalties;
}

// Comparator function that establishes a sorting criterion by giving
// priority to those car models with the greatest number of upgrades
// and, secondly, to those with the greatest number of cars.
bool comparator(const car_model& model1, const car_model& model2) {
  if (model1.num_upgrades != model2.num_upgrades)
    return model1.num_upgrades > model2.num_upgrades;
  return model1.num_cars > model2.num_cars;
}

// Function that builds up a solution in small steps by following
// a greedy algorithm given a set of parameters.
void greedy(vector<int>& solution, vector<int>& used,
    vector<car_model>& models, const vector<pair<int,int>>& resources,
    const string argv, const clock_t time) {

  // We store the size of the solution and the number of classes.
  int n = solution.size();
  int classes = models.size();

  // We sort the vector of models according to the criterion of the comparator.
  sort(models.begin(), models.end(), comparator);

  // We create a legend that will be needed in 'penalties' function
  // to keep track of the original order of the models.
  vector<int> legend(classes);
  for (int i = 0; i < classes; ++i) legend[models[i].model] = i;

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

  // We compute the penalties for the entire solution.
  int penalty = 0;
  for (int k = 0; k < n; ++k)
    penalty = penalties(solution, k, penalty, models, legend, resources);

  // We write in a file the solution found.
  write_to_file(penalty, time, solution, argv);
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
  // the greedy algorithm of the solution.
  clock_t time;
  time = clock();

  // We create a partial void solution that will be completed
  // and a vector that will keep track of the used car classes.
  vector<int> solution(cars);
  vector<int> used(classes, 0);
  // We fill the solution by following the greedy algorithm.
  greedy(solution, used, models, resources, argv[2], time);
}
