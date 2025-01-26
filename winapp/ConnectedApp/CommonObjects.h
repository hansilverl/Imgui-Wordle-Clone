#pragma once
#include <atomic>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>

// Represents the state of a single letter guess
struct LetterState {
    char letter;
    bool in_word;
    bool correct_position;
};

// Represents a complete guess attempt
struct GuessResult {
    std::string word;
    bool is_valid_word;
    bool is_correct;
    std::array<LetterState, 5> letter_states;
};

struct CommonObjects {
    // Control flags
    std::atomic_bool exit_flag = false;
    std::atomic_bool game_initialized = false;

    // Game state
    std::string current_answer;
    std::vector<GuessResult> guess_history;
    std::atomic_bool game_won = false;
    std::atomic_bool game_over = false;

    // Thread synchronization
    std::atomic_bool waiting_for_api = false;
    std::atomic_bool new_guess_available = false;

    // Current guess handling
    std::string current_guess;
};