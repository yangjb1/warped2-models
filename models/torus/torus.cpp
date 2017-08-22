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

std::string node_name (unsigned int index) {
    return std::string("Node_") + std::to_string(index);
}

std::vector<std::shared_ptr<warped::Event> > Node::initializeLP() {
    std::vector<std::shared_ptr<warped::Event>> events;

    for (unsigned int i = 0; i < grid_size_; i++) {
        events.emplace_back( new NodeEvent {node_name(this->index_), TS_INTERVAL} );
    }

    return events;
}

std::vector<std::shared_ptr<warped::Event> > Node::receiveEvent(const warped::Event& event) {
    auto node_event = static_cast<const NodeEvent&>(event);
    unsigned int event_ts = node_event.event_ts_ + TS_INTERVAL;

    std::vector<std::shared_ptr<warped::Event>> events;

    if (node_event.receiver_name_ == node_name(this->index_)) {
        events.emplace_back( new NodeEvent {node_name(this->index_), event_ts} );
    }

    return events;
}

unsigned int Node::neighbor(unsigned int index, direction_t direction) {
    unsigned int new_x = 0, new_y = 0;
    unsigned int current_y = index / grid_dimension_;
    unsigned int current_x = index % grid_dimension_;

    switch(direction) {
        case LEFT: {
            new_x = (current_x + grid_dimension_ - 1) % grid_dimension_;
            new_y = current_y;
        } break;

        case RIGHT: {
            new_x = (current_x + 1) % grid_dimension_;
            new_y = current_y;
        } break;

        case DOWN: {
            new_x = current_x;
            new_y = (current_y + 1) % grid_dimension_;
        } break;

        case UP: {
            new_x = current_x;
            new_y = (current_y + grid_dimension_ - 1) % grid_dimension_;
        } break;

        default: {
            std::cerr << "Invalid move direction " << direction << std::endl;
            assert(0);
        }
    }

    return (new_x + new_y * grid_dimension_);
}

int main(int argc, const char **argv) {
    /* Set the default values for the argument */
    unsigned int grid_dimension = 5;
    unsigned int grid_size = 1000;

    /* Read the argument */
    TCLAP::ValueArg<unsigned int> grid_dimension_arg("d", "grid-dimension",
                    "Dimensionality of the torus grid", false, grid_dimension, "unsigned int");
    TCLAP::ValueArg<unsigned int> grid_size_arg("s", "grid-size",
                    "Size of the torus grid", false, grid_size, "unsigned int");
    std::vector<TCLAP::Arg*> cmd_line_args = { &grid_dimension_arg, &grid_size_arg };
    warped::Simulation simulation {"Torus Network Simulation", argc, argv, cmd_line_args};

    grid_dimension = grid_dimension_arg.getValue();
    grid_size = grid_size_arg.getValue();

    /* Create the LPs */
    std::vector<Node> lps;
    std::vector<warped::LogicalProcess*> lp_pointers;

    unsigned int vertex_cnt   = grid_dimension * grid_dimension;

    for (unsigned int i = 0; i < vertex_cnt; i++) {

        lps.emplace_back(node_name(i), grid_dimension, grid_size, i);
    }

    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }

    simulation.simulate(lp_pointers);

    return 0;
}

