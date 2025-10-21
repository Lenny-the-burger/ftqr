#include "qrcodegen.hpp"
#include "ScreenHandler.hpp"

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
	// fake (for now) 4 random alphanumeric chars session id
	string sessionId = "09rD";
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
	string statusTitle = "-- Status --";

	int stPnlMrginX = 4;
	int stPnlMrginY = 2;

	int statusPanelStartX = stPnlMrginX;
	int statusPanelStartY = stPnlMrginY;

	int statusPanelWidth = qrDisplayStartX - stPnlMrginX * 2;
	int statusPanelHeight = sh.height - stPnlMrginY * 2;

	sh.moveCursor(centre(stPnlMrginX, statusPanelWidth, statusTitle.length()), statusPanelStartY);
	cout << statusTitle;

	// how many files are we publishing?

	vector<string> fileList = {
		"document.pdf",
		"image.png",
		"archive.zip",
		"very_long_filename_example_document_final_version.docx"
	};

	vector<int> fileSizes = {
		234567,
		3456789,
		45678901,
		987654321
	};

	vector<string> fileSizePretty;
	for (int i = 0; i < fileList.size(); i++) {
		fileSizePretty.push_back(bytesPretty(fileSizes[i]));
	}

	vector<string> fileNamePretty;
	// if the filename + size is longer than status panel width - 2, truncate filename
	// file names are right aligned while sizes are left aligned
	for (int i = 0; i < fileList.size(); i++) {
		int availableWidth = statusPanelWidth - 2 - fileSizePretty[i].length();
		string fname = fileList[i];
		if (fname.length() + 1 > availableWidth) {
			fname = fname.substr(0, availableWidth - 3) + "...";
		}
		fileNamePretty.push_back(" " + fname); // prepend space for fake indentation (terrible)
	}

	string filesTitle = "Published files:";

	// files title is 2 lines below status title left aligned
	sh.moveCursor(statusPanelStartX, statusPanelStartY + 2);
	cout << filesTitle;

	int fileListPaneHeight = min( max(5, (int)fileList.size()) , 10);
	int filesListPaneStartY = statusPanelStartY + 3;

	// list files
	for (int i = 0; i < fileList.size(); i++) {
		sh.moveCursor(leftAlign(stPnlMrginX, statusPanelWidth, fileList[i].length()), statusPanelStartY + i + 3);
		cout << fileNamePretty[i];

		sh.moveCursor(rightAlign(stPnlMrginX, statusPanelWidth, fileSizePretty[i].length()), statusPanelStartY + i + 3);
		cout << fileSizePretty[i];
	}

	// Active connections list 
	string connTitle = "Active connections:";
	int connPanelStartY = filesListPaneStartY + fileListPaneHeight;

	sh.moveCursor(leftAlign(stPnlMrginX, statusPanelWidth, connTitle.length()), connPanelStartY);
	cout << connTitle;

#pragma endregion

	cout.flush();
	cin.get();
}
