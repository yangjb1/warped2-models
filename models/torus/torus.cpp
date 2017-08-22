#include <vector>
#include <memory>
#include <cassert>
#include <algorithm>
#include <random>
#include <cstdlib>

#include "warped.hpp"
#include "torus.hpp"
#include "ppm/ppm.hpp"
#include "tclap/ValueArg.h"

#define TS_INTERVAL 1 /* Timestamp interval */

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(NodeEvent)

std::string node_name (unsigned int index, unsigned int dimension) {
    return std::string("Node_") + std::to_string(index) + std::string("_") + std::to_string(dimension);
}

std::vector<std::shared_ptr<warped::Event> > Node::initializeLP() {
    std::vector<std::shared_ptr<warped::Event>> events;

    events.emplace_back( new NodeEvent { node_name(this->index_, this->dimension_index_), TS_INTERVAL } );

    return events;
}

std::vector<std::shared_ptr<warped::Event> > Node::receiveEvent(const warped::Event& event) {
    std::vector<std::shared_ptr<warped::Event>> events;
    auto node_event = static_cast<const NodeEvent&>(event);
    unsigned int event_ts = node_event.event_ts_ + TS_INTERVAL;

    events.emplace_back(new NodeEvent {this->name_, event_ts});

    return events;
}

int main(int argc, const char **argv) {
    /* Set the default values for the argument */
    unsigned int grid_dimension = 5;
    unsigned int grid_size = 1000;
    unsigned int grid_order = 4;

    /* Read the argument */
    TCLAP::ValueArg<unsigned int> grid_dimension_arg("d", "dimension",
                    "Dimensionality of the torus", false, grid_dimension, "unsigned int");
    TCLAP::ValueArg<unsigned int> grid_size_arg("s", "size",
                    "Size of the torus grid", false, grid_size, "unsigned int");
    TCLAP::ValueArg<unsigned int> grid_order_arg("o", "order",
                    "Order of nodes in torus", false, grid_order, "unsigned int");

    std::vector<TCLAP::Arg*> cmd_line_args = {   &grid_dimension_arg, 
                                                 &grid_size_arg, 
                                                 &grid_order_arg 
                                             };

    warped::Simulation simulation {"Torus Network Simulation", argc, argv, cmd_line_args};

    grid_dimension = grid_dimension_arg.getValue();
    grid_size = grid_size_arg.getValue();
    grid_order = grid_order_arg.getValue();

    /* Create the LPs */
    std::vector<Node> lps;
    std::vector<warped::LogicalProcess*> lp_pointers;

    /* Torus is a grid of size n with k dimensions */
    for (unsigned int k = 0; k < grid_dimension; k++) {
        for (unsigned int n = 0; n < grid_size; n++) {
            lps.emplace_back(node_name(n, k), grid_dimension, grid_size, grid_order, k, n);
        }
    }

    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }

    simulation.simulate(lp_pointers);

    return 0;
}

