// main.cpp
// g++ main.cpp
#include <climits>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <cassert>

#include <cstdio>
#include <cctype>
#include <termios.h>

#include <unistd.h>
#include <ctime>

#define ESC_CODE 27

//#define PRINT(A) std::cout << #A << " = [" << (A) << "]\n";

#include "qwerty.hpp" // like "ncurses.h"

bool root_user = false;

int run_as_shell();
int run_as_info();
int run_as_normall();

namespace {


void char2vector(std::vector<std::string> &v, int c, char **a) {
	for (int i = 0; i < c; i++) {
		auto s = std::string(a[i]);
		v.push_back(s);
	}
}

void char2vector(std::vector<std::string> &v, char **a) {
	for (int i = 0;; i++) {
		if (a[i] == nullptr)
			return;
		auto s = std::string(a[i]);
		v.push_back(s);
	}
}

std::vector<std::string> char2vector(int c, char **a) {
	std::vector<std::string> v;

	for (int i = 0; i < c; i++) {
		auto s = std::string(a[i]);
		v.push_back(s);
	}
	return v;
}

std::vector<std::string> char2vector(char **a) {
	std::vector<std::string> v;
	for (int i = 0;; i++) {
		if (a[i] == nullptr)
			return v;
		auto s = std::string(a[i]);
		v.push_back(s);
	}
	return v;
}
} // end of namespace


int main(int argc, char **argv, char **envp) {

// set global flag: root_user
	if (std::string(getenv("USER")) == "root")
		::root_user = true;

	std::vector<std::string> v_args = char2vector(argc, argv);
	std::vector<std::string> v_envp = char2vector(envp);

	// if argument #1 = "--shell"
	if (argc >= 2 && v_args.at(1) == "--shell")
		return (run_as_shell());

// if argument #1 = "info-escape-sequences"
	if (argc >= 2 && v_args.at(1) == "info-escape-sequences")
		return (run_as_info());

/*
	// if argument #1 or #2 = "-x"
	bool arg_x_f = false;
	if (argc >= 2 && v_args.at(1) == "-x") arg_x_f = true;
	if (argc >= 3 && v_args.at(2) == "-x") arg_x_f = true;
*/

	return (run_as_normall());
}

int run_as_shell() {
	Qwerty q { };
	q.cls();

	{
		std::string s = "";
		if (::root_user)
			s = "<ESC> - exit , The Best Linux Shell 2022 (root)";
		else
			s = "The Best Linux Shell * 2022 * <ESC> - exit ";
		q.print_as_teletype(s);
	}

	next: while (!q.kbdhit()) {
		//sleeping for ? milliseconds
		usleep(1);
		// std::string s = ">";
		// q.print_as_teletype(s);
	}
	if (q.ch == ESC_CODE && q.esc_record == "")
		return 0;
	if (q.ch == ESC_CODE && q.esc_record != "")
		goto esc_sequences;
	{
		//std::string s = "";
		//s.push_back(q.ch);
		q.print_as_teletype();
	}
	goto next;
	esc_sequences: ;
	goto next;

}

int run_as_info() {
	{
		std::vector<std::string> v_esc;
		Qwerty q { };
		q.cls();

		while (true) {
			while (!q.kbdhit()) {
				std::string s = " -> ";
				s += q.esc_record;
				s += " <- ";
				s += std::to_string(q.col_limit);
				s += ":";
				s += std::to_string(q.row_limit);
				q.print_aligh(s);
			}
			if (q.ch != ESC_CODE)
				continue;
			if (q.esc_record == "") { // empty string = press only ESC
				for (auto const &s : v_esc) {
					std::string cmd;
					cmd = "echo '";
					cmd += s;
					cmd += "' >all_esc_sequences.list.txt";
					std::system(cmd.c_str());
				}
				// if pressed <ESC> then exit...
				return 0;
			}
			v_esc.push_back(q.esc_record);
		}

	}
}

int run_as_normall() {

	Qwerty q { };
	q.cls();

	int last_input = 0;
	int i = 0;

	// clear
	std::string full_space_line = "";
	for (int i = 1; i < q.col_limit; i++)
		full_space_line.push_back(' ');

	std::string s = "";
	auto old_sz = s.size();

	clock_t start, end, diff;

	next: start = clock();

	while (!q.kbdhit()) {

		s = "";

		{
			if (last_input == 0) {
				s += " Press <ESC> to exit. ";
			}
		}

		{
			if (last_input == 49) {
				s += "size:";
				s += std::to_string(q.col_limit);
				s += "/";
				s += std::to_string(q.row_limit);
				s += "(cols/rows) \n";
			}
		}

		{
			if (last_input == 50) {
				s += "CHAR_BIT = ";
				int i_ = CHAR_BIT;
				auto ss = std::to_string(i_);
				s += ss;
				s += " ";
				s += "__DATE__ ";
				s += __DATE__;
				s += " ";
			}
		}

		usleep(10);

		s += std::to_string(i++);
		s += " ";

		if (last_input == ESC_CODE) {
			s += " esc + ";
			s += q.esc_record;
			s += " ";
		} else {
			s += " dec:";
			s += std::to_string(last_input);
			s += " ";

			if (last_input > 32)
				s.push_back(last_input);
			else
				s += " NoPrintAble ";
		}

		if (s.size() != old_sz) {
			q.print_aligh(full_space_line, -1);
			q.print_aligh(full_space_line, 0);
			q.print_aligh(full_space_line, 1);
		}
		old_sz = s.size();
		q.print_aligh(s);

	}
	last_input = q.ch;

	end = clock();
	diff = end - start;

	q.cursor_push();
	q.cursor(1, 1);
	printf("Elapsed %lg seconds.       ", (double)diff / CLOCKS_PER_SEC);
	q.cursor_pop();

	if (q.ch != ESC_CODE)
		goto next;

	if (q.esc_record == "") { // empty string = press only ESC
		// if pressed <ESC> then exit...
		q.cls();
		return 0;
	}
	goto next;
}

//EOF
