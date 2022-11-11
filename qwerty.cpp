// qwerty.cpp

#include <iostream>
#include <cstdlib>
#include <vector>
#include <cassert>
//#include <unistd.h>
#include <cstdio>
#include <cctype>
#include <termios.h>

//#include <chrono>

#include <unistd.h>
#include <ctime>

//#include "qwerty.hpp"

//=============================[get_max_rows_cols_from_terminal]

//#include <cstdlib>
#include <fstream>
//#include <iostream>

#include <sys/ioctl.h>
//#include <stdio.h>
//#include <unistd.h>

#include "qwerty.hpp"
Cursor<int> off_get_max_rows_cols_from_terminal();
// variant #1
Cursor<int> get_max_rows_cols_from_terminal() {

#define TMPFILENAME "/tmp/asd123"

	const std::string cmd_get_rows_cols =
			"stty -F /dev/tty -a | grep rows > " TMPFILENAME;

	std::system(cmd_get_rows_cols.c_str());
	std::string s;
	std::ifstream f(TMPFILENAME); // .rdbuf();
	std::getline(f, s);
	std::system("rm " TMPFILENAME);
	std::vector<std::string> v { };
	std::string subs = "";
	for (const auto &ch : s) {
		if (ch == ' ' || ch == ';') {
			v.push_back(subs);
			subs = "";
			continue;
		}
		subs.push_back(ch);
	}

	bool f_rows = false;
	bool f_cols = false;

	int y, x;

	for (const auto &next_word : v) {

		if (f_rows) {
			f_rows = false;
			y = std::atoi(next_word.c_str());
		}

		if (f_cols) {
			f_cols = false;
			x = std::atoi(next_word.c_str());
			return (Cursor<int>(y, x));
		}

		if (next_word == "rows")
			f_rows = true;
		if (next_word == "columns")
			f_cols = true;
	}

	assert(false);
	return (off_get_max_rows_cols_from_terminal());
}

// variant #2
Cursor<int> off_get_max_rows_cols_from_terminal() {

	/* copy&paste from web:
	 https://www.linuxquestions.org/questions/programming-9/\
	get-width-height-of-a-terminal-window-in-c-810739/
	 */

	int cols = 80;
	int lines = 24;

#ifdef TIOCGSIZE
    struct ttysize ts;
    ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
    cols = ts.ts_cols;
    lines = ts.ts_lines;
#elif defined(TIOCGWINSZ)
	struct winsize ts;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
	cols = ts.ws_col;
	lines = ts.ws_row;
#endif /* TIOCGSIZE */

//    printf("Terminal is %dx%d\n", cols, lines);
	return (Cursor<int>(lines, cols));
}

//=============================[get_max_rows_cols_from_terminal]

void os_print(const std::string s) {
	std::cout << s << std::flush;
}

void os_print(const int ch) {
	std::string s;
	s.push_back(ch);
	std::cout << s << std::flush;
}

// --- like ncurses ---

void Qwerty::set_keypress() {
	termios n;
	tcgetattr(0, &n);
	o = n;
	n.c_lflag &= (~ICANON);
	n.c_lflag &= (~ECHO);

	n.c_cc[VTIME] = 0; // work only all 0
	n.c_cc[VMIN] = 0;
	tcsetattr(fileno(stdin), TCSANOW, &n);
}

void Qwerty::reset_keypress() {
	tcsetattr(fileno(stdin), TCSANOW, &o);
}

Qwerty::Qwerty() {
	cursor_home();
	Cursor<int> cur = get_max_rows_cols_from_terminal();
	row_limit = cur.row;
	col_limit = cur.col;
	v_stack_cursor = new std::vector<Cursor<int>>;
	set_keypress();
	esc_record = "";
//	esc_flag = false;
	ch = ' ';

}

Qwerty::~Qwerty() {
	cursor_home();
	//printf(" ~destructor() \n");
	delete v_stack_cursor;
	reset_keypress();
}

void Qwerty::cls() {
	esc("[2J");
}

void Qwerty::cursor_home() {
	esc("[1;1f");
	row = 1;
	col = 1;
}

void Qwerty::cursor() {	  //esc("[1;1f"); }
	cursor(row, col);
}
void Qwerty::cursor(const int row_, const int col_) {

	assert(row_ <= row_limit);
	assert(col_ <= col_limit);

	row = row_;
	col = col_;

	std::string s = "[";
	int y = row_;
	int x = col_;

	std::string pos_row = std::to_string(y);
	std::string pos_col = std::to_string(x);
	s += pos_row;
	s += ";";
	s += pos_col;
	s += "f";
	esc(s);
}

void Qwerty::esc(const std::string s) {
	std::string e = "\E";
	e += s;
	printf("%s", e.c_str());
}

void Qwerty::print_aligh(const std::string s) {

	std::string simple_one_line = "";
	int i = 0;
	for (const auto &ch: s) {
		if (ch == '\n') i++;
	}
	if (i > 1) {
		int new_i = i/2;
		i = 0 - new_i;
	}
	else i = -1;

	for (const auto &ch : s) {
		if (ch == '\n') {
			print_aligh(simple_one_line, i++);
			simple_one_line = "";
			continue;
		}
		simple_one_line.push_back(ch);
	}
	print_aligh(simple_one_line, i);
}

void Qwerty::print_aligh(const std::string s, int row_delta) {

	cursor_push();
	std::string msg = s;

	auto sz = msg.size();
	int delta = sz / 2;

	assert(col_limit > (int )sz);

	int y = row_limit / 2;
	y += row_delta;
	int x = col_limit / 2;

	x -= delta;

	cursor(y, x);
	os_print(msg);
	// printf("%s", msg.c_str());
	// char message[20] = {0x20, 'r', 0};
	// write(1, message, 2);
	cursor_pop();
	cursor();
}

void Qwerty::cursor_push() {
	v_stack_cursor->push_back(Cursor<int>(row, col));
}

void Qwerty::cursor_pop() {
	auto *v = v_stack_cursor;
	assert(v->size() != 0);

	Cursor<int> cr = { };
	cr = v->at(v->size() - 1);
	v->erase(v->end());
	row = cr.row;
	col = cr.col;

}

bool Qwerty::kbdhit() {
	if (read(0, &ch, 1) == 0)
		return false;
	if (ch != 27)
		return true;
	char buf[1024];
	int sz = 0;
	sz = read(0, &buf, 1024);
	if (sz == 0) {
		esc_record = "";
		return true;
	}
	std::string s = "";
	for (int i = 0; i < sz; i++) {
		if (buf[i] >= 32)
			s.push_back(buf[i]);
	}
	esc_record = s;
	return true;
}

void Qwerty::print_as_teletype(const std::string s) {

	if (s == "\n") {
		page_up();
		return;
	}

	cursor();

	for (const auto &ch_ : s) {
		ch = ch_;
		print_as_teletype();
	}

}

void Qwerty::move_cursor_forward() {
	if (col != col_limit)
		col++;
	else {
		col = 1;
		if (row != row_limit)
			row++;
		else {
			page_up();
			return;
		}
	}
	cursor();
}

void Qwerty::page_up() {
	os_print("\n");
	if (row != row_limit)
		row++;
	col = 1;
	cursor();
}

void Qwerty::print_as_teletype() {

	if (ch == '\n') {
		page_up();
		return;
	}
	cursor();
	os_print(ch);
	move_cursor_forward();
}

//EOF

