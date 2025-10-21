#ifndef SCREEN_HANDLER_HPP
#define SCREEN_HANDLER_HPP

#include <iostream>
#include <utility> // for std::pair

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

static int centre(int margin, int panelWidth, int contentWidth) {
    return margin + (panelWidth - contentWidth) / 2;
}

static int rightAlign(int margin, int panelWidth, int contentWidth) {
    return margin + panelWidth - contentWidth;
}

static int leftAlign(int margin, int panelWidth, int contentWidth) {
    return margin;
}

enum AlignMode {
    LEFT,
    CENTER,
    RIGHT
};

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
    AlignMode titleAlign;
	bool titleInverted;

    std::vector<std::string> items;
    AlignMode itemsAlign;
    std::string itemsPrefix;

    // Note that you may only have items_right if itemsAlign is LEFT
    std::vector<std::string> items_right;
};

std::string bytesPretty(size_t bytes) {
    const char* sizes[] = { "B", "KB", "MB", "GB", "TB" };
    int order = 0;
    double dblBytes = static_cast<double>(bytes);
    while (dblBytes >= 1024 && order < 4) {
        order++;
        dblBytes = dblBytes / 1024;
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
        int titleX;
        switch (panel.titleAlign) {
        case LEFT:
            titleX = leftAlign(panel.startX, panel.width, panel.title.length());
            break;
        case CENTER:
            titleX = centre(panel.startX, panel.width, panel.title.length());
            break;
        case RIGHT:
            titleX = rightAlign(panel.startX, panel.width, panel.title.length());
            break;
        }

        if (panel.titleInverted) {
            invCol();
		}
        
		// Draw spaces before title

        moveCursor(titleX, panel.startY);
        std::cout << panel.title;
        // Draw items
        for (size_t i = 0; i < panel.items.size() && i < static_cast<size_t>(panel.height - 1); i++) {
            int itemX;
            switch (panel.itemsAlign) {
            case LEFT:
                itemX = leftAlign(panel.startX, panel.width, panel.itemsPrefix.length() + panel.items[i].length());
                break;
            case CENTER:
                itemX = centre(panel.startX, panel.width, panel.itemsPrefix.length() + panel.items[i].length());
                break;
            case RIGHT:
                itemX = rightAlign(panel.startX, panel.width, panel.itemsPrefix.length() + panel.items[i].length());
                break;
            }
            moveCursor(itemX, panel.startY + 1 + i);
            std::cout << panel.itemsPrefix << panel.items[i];
            // If there is a right-aligned item
            if (!panel.items_right.empty() && i < panel.items_right.size()) {
                int rightItemX = rightAlign(panel.startX, panel.width, panel.items_right[i].length());
                moveCursor(rightItemX, panel.startY + 1 + i);
                std::cout << panel.items_right[i];
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