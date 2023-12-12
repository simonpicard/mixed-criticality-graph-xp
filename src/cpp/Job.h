#ifndef JOB_H
#define JOB_H
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#pragma once

class Job {
   public:
    Job() = default;

    Job(int T_, int D_, int X_, std::vector<int> const& C_)
        : T(T_), D(D_), X(X_), C(std::move(C_)), p(0){};
    Job(int T_, int D_, int X_, std::vector<int> const& C_, int p_)
        : T(T_), D(D_), X(X_), C(std::move(C_)), p(p_){};

    explicit Job(Job* other)
        : T(other->T),
          D(other->D),
          X(other->X),
          C(other->C),
          rct(other->rct),
          nat(other->nat),
          p(other->p){};
    Job(Job const& other) = default;
    ~Job() = default;

    int get_T() const { return T; };
    int get_D() const { return D; };
    int get_X() const { return X; };
    std::vector<int> get_C() const { return C; };
    int get_p() const { return p; };

    int get_rct() const { return rct; };
    int get_nat() const { return nat; };

    int get_ttd() const { return nat - T + D; };
    int get_laxity() const { return get_ttd() - rct; };

    bool is_active() const { return rct > 0; };
    bool is_eligible(int crit) const {
        return rct == 0 and nat == 0 and not X >= crit;
    };
    bool is_implicitly_completed(int crit) const {
        return rct == 0 and C[crit - 1] == C[X - 1];
    };
    bool is_deadline_miss() const { return rct > 0 and get_ttd() <= 0; };
    bool is_discarded(int crit) const { return X < crit; };

    void execute(bool run);
    void terminate() { rct = 0; }
    void request(int crit);
    void critic(int current_crit, int next_crit);

    std::string str() const;
    std::string dot_node() const;
    void repr() const;

   private:
    int T;
    int D;
    int X;
    std::vector<int> C;
    int p;  // priority for FJP/FTP

    int rct;
    int nat;

    void initialize();
};

#endif