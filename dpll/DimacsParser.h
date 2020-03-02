//
// Dimacs parser.
//
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "common.h"

#ifndef DPLL_DIMACSPARSER_H
#define DPLL_DIMACSPARSER_H


class DimacsParser {
public:
    /**
     * Parse a dimacs file.
     * @param file_name dimacs file name
     * @return a parsed formula (if succeeds)
     */
    static formula parse(const std::string &file_name) {
        std::ifstream fin(file_name);
        if (!fin) {
            std::cerr << "file not found: " << file_name << "'" << std::endl;
            std::exit(1);
        }

        while (!fin.eof()) {
            char ch;
            fin >> ch;

            if (ch == 'c') { // c-line: comment
                char buf[1024];
                fin.getline(buf, 1024);
            } else if (ch == 'p') { // p-line: clauses will begin
                char buf[1024];
                fin.getline(buf, 1024);
                break;
            } else { // unexpected line
                std::cerr << "parse error at char '" << ch << "'" << std::endl;
                std::exit(1);
            }
        }

        // clauses begin
        formula phi;
        while (!fin.eof()) {
            clause c;
            while (!fin.eof()) {
                int v;
                fin >> v;
                if (v == 0 && !c.empty()) {
                    phi.push_back(c);
                    break;
                }
                c.push_back(v);
            }
        }

        return phi;
    }
};


#endif //DPLL_DIMACSPARSER_H
