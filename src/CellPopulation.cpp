#include <cmath>
#include <Rcpp.h>

#include "CellPopulation.h"

CellPopulation::CellPopulation(Parameters *par, unsigned int size, double density) {

    m_param = par;
    m_population = SpatialHash<Cell>(1.0);
    double disk_radius = pow(size / density, 0.5);
    Point new_loc;
    Cell* temp;

    //create cells
    for (unsigned int i = 0; i < size; i++) {

        temp = new Cell(Point(0,0), m_param);
        GetRandomLocation(temp, disk_radius);
        m_population.Insert(temp->GetCoord(), temp);
        Rcpp::checkUserInterrupt();

    }

    SetGrowthRates();
    SeedCells();

}

CellPopulation::~CellPopulation() {

    SpatialHash<Cell>::full_iterator iter = m_population.begin();
    for (; iter != m_population.end(); ++iter) {

        delete &iter;

    }

}

Point CellPopulation::GetRandomLocation(Cell* cell, double rad) {

    double dist, ang, x, y;

    do {

        dist = R::runif(0, 1);
        ang = R::runif(0, 2 * M_PI);
        x = rad * pow(dist, 0.5) * cos(ang);
        y = rad * pow(dist, 0.5) * sin(ang);
        cell->SetCoord(Point(x,y));

    } while (!ValidCellPlacement(cell));

    return Point(x, y);

}

bool CellPopulation::ValidCellPlacement(Cell* cell) {

    SpatialHash<Cell>::full_iterator iter = m_population.begin();

    for (; iter != m_population.end(); ++iter) {

        if ((*iter).CellDistance(*cell) < 2) {

            return false;

        }

    }

    return true;

}

void CellPopulation::SetGrowthRates() {

    SpatialHash<Cell>::full_iterator iter = m_population.begin();

    for (; iter != m_population.end(); ++iter) {

        (*iter).SetGrowth(m_param->GetRandomGrowthRate());

    }

}

void CellPopulation::SeedCells() {

    double cycle_time, unif;
    SpatialHash<Cell>::full_iterator iter = m_population.begin();
    for (; iter != m_population.end(); ++iter) {

        unif = R::runif(0,1);

        if (unif < 0.75) { //interphase

            (*iter).SetRadius(R::runif(1,m_param->GetMaxRadius()));
            (*iter).SetAxisLength((*iter).GetRadius() * 2);

        } else { //mitosis

            (*iter).EnterRandomPointOfMitosis();

        }

    }

}

void CellPopulation::OneTimeStep() {

    int sz = m_population.size();

    for (int i = 0; i < sz; ++i) {

        Update();

    }

}

void CellPopulation::Update() {

    Cell* rand_cell = m_population.GetRandomValue();
    AttemptTrial(rand_cell);
CheckMitosis(rand_cell);

}

void CellPopulation::CheckMitosis(Cell* cell) {

    if (cell->ReadyToDivide()) {

        double gr_rate;    
        if (m_param->InheritGrowth()) {

            gr_rate = cell->GetGrowth();

        } else {

            gr_rate = m_param->GetRandomGrowthRate();

        }

        Point old_key = cell->GetCoord();
        Cell* daughter_cell = new Cell(cell->Divide(), gr_rate);
        m_population.Insert(daughter_cell->GetCoord(), daughter_cell);
        m_population.Update(old_key, cell->GetCoord());

    }

}

void CellPopulation::AttemptTrial(Cell *cell) {

    double pre_interaction = CalculateTotalInteraction(cell);
    int num_neighbors = CalculateNumberOfNeighbors(cell);
    Cell orig = *cell;
    bool growth = cell->DoTrial();

    double max_search = std::max(m_param->GetMaxTranslation(), m_param->GetMaxRadius());

    SpatialHash<Cell>::circular_iterator iter
        = m_population.begin(orig.GetCoord(), max_search);


    for (; iter != m_population.end(orig.GetCoord(), max_search); ++iter) {

        if ((*iter).CellDistance(*cell) < 0) {

            *cell = orig;

        }

    }

    m_population.Update(orig.GetCoord(), cell->GetCoord());
    
    if (growth) {
        
        return;

    } else {

        double post_interaction = CalculateTotalInteraction(cell);

        if (post_interaction == std::numeric_limits<double>::max()
                || !AcceptTrial(post_interaction - pre_interaction)
                || num_neighbors > CalculateNumberOfNeighbors(cell)) {

            m_population.Update(cell->GetCoord(), orig.GetCoord());
            *cell = orig;

        }
    
    }

}

bool CellPopulation::AcceptTrial(double delta_interaction) {

    if (delta_interaction <= 0.0) {

        return true;

    } else {

        double unif = R::runif(0, 1);
        double prob = exp(-1 * delta_interaction);
        return unif < prob;

    }

}

int CellPopulation::CalculateNumberOfNeighbors(Cell *cell) {

    int neighbors = 0;
    SpatialHash<Cell>::circular_iterator iter
        = m_population.begin(cell->GetCoord(), m_param->GetCompressionDELTA() + 1);

    for (; iter != m_population.end(cell->GetCoord(), m_param->GetCompressionDELTA() + 1); ++iter) {

        if (cell->CellDistance(*iter) <= m_param->GetCompressionDELTA()) {

            neighbors++;

        }

    }

    return neighbors;

}
    
double CellPopulation::CalculateTotalInteraction(Cell *cell) {

    double inter, sum = 0.0;

    SpatialHash<Cell>::circular_iterator iter
        = m_population.begin(cell->GetCoord(), m_param->GetCompressionDELTA() + 1);

    for (; iter != m_population.end(cell->GetCoord(), m_param->GetCompressionDELTA() + 1); ++iter) {

        if (&iter != cell) {

            inter = CalculateInteraction(&iter, cell);

            if (inter == std::numeric_limits<double>::max()) {

                return inter;

            }

            sum += inter;

        }

    }

    return sum;

}

double CellPopulation::CalculateInteraction(Cell* a, Cell* b) {

    double dist = a->CellDistance(*b);

    if (dist > m_param->GetCompressionDELTA()) {

        return 0.0;

    } else if (dist < 0) { //should never be called

        return std::numeric_limits<double>::max();

    } else {

        double part = pow(2 * dist / m_param->GetCompressionDELTA() - 1, 2);
        return m_param->GetResistanceEPSILON() * (part - 1);

    }

}

int CellPopulation::size() {

    return m_population.size();

}

void CellPopulation::RecordPopulation() {

    std::vector<double> current_pop;
    SpatialHash<Cell>::full_iterator iter = m_population.begin();
    Cell *temp;

    for (; iter != m_population.end(); ++iter) {

        current_pop.push_back((*iter).GetCoord().x);
        current_pop.push_back((*iter).GetCoord().y);
        current_pop.push_back((*iter).GetRadius());
        current_pop.push_back((*iter).GetAxisLength());
        current_pop.push_back((*iter).GetAxisAngle());
        current_pop.push_back((*iter).GetGrowth());

    }

    m_population_record.push_back(current_pop);

}

Rcpp::List CellPopulation::GetPopulationAsList() {

    return Rcpp::wrap(m_population_record);

}

