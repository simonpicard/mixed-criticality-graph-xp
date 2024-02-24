#ifndef JOB_H
#define JOB_H

#include "Types.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#pragma once

class Job {
   public:
    Job() = default;

    Job(int T, int D, Criticality X, std::vector<int> const& C, int p = 0) : T(T), D(D), X(X), C(std::move(C)), p(p) {
        initialize();
    };

    explicit Job(Job* other)
        : T(other->T),
          D(other->D),
          X(other->X),
          C(other->C),
          p(other->p),
          rct(other->rct),
          nat(other->nat),
          utilisation(other->utilisation){};
    Job(Job const& other) = default;
    ~Job() = default;

    int get_T() const { return T; };
    int get_D() const { return D; };
    int get_X() const { return X; };
    // TODO we use L_i in the paper; maybe we change that to avoid confusion?
    std::vector<int> get_C() const { return C; };
    int get_C(Criticality criticality) const { return C[criticality - 1]; };
    int get_p() const { return p; };

    int get_rct() const { return rct; };
    int get_nat() const { return nat; };

    int get_ttd() const { return nat - (T - D); };
    float get_ttvd(float discount_factor) const;
    int get_laxity() const { return get_ttd() - rct; };
    int get_worst_laxity(Criticality current_crit) const { return get_ttd() - rct - (C[1] - C[current_crit - 1]); };

    bool is_active() const { return rct > 0; };
    bool is_eligible(int crit) const { return rct == 0 and nat == 0 and X >= crit; };
    bool is_implicitly_completed(int crit) const { return rct == 0 and C[crit - 1] == C[X - 1]; };
    bool is_deadline_miss() const { return rct > 0 and get_ttd() <= 0; };
    bool is_discarded(int crit) const { return X < crit; };

    void execute(bool run);
    void terminate();
    void request(int crit);
    void critic(int current_crit, int next_crit, bool is_triggering);

    float get_utilisation_at_level(int at_level) const { return utilisation[at_level - 1]; };

    std::string str() const;
    std::string str_task() const;
    std::string dot_node() const;
    void repr() const;

    uint64_t get_hash() const;
    uint64_t get_hash_factor() const;
    uint64_t get_hash_idle() const;

   private:
    int T;
    int D;
    Criticality X;
    std::vector<int> C;
    int p;  // priority for FJP/FTP

    int rct;
    int nat;

    std::vector<float> utilisation =
        std::vector<float>{compute_utilisation_at_level(1), compute_utilisation_at_level(2)};

    float compute_utilisation_at_level(int at_level) const { return float(C[at_level - 1]) / float(T); }

    void initialize();
};

#endif
