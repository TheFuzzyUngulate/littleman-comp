#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <deque>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <map>

using std::string,
      std::fstream,
      std::vector,
      std::map;

enum Tokens {
	END, BRK,
	LBL, NUM,
	ADD, SUB,
	STA, LDA, LDI,
	BRA, BRZ, BRP,
	INP, OUT,
	HLT, 
	DAT
};

inline const char* toktostr(int ch) {
	switch(ch) {
		case Tokens::END: return "eof";
		case Tokens::BRK: return "break";
		case Tokens::LBL: return "label";
		case Tokens::NUM: return "number";
		case Tokens::ADD: return "add";
		case Tokens::SUB: return "sub";
		case Tokens::STA: return "store-addr";
		case Tokens::LDA: return "load-addr";
		case Tokens::BRA: return "branch";
		case Tokens::BRZ: return "branch-if-zero";
		case Tokens::BRP: return "branch-if-positive";
		case Tokens::INP: return "get-input";
		case Tokens::OUT: return "output-val";
		case Tokens::HLT: return "quit";
		case Tokens::DAT: return "store-data";
		default: {
			std::cerr << "cannot find token\n";
			exit(-1);
		}
	}
}

class Scanner {
	public:
		Scanner(fstream *fptr) {
			file = fptr;
		}

		int lex() {
			while(1) {
				char ch;
				
				if (!unget_tok_list.empty()) {
					ch = unget_tok_list.back();
					unget_tok_list.pop_back();
					return ch;
				}

				do ch = get();
				while (isspace(ch) && ch != '\n');
				lexeme.clear();

                if (ch != '\n')
                    still_space = false;

				if (ch == 0)
					return Tokens::END;
				else
				if (ch == '\n') {
                    lineno++;
                    if (!still_space)
                        ++mail_count;
                    still_space = true;
					return Tokens::BRK;
                }
				else
				if (ch == '/') {
					ch = get();
					if (ch == '/') {
						do ch = get();
						while (ch != '\n' && ch != 0);
						unget(ch);
					}
					else scan_err("illegal character");
				}
				else
				if (isdigit(ch)) {
					do {
						lexeme += ch;
						ch = get();
					} while (isdigit(ch));
					unget(ch);
					return Tokens::NUM;
				}
				else
				if (isalpha(ch)) {
					do {
						lexeme += tolower(ch);
						ch = get();
					} while (isalpha(ch));
					unget(ch);
					
					if (lexeme == "add")
						return Tokens::ADD;
					if (lexeme == "sub")
						return Tokens::SUB;
					if (lexeme == "sta")
						return Tokens::STA;
					if (lexeme == "ldi")
						return Tokens::LDI;
					if (lexeme == "lda")
						return Tokens::LDA;
					if (lexeme == "bra")
						return Tokens::BRA;
					if (lexeme == "brz")
						return Tokens::BRZ;
					if (lexeme == "brp")
						return Tokens::BRP;
					if (lexeme == "inp")
						return Tokens::INP;
					if (lexeme == "out")
						return Tokens::OUT;
					if (lexeme == "hlt")
						return Tokens::HLT;
					if (lexeme == "dat")
						return Tokens::DAT;
                    return Tokens::LBL;
				}
				else scan_err("illegal character \"" + string(ch, 1) + "\"");
			}
		}

        int transl_label(string str) {
            if (labels.find(str) == labels.end())
                scan_err("non-existent label used");
            std::cout << str << ", " << labels[str] << std::endl;
            return labels[str];
        }

        int getlineno() {return lineno;}
        string getlexeme() {return lexeme;}
        void setlabels() {
            if (labels.find(lexeme) != labels.end())
                scan_err("repeated label");
            labels[lexeme] = lineno;
        }
        int getmailno() {return mail_count;}
		void unget(int ch) {
			unget_list.push_back(ch);
		}
		void unget(Tokens tok) {
			unget_tok_list.push_back(tok);
		}

    private:
		int get() {
			if (!unget_list.empty()) {
				char ch = unget_list.back();
				unget_list.pop_back();
				return ch;
			}
			if (file->eof()) 
				return 0;
			else {
				char res = 0;
				file->read(&res, 1);
				return res;
			}
		}

        void scan_err(string err) {
            std::cout << "scanner error: "
                      << err << " at line "
                      << lineno << std::endl;
            exit(-1);
        }

		fstream *file;
		string lexeme;
        int lineno = 1;
        int mail_count = 0;
        bool still_space = false;
		vector<int> unget_list;
		vector<Tokens> unget_tok_list;
        map<string, int> labels;
};

#endif