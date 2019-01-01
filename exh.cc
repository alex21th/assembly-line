/*
  ________________________
 /\                       \
 \_|   EXHAUSTIVE SEARCH  |
   |                      |
   |        exh.cc        |
   |   ___________________|_
    \_/_____________________/
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
using namespace std;

// Data structure that sets the number of cars and
// the assignment of improvements of a car model.
struct car_model {
  int num_cars;
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

// We define the minimum penalty at a value close to infinity.
int min_penalty = INT_MAX;

// Function that generates all possible ways to extend
// a partial solution given a set of parameters.
void generate(int k, vector<int>& solution, vector<int>& used, int partial_penalty,
    const vector<car_model>& models, const vector<pair<int,int>>& resources,
    const string argv, const clock_t time) {

  // We store the size of the solution and the number of classes.
  int n = solution.size();
  int classes = models.size();

  // If the solution is completed, we write it and update the minimum penalty.
  if (k == n) {
    write_to_file(partial_penalty, time, solution, argv);
    min_penalty = partial_penalty;
  }

  // Otherwise we generate all possible ways to extend the partial solution.
  else {
    // We extend the partial solution for all possible classes 'i'.
    for (int i = 0; i < classes; ++i)
      // If we have not used all cars of a particular class, we try them.
      if (used[i] < models[i].num_cars) {
        // We fill the solution with a car model and mark it as used.
        solution[k] = i;
        ++used[i];
        // We keep the old penalty and compute the new penalty of the solution.
        int old_penalty = partial_penalty;
        partial_penalty = penalties(solution, k, partial_penalty, models, resources);
        // We extend the solution only if we have found a smaller penalty.
        if (partial_penalty < min_penalty)
          generate(k+1, solution, used, partial_penalty, models, resources, argv, time);
        // In case we do not extend the solution, we restore the old penalty.
        partial_penalty = old_penalty;
        --used[i];
      }
} }

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
  int group; bool upgrade;
  for (int i = 0; i < classes; ++i) {
    in >> group >> models[group].num_cars;
    for (int j = 0; j < improvements; ++j) {
      in >> upgrade;
      models[group].upgrades.push_back(upgrade);
  } }

  // We close the file input stream.
  in.close();

  // We get the current time before executing the
  // exhaustive search algorithm of the solution.
  clock_t time;
  time = clock();

  // We create a partial void solution that will be completed
  // and a vector that will keep track of the used car classes.
  vector<int> solution(cars);
  vector<int> used(classes, 0);
  // We generate all possible ways to extend the partial solution.
  generate(0, solution, used, 0, models, resources, argv[2], time);
}
