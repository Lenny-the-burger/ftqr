#include "qrcodegen.hpp"
#include "screenstuff.hpp"
#include "networking.h"

#include <iostream>
#include <string> 

using namespace std;
using namespace qrcodegen;

using std::uint8_t;
using qrcodegen::QrCode;
using qrcodegen::QrSegment;

int main() {
	cout << "Starting ftqr..." << endl;

	ScreenHandler sh;

#pragma region QR code
	string prefix = "www.ftqr.cc/";
	string sessionId = makeNewSession();
	string text = prefix + sessionId;

	const QrCode qr = QrCode::encodeText(text.c_str(), QrCode::Ecc::LOW);

	int qrSize = qr.getSize();

	// qr code is in the middle of the right half of the screen
	int qrDisplayStartX = sh.width / 2 + (sh.width / 2 - qrSize * 2) / 2;

	// Centered vertically + 2 lines for plaintext caption
	int qrDisplayStartY = ((sh.height + 1) - qrSize) / 2 - 1;

	// Draw qr code
	for (int y = 0; y < qr.getSize(); y++) {
		sh.moveCursor(qrDisplayStartX, qrDisplayStartY + y);

		for (int x = 0; x < qr.getSize(); x++) {
			cout << (qr.getModule(x, y) ? "\033[47m" : "");
			cout << "  ";
			cout << "\033[0m";
		}
	}

	// Draw caption
	string caption = "Or open this link in a browser: " + text;
	
	// Centered below the qr 
	sh.moveCursor(sh.width / 2 + centre(0, sh.width / 2, caption.length()), qrDisplayStartY + qrSize + 1);
	cout << caption;
#pragma endregion
#pragma region bars

	// draw title centered at top
	string title = "  File Transfer Quick Response (code) - ftqr  ";

	sh.moveCursor(0, 0);
	int titleLen = title.length();

	sh.invCol();

	// draw spaces before title
	for (int i = 0; i < (sh.width - titleLen) / 2; i++) {
		cout << " ";
	}

	// reset colors
	sh.unCol();
	cout << title;
	sh.invCol();

	// draw spaces after title
	for (int i = 0; i < (sh.width - titleLen + 1) / 2; i++) {
		cout << " ";
	}

	// Draw bottom bar
	string bottomBar = " ^X to exit   ^D to detach ";
	
	// Bottom bar isnt centered, just left aligned
	sh.moveCursor(0, sh.height - 1);
	sh.invCol();
	cout << bottomBar;
	// fill rest of line with spaces
	for (int i = bottomBar.length(); i < sh.width; i++) {
		cout << " ";
	}

	sh.unCol();
#pragma endregion
#pragma region status panel

	// status panel has same height as qr code

	// Status panel on left half of screen

	int stPnlMrginX = 4;
	int stPnlMrginY = 2;

	int statusPanelStartX = stPnlMrginX;
	int statusPanelStartY = stPnlMrginY;

	int statusPanelWidth = qrDisplayStartX - stPnlMrginX * 2;
	int statusPanelHeight = sh.height - stPnlMrginY * 2;


	listPanel filesPanel;
	filesPanel.startX = statusPanelStartX;
	filesPanel.startY = statusPanelStartY + 2;
	filesPanel.width = statusPanelWidth;
	filesPanel.height = 5;

	filesPanel.title = "Published files:";
	filesPanel.titleAlign = LEFT;

	// how many files are we publishing?

	filesPanel.items = {
		"document.pdf",
		"image.png",
		"archive.zip",
		"very_long_filename_example_document_final_version.docx"
	};
	filesPanel.itemsPrefix = " -";

	vector<size_t> fileSizes = {
		234567,
		3456789,
		45678901,
		9876543219
	};

	for (int i = 0; i < filesPanel.items.size(); i++) {
		filesPanel.items_right.push_back(bytesPretty(fileSizes[i]));
	}

	sh.renderPanel(filesPanel);



	// Active connections list 
	listPanel connPanel;
	connPanel.startX = statusPanelStartX;
	connPanel.startY = filesPanel.startY + filesPanel.height + 1;
	connPanel.width = statusPanelWidth;
	connPanel.height = 6; // 5 con + title
	
	connPanel.title = "Active connections:";
	connPanel.titleAlign = LEFT;

	// fake connections (only displays user agents, not actual IPs)
	connPanel.items = {
		"[0] Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.5993.90 Safari/537.36",
		"[1] Mozilla/5.0 (Android 14; Mobile; rv:120.0) Gecko/120.0 Firefox/120.0",
		"[2] curl/8.2.1",
		"[3] Wget/1.21.3 (linux-gnu)",
	};

	vector<string> connStates = {
		"/\\", // uploading
		"\\/", // downloading
		"++", // new connection
		"--" // idle
	};

	for (int i = 0; i < connPanel.items.size(); i++) {
		connPanel.items_right.push_back(connStates[3]);
	}

	connPanel.items_right[2] = connStates[1];

	connPanel.items_right_w = 2;

	sh.renderPanel(connPanel);


	// Console log panel
	listPanel logPanel;
	logPanel.startX = statusPanelStartX;
	logPanel.startY = connPanel.startY + connPanel.height + 1;
	logPanel.width = statusPanelWidth;
	logPanel.height = statusPanelHeight - (logPanel.startY - statusPanelStartY);
	logPanel.title = " log ";
	logPanel.titleStyle = HOLE;

	logPanel.items = {
		"[09:15:23] Published 4 files",
		"[09:15:30] New connection [3] via tty",
		"[09:15:32] File 'image.png' start download by [2]",
		"[09:15:35] File 'document.pdf' start upload by [0]",
		"[09:15:40] Connection [1] disconnected",
		"[09:15:40] File 'image.png' finish download by [2]"
	};

	sh.renderPanel(logPanel);


#pragma endregion

	cout.flush();
	cin.get();
}
