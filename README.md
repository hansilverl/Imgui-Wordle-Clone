
# Imgui-Wordle-Clone
![image](https://github.com/user-attachments/assets/6e88bd17-fe55-4f8d-b3df-11cc39591778)


This project is a clone of the popular word game [Wordle](https://www.nytimes.com/games/wordle/index.html), implemented using ImGui for the user interface and a custom API for game logic.

## Features

- **Wordle API**: A simple API to host your own Wordle game.
- **Custom Fonts**: Special fonts used for backspace and score icons based on FontAwesome.ttf.
- **Leaderboard**: Display high scores in a styled window.
- **File Handling**: Scores are appended to the file instead of rewriting the entire file.

## Wordle API

The Wordle API is hosted at [Wordle API](https://gitlab.com/MoofyWoofy/wordle-api). By default, this word list uses [combined_wordlist.txt](https://github.com/Kinkelin/WordleCompetition/blob/main/data/official/combined_wordlist.txt).

### Usage

To find out the answer for today's Wordle, you can make a GET request to [https://wordle-api-kappa.vercel.app/answer](https://wordle-api-kappa.vercel.app/answer).

To consume the API, send a POST request to [https://wordle-api-kappa.vercel.app/{GUESS}](https://wordle-api-kappa.vercel.app/{GUESS}), where `{GUESS}` is the guessed word.

Example:
```
https://wordle-api-kappa.vercel.app/quote
```

### Response

If the guessed word is correct, the API response will be:
```json
{
  "guess": "QUOTE",
  "is_correct": true,
  "is_word_in_list": true
}
```

If the word is not in the word list, the API response will be:
```json
{
  "guess": "ABCDE",
  "is_correct": false,
  "is_word_in_list": false
}
```

If the guessed word is in the list but the guess is incorrect, the API response will be:
```json
{
  "guess": "QUOTE",
  "is_correct": false,
  "is_word_in_list": true,
  "character_info": [
    {
      "char": "Q",
      "scoring": {
        "in_word": false,
        "correct_idx": false
      }
    },
    {
      "char": "U",
      "scoring": {
        "in_word": false,
        "correct_idx": false
      }
    },
    {
      "char": "O",
      "scoring": {
        "in_word": true,
        "correct_idx": true
      }
    },
    {
      "char": "T",
      "scoring": {
        "in_word": false,
        "correct_idx": false
      }
    },
    {
      "char": "E",
      "scoring": {
        "in_word": false,
        "correct_idx": false
      }
    }
  ]
}
```

## Custom Fonts

- **Backspace Icon**: [icons8 - Backspace](https://icons8.com/icons/set/backspace)
- **Leaderboard Icon**: [svgrepo - Leaderboard](https://www.svgrepo.com/svg/487506/leaderboard)
- **Font Editing**: Edited with [Tophix Font Editor](https://tophix.com/font-tools/font-editor)

## Important Files

- **Colors.h**: Defines the color palette used in the application.
- **CommonObjects.h**: Contains common objects and structures used throughout the application.
- **ConnectedApp.cpp**: Handles the connection logic for the application.
- **DownloadThread.cpp/h**: Manages the downloading of necessary resources.
- **DrawThread.cpp/h**: Handles the rendering of the game board and user interface.
- **GameLogic.cpp/h**: Contains the game logic and API interaction.
- **imgui.ini**: ImGui configuration file.
- **OnScreenKb.cpp/h**: Manages the on-screen keyboard rendering and interaction.
- **Scoreboard.cpp/h**: Handles the rendering and management of the high scores.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more information.

## Acknowledgments

- Wordle API by [MoofyWoofy](https://gitlab.com/MoofyWoofy/wordle-api)
- FontAwesome for the custom fonts
- icons8 and svgrepo for the icons
- Tophix for the font editing tool
- Score System inspired by [Otaku-kun, A simple scoring scheme for Wordle](https://www.haibane.info/2022/01/12/a-simple-scoring-scheme-for-wordle)

This `README.md` file provides a comprehensive overview of your project, including its features, usage instructions, custom fonts, important files, and acknowledgments. It is formatted with Markdown for better readability and presentation.
