// qwerty.hpp

#pragma once

template <typename T>
struct Cursor {
	T row;
	T col;
	Cursor() { row = 1; col = 1; } 
	Cursor(const T y, const T x) {
		row = y;
		col = x;
	}
	~Cursor() {}
};

class Qwerty {
	int row;
	int col;

public:
	int row_limit;
	int col_limit;

private:
	std::vector<Cursor<int>> *v_stack_cursor;
	termios o;

public:
	std::string esc_record;
//	bool esc_flag;
	int ch;

	Qwerty();
	~Qwerty();
	void esc(const std::string); 
	void cls();	
	void cursor();
	void cursor_home(); // reset cursor to up (row = 1 col = 1)
	void cursor(const int, const int);
    void print_aligh(const std::string);
    void print_aligh(const std::string, int);
	void cursor_push();
	void cursor_pop();	
	bool kbdhit();
	void print_as_teletype();
	void print_as_teletype(const std::string s);
private:
	void set_keypress();
	void reset_keypress();
	void move_cursor_forward();
	void page_up();
};

//EOF

