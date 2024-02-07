#include <fstream>
#include <iostream>
#include <string>
#include <vector>

inline std::vector<std::vector<int>> power_set(const std::vector<int>& elts) {
    if (elts.empty()) {
        return std::vector<std::vector<int>>(
            1,                    // vector contains 1 element which is...
            std::vector<int>());  // ...empty vector of ints
    }

    else {
        std::vector<std::vector<int>> smaller =
            power_set(std::vector<int>(elts.begin() + 1, elts.end()));
        int elt = elts[0];  // in Python elt is a list (of int)
                            //      withElt = []
        std::vector<std::vector<int>> withElt;
        //      for s in smaller:
        for (const std::vector<int>& s : smaller) {
            //          withElt.append(s + elt)
            withElt.push_back(s);
            withElt.back().push_back(elt);
        }
        //      allofthem = smaller + withElt
        std::vector<std::vector<int>> allofthem(smaller);
        allofthem.insert(allofthem.end(), withElt.begin(), withElt.end());
        //      return allofthem
        return allofthem;
    }
}

inline void append_to_file(std::string const& path,
                           std::string const& to_append) {
    std::ofstream o_file;
    o_file.open(path, std::ios::in | std::ios::out | std::ios::ate);
    o_file << to_append;
    o_file.close();
}