#include "Job.h"

void Job::initialize() {
    nat = 0;
    rct = 0;
}

float Job::get_ttvd(float discount_factor) const {
    if (X == 1) return (float)get_ttd();
    return 1.0 * nat - (1.0 * T - D * discount_factor);
};

void Job::execute(bool run) {
    if (run) {
        rct--;
    }
    if (is_active()) {
        nat--;
    } else {
        nat = std::max(nat - 1, 0);
    }
}

void Job::terminate() { rct = 0; }

void Job::request(int crit) {
    rct = C[crit - 1];
    nat = T;
}

void Job::critic(int current_crit, int next_crit, bool is_triggering) {
    if (current_crit == next_crit) {
        return;
    }
    if (X < next_crit) {
        terminate();
    } else {
        if (is_active() or is_triggering) {
            rct = rct + C[next_crit - 1] - C[current_crit - 1];
        }
    }
}

void Job::repr() const { std::cout << str() << std::endl; }

std::string Job::str() const {
    std::stringstream ss;
    ss << "(" << rct << ", " << nat << ")";
    return ss.str();
}

std::string Job::dot_node() const {
    std::stringstream ss;
    // ss << "(" << rct << ", " << nat << ")";
    ss << rct << "," << nat;
    return ss.str();
}

uint64_t Job::get_hash() const {
    uint64_t hash = rct;
    uint64_t factor = C[1] + 1;

    hash += nat * factor;
    return hash;
}

uint64_t Job::get_hash_factor() const {
    uint64_t factor = C[1] + 1;
    factor = factor * (T + 1);
    return factor;
}

uint64_t Job::get_hash_idle() const {
    uint64_t hash = rct;
    uint64_t factor = C[1] + 1;

    if (rct > 0) hash += nat * factor;

    return hash;
}