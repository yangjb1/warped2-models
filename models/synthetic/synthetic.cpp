#include <cassert>
#include <random>
#include "synthetic.hpp"
#include "Graph.hpp"
#include "tclap/ValueArg.h"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(InternalEvent)
WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(ExternalEvent)

inline std::string Synthetic::lpName(const unsigned int lp_index) {

    return std::string("Node_") + std::to_string(lp_index);
}

std::vector<std::shared_ptr<warped::Event> > Synthetic::initializeLP() {

    // Register random number generator
    registerRNG<std::default_random_engine>(rng_);

    std::exponential_distribution<double> time_expo(1.0/mean_time_);
    std::vector<std::shared_ptr<warped::Event> > events;

    for (unsigned int i = 0; i < num_nodes_; i++) {
        unsigned int time = (unsigned int)std::ceil(time_expo(*rng_));
        events.emplace_back(new InternalEvent {time});
    }
    return events;
}

std::vector<std::shared_ptr<warped::Event> > Synthetic::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > response_events;
    std::exponential_distribution<double> time_expo(1.0/mean_time_);
    std::uniform_int_distribution<int> ext_node(0, adjacency_list_.size()-1);

    /* Check if event received is a self/internal timer event */
    if (event.sender_name_ == event.receiverName()) {

        /* Restart the timer/internal event */
        unsigned int time = (unsigned int)std::ceil(time_expo(*rng_));
        response_events.emplace_back(new InternalEvent {time});

        /* Send an external event to one of the nodes in its adjacency list */
        auto id = (unsigned int) ext_node(*rng_);
        response_events.emplace_back(
                new ExternalEvent { adjacency_list_[id],
                                    1,
                                    percent_state_size_change_,
                                    percent_state_change_,
                                    time    });

    } else { /* Event received from other LPs/nodes */

    }

    return response_events;
}

int main(int argc, const char** argv) {

    unsigned int num_nodes              = 100000;
    std::string network_type            = "Watts-Strogatz";
    std::string network_params          = "30,0.1";
    unsigned int mean_time              = 10;
    unsigned int state_size             = 100;
    double percent_state_size_change    = 5.0;
    double percent_state_change         = 10.0;

    TCLAP::ValueArg<unsigned int> num_nodes_arg("n", "num-nodes", "Number of nodes",
                                                            false, num_nodes, "unsigned int");
    TCLAP::ValueArg<std::string> network_type_arg("t", "network-type", "Network type",
                                                            false, network_type, "string");
    TCLAP::ValueArg<std::string> network_params_arg("p", "network-params",
                "Multiple network params separated by ','", false, network_params, "string");
    TCLAP::ValueArg<unsigned int> mean_time_arg("m", "mean-time", "Time to run the simulation",
                                                            false, mean_time, "unsigned int");
    TCLAP::ValueArg<unsigned int> state_size_arg("s", "state-size", "Size of the lp state",
                                                            false, state_size, "unsigned int");
    TCLAP::ValueArg<double> percent_state_size_change_arg("g", "percent-state-size-change", 
                "Change in state size in percentage", false, percent_state_size_change, "double");
    TCLAP::ValueArg<double> percent_state_change_arg("f", "percent-state-change",
                "Change state in percentage", false, percent_state_change, "double");

    std::vector<TCLAP::Arg*> args = {&num_nodes_arg, &mean_time_arg, &state_size_arg,
                            &percent_state_size_change_arg, &percent_state_change_arg};

    warped::Simulation synthetic_sim {"Synthetic Simulation", argc, argv, args};

    num_nodes                   = num_nodes_arg.getValue();
    network_type                = network_type_arg.getValue();
    network_params              = network_params_arg.getValue();
    mean_time                   = mean_time_arg.getValue();
    state_size                  = state_size_arg.getValue();
    percent_state_size_change   = percent_state_size_change_arg.getValue();
    percent_state_change        = percent_state_change_arg.getValue();

    std::vector<Synthetic> lps;
    std::vector<std::string> lp_names;

    for (unsigned int i = 0; i < num_nodes; i++) {
        auto name = Synthetic::lpName(i);
        lp_names.push_back(name);
        lps.emplace_back(name, num_nodes, mean_time, state_size,
                percent_state_size_change, percent_state_change, i);
    }

    std::vector<warped::LogicalProcess*> lp_pointers;
    for (auto& lp : lps) {
        lp_pointers.push_back(&lp);
    }

    // Create the Network Graph
    Graph *graph = nullptr;
    std::string delimiter = ",";
    if (network_type == "Watts-Strogatz") { // If the choice is Watts-Strogatz
        size_t pos = network_params.find(delimiter);
        std::string token = network_params.substr(0, pos);
        unsigned int k = (unsigned int) std::stoul(token);
        network_params.erase(0, pos + delimiter.length());
        double beta = std::stod(network_params);
        graph = new WattsStrogatz(lp_names, k, beta);

    } else if (network_type == "Barabasi-Albert") { // If the choice is Barabasi-Albert
        size_t pos = network_params.find(delimiter);
        std::string token = network_params.substr(0, pos);
        unsigned int m = (unsigned int) std::stoul(token);
        network_params.erase(0, pos + delimiter.length());
        double a = std::stod(network_params);
        graph = new BarabasiAlbert(lp_names, m, a);

    } else { // Invalid choice
        std::cerr << "Invalid choice of network." << std::endl;
        return 0;
    }

    /* Fetch the adjacency list for each node */
    for (auto& lp : lps) {
        lp.adjacency_list_ = graph->adjacencyList(lp.name_);
        assert(lp.adjacency_list_.size());
    }
    delete graph;

    /* Simulate the synthetic model */
    synthetic_sim.simulate(lp_pointers);

    return 0;
}

