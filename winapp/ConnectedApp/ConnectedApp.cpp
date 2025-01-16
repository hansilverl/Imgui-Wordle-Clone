#include <iostream>
#include <thread>
#include <string>
#include "CommonObject.h"
#include "DownloadThread.h"

// Helper function to validate input
bool isValidInput(const std::string& input) {
    if (input.length() != 5) return false;
    return std::all_of(input.begin(), input.end(), [](char c) {
        return std::isalpha(c);
        });
}

// Helper function to display the current game state
void displayGameState(const CommonObjects& common) {
    std::cout << "\nGuess history:\n";
    for (const auto& guess : common.guess_history) {
        if (!guess.is_valid_word) {
            std::cout << guess.word << " - Not a valid word!\n";
            continue;
        }

        std::cout << guess.word << " - ";
        for (const auto& letter : guess.letter_states) {
            if (letter.correct_position) {
                std::cout << "_" << letter.letter << "_ ";
            }
            else if (letter.in_word) {
                std::cout << "*" << letter.letter << "* ";
            }
            else {
                std::cout << " " << letter.letter << "  ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

int main() {
    CommonObjects common;
    DownloadThread download;

    // Start the download thread
    auto download_thread = std::jthread([&](std::stop_token stoken) {
        download(common);
        });

    // Wait for game initialization
    std::cout << "Initializing game...\n";
    while (!common.game_initialized && !common.exit_flag) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!common.game_initialized) {
        std::cout << "Failed to initialize game. Please check your internet connection.\n";
        common.exit_flag = true;
        return 1;
    }

    std::cout << "Wordle game started! Enter your 5-letter guesses.\n";

    // Main game loop
    while (!common.exit_flag && !common.game_over) {
        std::string input;
        std::cout << "Enter guess (" << common.guess_history.size() + 1 << "/6): ";
        std::getline(std::cin, input);

        // Convert to uppercase to match API expectations
        std::transform(input.begin(), input.end(), input.begin(), ::toupper);

        if (input == "QUIT") {
            break;
        }

        if (!isValidInput(input)) {
            std::cout << "Please enter a valid 5-letter word.\n";
            continue;
        }

        // Submit guess to API
        common.current_guess = input;
        common.new_guess_available = true;

        // Wait for API response
        while (common.waiting_for_api) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        // Display current game state
        displayGameState(common);

        // Check game end conditions
        if (common.game_won) {
            std::cout << "Congratulations! You've won!\n";
            break;
        }
        else if (common.game_over) {
            std::cout << "Game Over! The word was: " << common.current_answer << "\n";
            break;
        }
    }

    // Clean up
    common.exit_flag = true;
    return 0;
}