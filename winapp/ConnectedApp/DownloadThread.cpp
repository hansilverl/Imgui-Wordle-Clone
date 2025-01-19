#include "DownloadThread.h"

void DownloadThread::operator()(CommonObjects& common) {
    // Initialize the game by getting today's answer
    try {
        initializeGame(common);
    }
    catch (const std::exception& e) {
        std::cerr << "Error initializing game: " << e.what() << std::endl;
        return;
    }

    while (!common.exit_flag) {
        if (common.new_guess_available && !common.waiting_for_api) {
            try {
                processGuess(common);
            }
            catch (const std::exception& e) {
                std::cerr << "Error processing guess: " << e.what() << std::endl;
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
    common.waiting_for_api = true;

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
            if (result.is_valid_word && !result.is_correct){
                if (!json_result["character_info"].is_null()) {
                    for (size_t i = 0; i < 5; ++i) {
                        const auto& char_info = json_result["character_info"][i];
                        result.letter_states[i].letter = char_info["char"].get<std::string>()[0];
                        result.letter_states[i].in_word = char_info["scoring"]["in_word"].get<bool>();
                        result.letter_states[i].correct_position = char_info["scoring"]["correct_idx"].get<bool>();
                    }
                }

                if (!result.is_valid_word) {
					// notify the user that the word is not valid
					std::cout << result.word << " - Not a valid word!\n";   
					// we currently use cout, but soon will transition to a GUI, so we will have to change all of the couts
				}

                common.guess_history.push_back(result);

                if (result.is_correct) {
                    common.game_won = true;
                    common.game_over = true;
                }
                else if (common.guess_history.size() >= 6) {
                    common.game_over = true;
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            throw;
        }
    }
    else {
        std::cerr << "Error processing guess: " << (res ? res->status : -1) << std::endl;
        throw std::runtime_error("Failed to process guess");
    }

    common.waiting_for_api = false;
}
