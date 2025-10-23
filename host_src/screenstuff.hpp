#ifndef SCREEN_HANDLER_HPP
#define SCREEN_HANDLER_HPP

#include <iostream>
#include <utility> // for std::pair
#include <string>

#ifdef _WIN32
#include <windows.h>
const int ORDER_SIZE = 1024; // on windows 1 KB is 1024 bytes
#else
#include <sys/ioctl.h>
#include <unistd.h>
const int ORDER_SIZE = 1000; // on unix-like systems 1 KB is 1000 bytes
#endif

using std::string;

enum AlignMode {
    LEFT,
    CENTER,
    RIGHT
};

static int centre(int margin, int panelWidth, int contentWidth) {
    return margin + (panelWidth - contentWidth) / 2;
}

static int rightAlign(int margin, int panelWidth, int contentWidth) {
    return margin + panelWidth - contentWidth;
}

static int leftAlign(int margin, int panelWidth, int contentWidth) {
    return margin;
}

static int align(int margin, int panelWidth, int contentWidth, AlignMode mode) {
    switch (mode) {
    case LEFT:
        return leftAlign(margin, panelWidth, contentWidth);
    case CENTER:
        return centre(margin, panelWidth, contentWidth);
    case RIGHT:
        return rightAlign(margin, panelWidth, contentWidth);
    default:
        return leftAlign(margin, panelWidth, contentWidth);
    }
}

enum TitleStyle {
	NORMAL, // white on black
	INVERTED, // black on white
	HOLE // spaces around title are inverted but title is normal
};

struct listPanel {
    int startX;
    int startY;
    int width;
    int height;

    std::string title;
	AlignMode titleAlign = CENTER;
	TitleStyle titleStyle = NORMAL;

    std::vector<std::string> items;
	AlignMode itemsAlign = LEFT;
	std::string itemsPrefix = " ";

    std::vector<std::string> items_right;
	AlignMode items_rightAlign = RIGHT;

	// how wide is right pane (left items get truncated)

    // give right panel items exactly n characters
    int items_right_w = 0;

    // split panel relativly
    float items_right_rel = 0;

	// split panel to always give righ col items enough space
    bool items_right_fit = true;
};

std::string bytesPretty(size_t bytes) {
    const char* sizes[] = { "B", "KB", "MB", "GB", "TB" };
    int order = 0;
    double dblBytes = static_cast<double>(bytes);
    while (dblBytes >= ORDER_SIZE && order < 4) {
        order++;
        dblBytes = dblBytes / ORDER_SIZE;
    }
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%.2f %s", dblBytes, sizes[order]);
    return std::string(buffer);
}

class ScreenHandler {
public:
    int width;
    int height;

    ScreenHandler() {
        // Switch to the alternate buffer
        std::cout << "\033[?1049h";
        // Clear screen
        std::cout << "\033[2J";
        // Move cursor to home
        std::cout << "\033[H";
        // Hide cursor
        std::cout << "\033[?25l";

        updateSize(); // get initial window dimensions
    }

    ~ScreenHandler() {
        // Show cursor again
        std::cout << "\033[?25h";
        // Return to the normal buffer
        std::cout << "\033[?1049l";
        // Reset colors / attributes
        std::cout << "\033[0m";
    }

    void updateSize() {
        auto [w, h] = getTerminalSize();
        width = w;
        height = h;
    }

    void cls() {
        // Clear screen
        std::cout << "\033[2J";
        // Move cursor to home
        std::cout << "\033[H";
	}

	// Move cursor to (x, y)
    void moveCursor(int x, int y) {
        std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H";
	}

	// black on white
    void invCol() {
        std::cout << "\033[47m" << "\033[30m";
	}

    void unCol() {
        std::cout << "\033[0m";
	}

    void renderPanel(const listPanel& panel) {
        // Draw title
		int titleX = align(panel.startX, panel.width, panel.title.length(), panel.titleAlign);
        moveCursor(panel.startX, panel.startY);

        // Draw spaces before title
        if (panel.titleStyle == INVERTED || panel.titleStyle == HOLE) {
            invCol();
            for (int i = panel.startX; i < panel.startX + panel.width; i++) {
                std::cout << " ";
			}

            if (panel.titleStyle == HOLE) {
                unCol();
			}
		}

        moveCursor(titleX, panel.startY);
        std::cout << panel.title;
        unCol();

        size_t items_maxlength = panel.width;
		size_t right_items_maxlength = 0;

        // Calculte r items width
        if (panel.items_right.size() > 0) {
        	if (panel.items_right_w) {
        		right_items_maxlength = panel.items_right_w;
        		items_maxlength = panel.width - right_items_maxlength - 1; // -1 for gap
        	} else if (panel.items_right_rel) {
        		right_items_maxlength = (float)panel.width * panel.items_right_rel;
        		items_maxlength = panel.width - right_items_maxlength - 1; // -1 for gap
        	} else if (panel.items_right_fit) {
                right_items_maxlength = 0;
        		for (std::string item: panel.items_right) {
                    right_items_maxlength = max(right_items_maxlength, (size_t)item.length());
        		}
        		items_maxlength = panel.width - right_items_maxlength - 1; // -1 for gap
        	}
        }

        int right_startX = panel.startX + (int)items_maxlength + 1; // +1 for the gap space

        // Draw items
        for (size_t i = 0; i < panel.items.size() && i < panel.height - 1; i++) {
			string item = panel.items[i];

			// Truncate item if too long
            if (item.length() + panel.itemsPrefix.length() > items_maxlength) {
				item = item.substr(0, items_maxlength - panel.itemsPrefix.length() - 3) + "...";
            }

			int itemX = align(panel.startX, items_maxlength, item.length() + panel.itemsPrefix.length(), panel.itemsAlign);
            moveCursor(itemX, panel.startY + 1 + i);
            std::cout << panel.itemsPrefix << item;

			// Draw right item if exists
            if (panel.items_right.size() > i) {
				string right_item = panel.items_right[i];
				int right_itemX = align(right_startX, (int)right_items_maxlength, right_item.length(), panel.items_rightAlign);

				// Calculate available space for right item within panel bounds
				int panel_right_edge = panel.startX + panel.width;
				int available_space = panel_right_edge - right_itemX;

				// Truncate if item doesn't fit
				if (available_space < (int)right_item.length()) {
					if (panel.items_rightAlign == RIGHT) {
						// For right-aligned items, cut from the left
						int overflow = (int)right_item.length() - available_space;
						right_item = right_item.substr(overflow);
					} else {
						// For left/center aligned, cut from the right
						right_item = right_item.substr(0, available_space);
					}
				}

				moveCursor(right_itemX, panel.startY + 1 + i);
				std::cout << right_item;
            }
        }
	}

private:
    std::pair<int, int> getTerminalSize() {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
            return { cols, rows };
        }
        return { 80, 25 };
#else
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
            return { w.ws_col, w.ws_row };
        return { 80, 25 };
#endif
    }
};

#endif // SCREEN_HANDLER_HPP
