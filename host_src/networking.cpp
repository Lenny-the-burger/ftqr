#include "networking.h"

// Makes a new session and returns the session ID which should always be 4 characters long
std::string makeNewSession() {
	// in real this would contact server to request a new session

    // Why is this so stupid in cpp
	const int length = 4;
    const std::string CHARACTERS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

    std::string random_string;
    for (int i = 0; i < length; ++i) {
        random_string += CHARACTERS[distribution(generator)];
    }

    return random_string;
}
