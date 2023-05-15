#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <deque>
#include <algorithm>
#include <string>
#include <map>
#include "scanner.hpp"

using std::string,
      std::map;

class Operation {
    public:
        Operation(Tokens tok, int arg) : tok(tok), intarg(arg) {}
        Operation(Tokens tok, string arg) : tok(tok), strarg(arg) {}
        bool is_alpha() {return strarg != "";}
        Tokens tok;
        int intarg = -1;
        string strarg = "";
};

class Processor {
    public:
        Processor(Scanner *sc) {
            scan = sc;
            opcode[Tokens::ADD] = 100;
            opcode[Tokens::SUB] = 200;
            opcode[Tokens::STA] = 300;
            opcode[Tokens::LDA] = 500;
            opcode[Tokens::BRA] = 600;
            opcode[Tokens::BRZ] = 700;
            opcode[Tokens::BRP] = 800;
        }

        void instr_cycle() {
            // process file
            store_info();
            // get all the data
            proc_ops();

            while (true) {
                // fetch step
                int instr = mailboxes[prog_c++];

                // decode step
                int foreinstr = instr / 100;
                int backinstr = instr % 100;

                // execute step
                int arg;
                switch(foreinstr) {
                    case 0:
                        if (instr == 0) return;
                        else proc_err("invalid command");
                    case 1:
                        arg = mailboxes[backinstr];
                        accum += arg;
                        Processor::log("added argument at address " + std::to_string(backinstr) + " to accum");
                        break;
                    case 2:
                        arg = mailboxes[backinstr];
                        accum += 999 - arg + 1;
                        Processor::log("subtracted argument at address " + std::to_string(backinstr) + " from accum");
                        break;
                    case 3:
                        mailboxes[backinstr] = accum;
                        Processor::log("stored accum into address " + std::to_string(backinstr));
                        break;
                    case 5:
                        accum = mailboxes[backinstr];
                        Processor::log("loaded address " + std::to_string(backinstr) + " into accum");
                        break;
                    case 6:
                        prog_c = backinstr;
                        Processor::log("jumped to address " + std::to_string(backinstr));
                        break;
                    case 7:
                        Processor::log("attempt conditional jumped to address");
                        if (accum == 0) {
                            prog_c = backinstr;
                            Processor::log("jump to address " + std::to_string(backinstr) + " passed");
                        }
                        break;
                    case 8:
                        Processor::log("attempt conditional jumped to address");
                        if (accum >= 0 && accum < 100) {
                            prog_c = backinstr;
                            Processor::log("jump to address " + std::to_string(backinstr) + " passed");
                        }
                        break;
                    default:
                        if (instr == 901) {
                            Processor::log("reading inbox");
                            std::cin >> inbox;
                            inbox %= 1000;
                            accum = inbox;
                        }
                        else
                        if (instr == 902) {
                            Processor::log("writing to outbox");
                            outbox = accum;
                        }
                }
                if (accum / 1000 > 0)
                    accum = accum % 1000;
                if (accum / 100 > 0)
                    accum = (accum % 100) + 900;
                displayRegs();
            }
        }

        void proc_ops() {
            for (auto x : oplist) {
                int init = opcode[x.tok];
                int g_int = x.intarg;
                string g_str = x.strarg;
                Tokens tok = x.tok;
                mailboxes.push_back(
                    (tok == Tokens::DAT || tok == Tokens::INP || tok == Tokens::OUT || tok == Tokens::HLT) 
                        ? g_int 
                        : (x.is_alpha()) 
                            ? init + getlabeladdr(g_str) 
                            : init + g_int
                );
            }
        }

        void store_info() {
            int instr = 0;
            int state = 0;
            vector<Tokens> tmp;
            int mailsize = 0;
            while (true) {
                int tok = scan->lex();
                switch(state) {
                    case 0: {
                        switch(tok) {
                            case Tokens::LBL: {
                                addlabel(mailsize);
                                break;
                            }
                            case Tokens::END: return;
                            case Tokens::BRK: break;
                            case Tokens::NUM: proc_err("illegal number found");
                            case Tokens::INP: {
                                oplist.push_back(Operation(Tokens::INP, 901));
                                state = 1;
                                break;
                            }
                            case Tokens::OUT: {
                                oplist.push_back(Operation(Tokens::INP, 902));
                                state = 1;
                                break;
                            }
                            case Tokens::HLT: {
                                oplist.push_back(Operation(Tokens::INP, 0));
                                state = 1;
                                break;
                            }
                            case Tokens::DAT: {
                                tok = scan->lex();
                                if (tok == Tokens::NUM) {
                                    auto k = atoi(scan->getlexeme().c_str());
                                    if (k > 99) k %= 100;
                                    oplist.push_back(Operation(Tokens::DAT, k));
                                }
                                else {
                                    oplist.push_back(Operation(Tokens::DAT, 0));
                                    scan->unget((Tokens)tok);
                                }
                                state = 1;
                                break;
                            }
                            default: {
                                auto old_tok = tok;
                                tok = scan->lex();
                                if (tok == Tokens::LBL)
                                    oplist.push_back(Operation((Tokens)old_tok, scan->getlexeme()));
                                else
                                if (tok == Tokens::NUM) {
                                    auto k = atoi(scan->getlexeme().c_str());
                                    if (k > 99) k %= 100;
                                    oplist.push_back(Operation((Tokens)old_tok, k));
                                }
                                else proc_err("illegal opcode argument");
                                state = 1;
                                break;
                            }
                        }
                        break;
                    }
                    case 1: {
                        switch(tok) {
                            case Tokens::END: return;
                            case Tokens::BRK: {
                                ++mailsize;
                                state = 0;
                                break;
                            }
                            default: proc_err("illegal token \"" + string(toktostr((Tokens)tok)));
                        }
                        break;
                    }
                }
            }
        }
        
        void print_items() {
            std::cout << "printing items...\n";
            for (int i = 0; i < mailboxes.size(); ++i) {
                std::cout << "mailbox [" << i
                          << "] : " << mailboxes[i]
                          << std::endl;
            }
        }

    private:
        void proc_err(string err) {
            std::cerr << err
                      << " at line "
                      << scan->getlineno()
                      << std::endl;
            exit(-1);
        }
        void log(string str) {
            std::cout << "log: "
                      << str << std::endl;
        }
        void displayRegs() {
            std::cout << "inbox: "
                      << inbox << std::endl;
            std::cout << "outbox: "
                      << outbox << std::endl;
            std::cout << "accum: "
                      << accum << std::endl
                      << std::endl;
        }
        int getlabeladdr(string str) {
            if (labels.find(str) == labels.end())
                proc_err("non-existent label used");
            return labels[str];
        }
        void addlabel(int k) {
            if (labels.find(scan->getlexeme()) != labels.end())
                proc_err("label repeated");
            labels[scan->getlexeme()] = k;
        }

        Scanner *scan;
        vector<int> mailboxes;
        map<Tokens, int> opcode;
        map<string, int> labels;
        vector<Operation> oplist;
        int accum = 0;
        int prog_c = 0;
        int inbox = 0, outbox = 0;
};

#endif