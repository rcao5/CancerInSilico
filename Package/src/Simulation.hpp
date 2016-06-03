#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include "CellPopulation.hpp"
#include "Parameters.hpp"

#include <Rcpp.h>

class Simulation {

private:

  CellPopulation m_cells;
  Parameters* m_param;
  
public:

  Simulation(Parameters*);
	~Simulation() {}

  void Run(int);
  Rcpp::NumericMatrix GetCellsAsMatrix();

};

#endif
