#include "DownloadThread.h"

void DownloadThread::operator()(CommonObjects& common) {
    try {
        initializeGame(common);
        game_logic.notifyGameInitialized();
    }
    catch (const std::exception& e) {
        game_logic.notifyApiError(e.what());
        return;
    }

    while (!common.exit_flag) {
        if (common.new_guess_available && common.waiting_for_api) {
            try {
                processGuess(common);
            }
            catch (const std::exception& e) {
                game_logic.notifyApiError(e.what());
            }
            common.new_guess_available = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}


void DownloadThread::initializeGame(CommonObjects& common) {
    httplib::SSLClient cli("wordle-api-kappa.vercel.app");
    cli.enable_server_certificate_verification(false); 
    auto res = cli.Get("/answer");

    if (res && res->status == 200) {
        try {
            auto json_result = nlohmann::json::parse(res->body);
            common.current_answer = json_result["word"].get<std::string>();
            common.game_initialized = true;
        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            throw;
        }
    }
    else {
        std::cerr << "Error fetching answer: " << (res ? res->status : -1) << std::endl;
        throw std::runtime_error("Failed to fetch answer");
    }
}

void DownloadThread::processGuess(CommonObjects& common) {
    httplib::SSLClient cli("wordle-api-kappa.vercel.app");
    cli.enable_server_certificate_verification(false);

    auto res = cli.Post("/" + common.current_guess);

    if (res && res->status == 200) {
        try {
            auto json_result = nlohmann::json::parse(res->body);
            GuessResult result;
            result.word = json_result["guess"].get<std::string>();
            result.is_valid_word = json_result["is_word_in_list"].get<bool>();
            result.is_correct = json_result["is_correct"].get<bool>();

            if (result.is_valid_word) {
                if (!json_result["character_info"].is_null()) {
                    for (size_t i = 0; i < 5; ++i) {
                        const auto& char_info = json_result["character_info"][i];
                        result.letter_states[i].letter = char_info["char"].get<std::string>()[0];
                        result.letter_states[i].in_word = char_info["scoring"]["in_word"].get<bool>();
                        result.letter_states[i].correct_position = char_info["scoring"]["correct_idx"].get<bool>();
                    }
                }
                game_logic.notifyGuessProcessed(result);
            }
            else {
                // Handle invalid word
                game_logic.notifyApiError("Invalid word: " + result.word);
            }
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Error parsing API response: " + std::string(e.what()));
        }
    }
    else {
        throw std::runtime_error("Failed to process guess. Status: " +
            std::to_string(res ? res->status : -1));
    }
}