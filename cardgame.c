#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ID_SIZE 20
#define SAVE_FILE "game_save.txt"
#define MAX_PLAYERS 5

typedef struct 
{
    int rank; 
    char suit; 
} Card;

typedef struct 
{
    char *id; 
    Card *deck; 
    int deckSize; 
    int score; 
} Player;

typedef struct 
{
    Card *main_deck;
    int main_deck_size;
    Player players[MAX_PLAYERS];
    Player computer;
    int current_player_index;
    int round;
    int player_count;
    int consecutive_ties;
    int last_winner;
} GameState;


void menu(GameState *game);
void createAcc(GameState *game);
void login(GameState *game);
void logout(GameState *game);
void shuffle(Card *deck, int size);
void dealCards(GameState *game, int numCards);
void createDeck(Card *deck);
void freeMemory(GameState *game);
void dispScore(GameState *game);
void printCard(Card card);
void printDeck(Player player);
void saveGame(GameState *game);
void newRound(GameState *game);
void loadGame(GameState *game);
void playRound(GameState *game);
int findPlayerIndex(GameState *game, const char *username);
void continueGame(GameState *game);
void quicksortPlayers(Player players[], int low, int high);
int binarySearchPlayers(Player players[], int l, int r, const char *username);
int partition(Player players[], int low, int high);
void swapPlayers(Player *a, Player *b);
void addCardToDeck(Player *player, Card card);
void handleTie(GameState *game, Player *player, Card playerCard, Card computerCard);

int main()
{
    srand(time(NULL));
    GameState game;
    
    // Initialize game state
    game.main_deck = malloc(52 * sizeof(Card));
    game.main_deck_size = 52;
    createDeck(game.main_deck);
    
    // Initialize player
    for (int i = 0; i < MAX_PLAYERS; i++) {
        game.players[i].id = NULL;
        game.players[i].deck = NULL;
        game.players[i].deckSize = 0;
        game.players[i].score = 0;
    }

    // Initialize pc
    game.computer.id = strdup("Computer");
    game.computer.deck = NULL;
    game.computer.deckSize = 0;
    game.computer.score = 0;

    game.current_player_index = -1;
    game.round = 0;
    game.player_count = 0;
    game.consecutive_ties = 0;
    game.last_winner = -1;

    // Auto load 
    printf(" ~ WELCOME ~  \n");
    loadGame(&game);

    menu(&game);
    freeMemory(&game);

    return 0;
}

void menu(GameState *game)
{
    int choice;
    char buffer[4];
    
    do {
        printf("\n    MENU \n");
        printf("\n");
        printf("Options : \n");
        printf("1. Create Account\n");
        printf("2. Login\n");
        printf("3. Logout\n");
        printf("4. New Game\n");
        printf("5. Save Game\n");
        printf("6. Load Game\n");
        printf("7. Continue Game\n");
        printf("8. Display Score\n");
        printf("9. Exit\n");
        printf("\n");
        if (game->player_count > 0) {
        printf("Available players: ");
        for (int i = 0; i < game->player_count; i++) {
            if (game->players[i].id) {
                printf("%s ", game->players[i].id);
            }
        }
        printf("\n");
        printf("\n");
    }
        
        do {
            printf("Enter your choice (1-9): ");
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                printf("Error reading input. Please try again.\n");
                continue;
            }
            choice = atoi(buffer);
        } while (choice < 1 || choice > 9);

        switch (choice) {
            case 1:
                createAcc(game);
                break;
            case 2:
                login(game);
                break;
            case 3:
                logout(game);
                break;
            case 4:
                if (game->current_player_index != -1) {
                    newRound(game);
                } else {
                    printf("You are not logged in!\n");
                }
                break;
            case 5:
                saveGame(game);
                break;
            case 6:
                loadGame(game);
                break;
            case 7:
                continueGame(game);
                break;
            case 8:
                dispScore(game);
                break;
            case 9:
                printf("Exiting game...\n");
                break;
            default:
                printf("ERROR: Unknown option, please try again.\n");
        }
    } while (choice != 9);
}

void createAcc(GameState *game)
{
    if (game->player_count >= MAX_PLAYERS) {
        printf("Maximum number of players (%d) reached!\n", MAX_PLAYERS);
        return;
    }

    char buffer[ID_SIZE];
    printf("Please enter a username (max %d characters): ", ID_SIZE - 1);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        printf("Error reading input.\n");
        return;
    }
    buffer[strcspn(buffer, "\n")] = 0;

    if (strlen(buffer) == 0) {
        printf("ERROR: No characters were typed.\n");
        return;
    }

    if (strlen(buffer) >= ID_SIZE - 1) {
        printf("ERROR: The username surpassed the max characters available.\n");
        return;
    }

    // !Check if username already exists!
    for (int i = 0; i < game->player_count; i++) {
        if (game->players[i].id && strcmp(game->players[i].id, buffer) == 0) {
            printf("ERROR: Username already exists!\n");
            return;
        }
    }

    // Create new player
    int index = game->player_count;
    game->players[index].id = strdup(buffer);
    game->players[index].deck = NULL;
    game->players[index].deckSize = 0;
    game->players[index].score = 0;
    
    game->player_count++;
    game->current_player_index = index;

    // Sort after creation of new player
    quicksortPlayers(game->players, 0, game->player_count - 1);

    printf("Created account with username: %s\n", game->players[index].id);
    printf("Total players: %d\n", game->player_count);
}

void login(GameState *game)
{
    if (game->player_count == 0) {
        printf("No players exist. Please create an account first or load a saved game.\n");
        return;
    }

    char buffer[ID_SIZE];
    printf("Please enter your username (max %d characters): ", ID_SIZE - 1);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        printf("Error reading input.\n");
        return;
    }
    buffer[strcspn(buffer, "\n")] = 0;

    if (strlen(buffer) == 0) {
        printf("ERROR: No characters were typed.\n");
        return;
    }

    int index = binarySearchPlayers(game->players, 0, game->player_count - 1, buffer);
    if (index == -1) {
        printf("ERROR: Player '%s' not found!\n", buffer);
        printf("Available players: ");
        for (int i = 0; i < game->player_count; i++) {
            if (game->players[i].id) {
                printf("%s ", game->players[i].id);
            }
        }
        printf("\n");
        return;
    }

    game->current_player_index = index;
    printf("Logged in successfully!\n");
    printf("Welcome, %s (Score: %d)\n", game->players[index].id, game->players[index].score);
}

void logout(GameState *game)
{
    if (game->current_player_index == -1) {
        printf("No user is currently logged in!\n");
        return;
    }
    
    printf("Logging out %s...\n", game->players[game->current_player_index].id);
    game->current_player_index = -1;
}

int findPlayerIndex(GameState *game, const char *username)
{
    return binarySearchPlayers(game->players, 0, game->player_count - 1, username);
}

void continueGame(GameState *game)
{
    if (game->current_player_index == -1) {
        printf("You need to be logged in to continue a game!\n");
        return;
    }

    Player *current_player = &game->players[game->current_player_index];
    
    if (current_player->deckSize > 0 && game->computer.deckSize > 0) {
        printf("Continuing game for %s...\n", current_player->id);
        playRound(game);
    } else {
        printf("No active game to continue. Start a new game first.\n");
    }
}

void newRound(GameState *game)
{
    if (game->current_player_index == -1) {
        printf("You are not logged in!\n");
        return;
    }

    if (game->main_deck_size < 2) {
        printf("Not enough cards in the deck to play.\n");
        return;
    }

    printf("Enter the number of cards you want (1 - %d): ", game->main_deck_size / 2);
    char buffer[10];
    int numCards;
    
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        printf("Error reading input.\n");
        return;
    }
    numCards = atoi(buffer);
    
    if (numCards < 1 || numCards > game->main_deck_size / 2) {
        printf("Invalid number of cards.\n");
        return;
    }

    shuffle(game->main_deck, game->main_deck_size);
    dealCards(game, numCards);
    game->round = 0;
    game->consecutive_ties = 0;
    game->last_winner = -1;

    playRound(game);
}

void playRound(GameState *game)
{
    if (game->current_player_index == -1) {
        printf("No player is logged in!\n");
        return;
    }

    Player *current_player = &game->players[game->current_player_index];
    char buffer[10];
    int continuePlaying = 1;

    while (current_player->deckSize > 0 && game->computer.deckSize > 0 && continuePlaying) {
        game->round++;
        printf("\n    Round %d \n", game->round);
        
        Card playerCard = current_player->deck[0];
        Card computerCard = game->computer.deck[0];

        printf("Player's card: ");
        printCard(playerCard);
        printf("Computer's card: ");
        printCard(computerCard);

        int current_winner = -1;
        
        if (playerCard.rank > computerCard.rank) {
            printf("You win this round!\n");
            current_player->score += 3;
            current_winner = 0;
            
            // player gets the cards that are played because they won
            addCardToDeck(current_player, playerCard);
            addCardToDeck(current_player, computerCard);
            
        } else if (playerCard.rank < computerCard.rank) {
            printf("Computer wins this round!\n");
            game->computer.score += 3;
            current_player->score = (current_player->score > 0) ? current_player->score - 1 : 0;
            current_winner = 1;
            
            // pc gets the cards that are played because they won
            addCardToDeck(&game->computer, playerCard);
            addCardToDeck(&game->computer, computerCard);
            
        } else {
            printf("It's a tie!\n");
            handleTie(game, current_player, playerCard, computerCard);
            game->consecutive_ties++;
        }

        // first card removed from deck
        for (int i = 0; i < current_player->deckSize - 1; i++) {
            current_player->deck[i] = current_player->deck[i + 1];
        }
        for (int i = 0; i < game->computer.deckSize - 1; i++) {
            game->computer.deck[i] = game->computer.deck[i + 1];
        }
        
        current_player->deckSize--;
        game->computer.deckSize--;

        // !!! check from 3 consecutive ties !!!
        if (game->consecutive_ties >= 3) {
            printf("Three consecutive ties! Shuffling both decks\n");
            shuffle(current_player->deck, current_player->deckSize);
            shuffle(game->computer.deck, game->computer.deckSize);
            game->consecutive_ties = 0;
        }
        
        // update the last winner in case of tie
        if (current_winner != -1) {
            if (game->last_winner == current_winner) {
                game->consecutive_ties = 0;
            }
            game->last_winner = current_winner;
        }

        printf("\nCurrent Score:\n");
        printf("%s: %d\n", current_player->id, current_player->score);
        printf("Computer: %d\n", game->computer.score);
        printf("Cards remaining - %s: %d, Computer: %d\n", 
               current_player->id, current_player->deckSize, game->computer.deckSize);

        // continue...?
        if (current_player->deckSize > 0 && game->computer.deckSize > 0) {
            printf("\nDo you want to continue to the next round? (1 = Yes, 0 = No): ");
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                printf("Error reading input. Continuing to next round.\n");
                continue;
            }
            continuePlaying = atoi(buffer);
            
            if (!continuePlaying) {
                printf("Game paused. You can continue later :) \n");
                break;
            }
        }
    }

    if (current_player->deckSize == 0) {
        printf("\nYou have no cards left. Game Over!\n");
    } else if (game->computer.deckSize == 0) {
        printf("\nComputer has no cards left. You Win!\n");
    }
}

void handleTie(GameState *game, Player *player, Card playerCard, Card computerCard)
{
    // return to the main deck
    if (game->main_deck_size < 52) {
        game->main_deck[game->main_deck_size++] = playerCard;
        game->main_deck[game->main_deck_size++] = computerCard;
    }
    
    // give out new card
    if (game->main_deck_size >= 2) {
        addCardToDeck(player, game->main_deck[game->main_deck_size - 2]);
        addCardToDeck(&game->computer, game->main_deck[game->main_deck_size - 1]);
        game->main_deck_size -= 2;
    }
}

void addCardToDeck(Player *player, Card card)
{
    player->deck = realloc(player->deck, (player->deckSize + 1) * sizeof(Card));
    player->deck[player->deckSize] = card;
    player->deckSize++;
}

void dealCards(GameState *game, int numCards)
{
    if (game->current_player_index == -1) {
        printf("No player is logged in!\n");
        return;
    }

    Player *current_player = &game->players[game->current_player_index];
    
    // Free existing decks
    free(current_player->deck);
    free(game->computer.deck);
    
    // Allocate new decks
    current_player->deck = malloc(numCards * sizeof(Card));
    game->computer.deck = malloc(numCards * sizeof(Card));
    
    if (!current_player->deck || !game->computer.deck) {
        printf("ERROR: Memory allocation failed.\n");
        return;
    }
    
    current_player->deckSize = numCards;
    game->computer.deckSize = numCards;

    // give out cards
    for (int i = 0; i < numCards; i++) {
        current_player->deck[i] = game->main_deck[i];
        game->computer.deck[i] = game->main_deck[i + numCards];
    }

    // remove cards from main deck 
    for (int i = 0; i < game->main_deck_size - (2 * numCards); i++) {
        game->main_deck[i] = game->main_deck[i + (2 * numCards)];
    }
    game->main_deck_size -= (2 * numCards);

    printf("Cards have been dealt to %s and Computer!\n", current_player->id);
}

void shuffle(Card *deck, int size)
{
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Card temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

void createDeck(Card *deck)
{
    char suits[] = {'H', 'D', 'C', 'S'};
    int index = 0;
    
    for (int s = 0; s < 4; s++) {
        for (int r = 2; r <= 14; r++) {
            deck[index].rank = r;
            deck[index].suit = suits[s];
            index++;
        }
    }
}

void printCard(Card card)
{
    char rank_str[3];
    if (card.rank >= 2 && card.rank <= 10) {
        sprintf(rank_str, "%d", card.rank);
    } else {
        switch (card.rank) {
            case 11: strcpy(rank_str, "J"); break;
            case 12: strcpy(rank_str, "Q"); break;
            case 13: strcpy(rank_str, "K"); break;
            case 14: strcpy(rank_str, "A"); break;
            default: strcpy(rank_str, "?"); break;
        }
    }
    printf("%s%c\n", rank_str, card.suit);
}

void saveGame(GameState *game)
{
    FILE *file = fopen(SAVE_FILE, "w");
    if (!file) {
        printf("ERROR: Could not create save file.\n");
        return;
    }

    // save the number of players and the index of current player
    fprintf(file, "PLAYER_COUNT:%d\n", game->player_count);
    fprintf(file, "CURRENT_PLAYER_INDEX:%d\n", game->current_player_index);

    // Save for all the players
    for (int i = 0; i < game->player_count; i++) {
        fprintf(file, "PLAYER_ID:%s\n", game->players[i].id);
        fprintf(file, "PLAYER_SCORE:%d\n", game->players[i].score);
        fprintf(file, "PLAYER_DECK_SIZE:%d\n", game->players[i].deckSize);
        for (int j = 0; j < game->players[i].deckSize; j++) {
            fprintf(file, "PLAYER_CARD:%d %c\n", game->players[i].deck[j].rank, game->players[i].deck[j].suit);
        }
        fprintf(file, "PLAYER_END\n");
    }

    // Save computer info
    fprintf(file, "COMPUTER_SCORE:%d\n", game->computer.score);
    fprintf(file, "COMPUTER_DECK_SIZE:%d\n", game->computer.deckSize);
    for (int i = 0; i < game->computer.deckSize; i++) {
        fprintf(file, "COMPUTER_CARD:%d %c\n", game->computer.deck[i].rank, game->computer.deck[i].suit);
    }

    // Save main deck
    fprintf(file, "MAIN_DECK_SIZE:%d\n", game->main_deck_size);
    for (int i = 0; i < game->main_deck_size; i++) {
        fprintf(file, "MAIN_CARD:%d %c\n", game->main_deck[i].rank, game->main_deck[i].suit);
    }

    // Save round info and game state
    fprintf(file, "ROUND:%d\n", game->round);
    fprintf(file, "CONSECUTIVE_TIES:%d\n", game->consecutive_ties);
    fprintf(file, "LAST_WINNER:%d\n", game->last_winner);
    
    fclose(file);
    printf("Game saved successfully to %s!\n", SAVE_FILE);
}

void loadGame(GameState *game)
{
    FILE *file = fopen(SAVE_FILE, "r");
    if (!file) {
        printf("No saved game found. Starting a new one \n");
        return;
    }

    char line[100];
    int current_player = -1;
    int player_card_index = 0;
    int computer_card_index = 0;
    int main_card_index = 0;

    // Clear data
    for (int i = 0; i < MAX_PLAYERS; i++) {
        free(game->players[i].id);
        free(game->players[i].deck);
        game->players[i].id = NULL;
        game->players[i].deck = NULL;
        game->players[i].deckSize = 0;
        game->players[i].score = 0;
    }
    
    free(game->computer.deck);
    game->computer.deck = NULL;
    game->computer.deckSize = 0;
    game->computer.score = 0;
    
    free(game->main_deck);
    game->main_deck = NULL;
    game->main_deck_size = 0;
    
    game->current_player_index = -1;
    game->round = 0;
    game->player_count = 0;
    game->consecutive_ties = 0;
    game->last_winner = -1;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;

        if (strstr(line, "PLAYER_COUNT:") == line) {
            game->player_count = atoi(line + 13);
        }
        else if (strstr(line, "CURRENT_PLAYER_INDEX:") == line) {
            game->current_player_index = atoi(line + 21);
        }
        else if (strstr(line, "PLAYER_ID:") == line) {
            current_player++;
            if (current_player < MAX_PLAYERS) {
                game->players[current_player].id = strdup(line + 10);
            }
        }
        else if (strstr(line, "PLAYER_SCORE:") == line) {
            if (current_player >= 0 && current_player < MAX_PLAYERS) {
                game->players[current_player].score = atoi(line + 13);
            }
        }
        else if (strstr(line, "PLAYER_DECK_SIZE:") == line) {
            if (current_player >= 0 && current_player < MAX_PLAYERS) {
                int size = atoi(line + 17);
                game->players[current_player].deck = malloc(size * sizeof(Card));
                game->players[current_player].deckSize = size;
                player_card_index = 0;
            }
        }
        else if (strstr(line, "PLAYER_CARD:") == line) {
            if (current_player >= 0 && current_player < MAX_PLAYERS) {
                int rank;
                char suit;
                sscanf(line + 12, "%d %c", &rank, &suit);
                if (player_card_index < game->players[current_player].deckSize) {
                    game->players[current_player].deck[player_card_index].rank = rank;
                    game->players[current_player].deck[player_card_index].suit = suit;
                    player_card_index++;
                }
            }
        }
        else if (strstr(line, "PLAYER_END") == line) {
            player_card_index = 0;
        }
        else if (strstr(line, "COMPUTER_SCORE:") == line) {
            game->computer.score = atoi(line + 15);
        }
        else if (strstr(line, "COMPUTER_DECK_SIZE:") == line) {
            int size = atoi(line + 19);
            game->computer.deck = malloc(size * sizeof(Card));
            game->computer.deckSize = size;
            computer_card_index = 0;
        }
        else if (strstr(line, "COMPUTER_CARD:") == line) {
            int rank;
            char suit;
            sscanf(line + 14, "%d %c", &rank, &suit);
            if (computer_card_index < game->computer.deckSize) {
                game->computer.deck[computer_card_index].rank = rank;
                game->computer.deck[computer_card_index].suit = suit;
                computer_card_index++;
            }
        }
        else if (strstr(line, "MAIN_DECK_SIZE:") == line) {
            game->main_deck_size = atoi(line + 15);
            game->main_deck = malloc(game->main_deck_size * sizeof(Card));
            main_card_index = 0;
        }
        else if (strstr(line, "MAIN_CARD:") == line) {
            int rank;
            char suit;
            sscanf(line + 10, "%d %c", &rank, &suit);
            if (main_card_index < game->main_deck_size) {
                game->main_deck[main_card_index].rank = rank;
                game->main_deck[main_card_index].suit = suit;
                main_card_index++;
            }
        }
        else if (strstr(line, "ROUND:") == line) {
            game->round = atoi(line + 6);
        }
        else if (strstr(line, "CONSECUTIVE_TIES:") == line) {
            game->consecutive_ties = atoi(line + 17);
        }
        else if (strstr(line, "LAST_WINNER:") == line) {
            game->last_winner = atoi(line + 12);
        }
    }
    
    fclose(file);
    
    // restore pc id
    game->computer.id = strdup("Computer");
    
    // sort after load
    quicksortPlayers(game->players, 0, game->player_count - 1);
}

void dispScore(GameState *game)
{
    if (game->player_count == 0) {
        printf("No players found.\n");
        return;
    }

    printf("\n=== PLAYER SCORES ===\n");
    for (int i = 0; i < game->player_count; i++) {
        if (game->players[i].id) {
            printf("%s: %d points\n", game->players[i].id, game->players[i].score);
        }
    }
    
    if (game->current_player_index != -1) {
        printf("\nCurrent player: %s (Score: %d)\n", 
               game->players[game->current_player_index].id,
               game->players[game->current_player_index].score);
    }
    printf("Computer: %d points\n", game->computer.score);
}

void freeMemory(GameState *game)
{
    // Free all players
    for (int i = 0; i < MAX_PLAYERS; i++) {
        free(game->players[i].id);
        free(game->players[i].deck);
        game->players[i].id = NULL;
        game->players[i].deck = NULL;
        game->players[i].deckSize = 0;
        game->players[i].score = 0;
    }
    
    free(game->computer.id);
    free(game->computer.deck);
    game->computer.id = NULL;
    game->computer.deck = NULL;
    game->computer.deckSize = 0;
    game->computer.score = 0;
    
    free(game->main_deck);
    game->main_deck = NULL;
    game->main_deck_size = 0;
    
    game->current_player_index = -1;
    game->round = 0;
    game->player_count = 0;
    game->consecutive_ties = 0;
    game->last_winner = -1;
}

void printDeck(Player player)
{
    if (!player.deck || player.deckSize <= 0) {
        printf("No cards in deck to display.\n");
        return;
    }
    
    printf("%s's Deck:\n", player.id ? player.id : "Unknown");
    for (int i = 0; i < player.deckSize; i++) {
        printf("Card %d: ", i + 1);
        printCard(player.deck[i]);
    }
}


void swapPlayers(Player *a, Player *b)
{
    Player temp = *a;
    *a = *b;
    *b = temp;
}

int partition(Player players[], int low, int high)
{
    char *pivot = players[high].id;
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        if (strcmp(players[j].id, pivot) < 0) {
            i++;
            swapPlayers(&players[i], &players[j]);
        }
    }
    swapPlayers(&players[i + 1], &players[high]);
    return (i + 1);
}

// Quicksort for players(id)

void quicksortPlayers(Player players[], int low, int high)
{
    if (low < high) {
        int pi = partition(players, low, high);
        quicksortPlayers(players, low, pi - 1);
        quicksortPlayers(players, pi + 1, high);
    }
}

// binary search for player's username
int binarySearchPlayers(Player players[], int l, int r, const char *username)
{
    while (l <= r) {
        int m = l + (r - l) / 2;
        int cmp = strcmp(players[m].id, username);
        
        if (cmp == 0)
            return m;
            
        if (cmp < 0)
            l = m + 1;
        else
            r = m - 1;
    }
    
    return -1;
}
