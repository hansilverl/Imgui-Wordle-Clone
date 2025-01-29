#include <iostream>
#include <thread>
#include <string>
#include <algorithm>
#include "CommonObjects.h"
#include "DownloadThread.h"
#include "GameLogic.h"
#include "DrawThread.h"
#include "Scoreboard.h"

int main() {
	// initialize the game logic and threads
	CommonObjects common;
	GameLogic game_logic(common);
	DownloadThread download(game_logic);
	DrawThread draw(game_logic);

	// Start the download thread
	auto download_thread = std::jthread([&](std::stop_token stoken) {
		download(common);
	});

	// Run the draw thread (this will block until window is closed)
	draw();

	// Cleanup
	common.exit_flag = true;

	return 0;
}