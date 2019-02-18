
#include <stdio.h>
#include <stdlib.h>
#include "helper.h"


#include <unistd.h>
#include <termios.h>
#include <pthread.h>

/// Parameters:

const integer height = 50;
const integer width = 50;

const integer max_inventory_count = 10;

/// Constants

#define air 0
#define dirt 1
/// ...
#define player -1


#define block_placement true
#define block_removal false
#define survival false
#define creative true


/// Structures:

struct block {
    integer type;
};

struct signed_integer_pair {
    signed_integer first;
    signed_integer second;
};
struct float_pair {
    float first;
    float second;
};

struct item {
    integer item;
    integer count;
};\

struct inventory {
    struct item* items;
    integer item_count;
};



/// Globals:

struct block** space;

struct signed_integer_pair player_location;
struct signed_integer_pair previous_player_location;


boolean block_mode = block_removal;
boolean gamemode = survival;


struct inventory inventory;
integer selected_item = 0;


boolean hault = false;
char input = 0;

pthread_t input_thread;
pthread_mutex_t input_lock;


/// Helpers:

void initialize_space() {
    space = calloc(height, sizeof(struct block*));
    for (integer i = 0; i < height; i++)
        space[i] = calloc(width, sizeof(struct block));
}

void generate() {
    /// generate flat dirt world:
    for (integer i = 0; i < 8; i++) {
        for (integer j = width; j--;) {
            space[i][j].type = dirt;
        }
    }
}

void set_player_location() {
    space[previous_player_location.first][previous_player_location.second].type = air;
    space[player_location.first][player_location.second].type = player;
    previous_player_location = player_location;
}

void spawn_player() {
    player_location.first = height / 2;
    player_location.second = width / 2;
    previous_player_location = player_location;
    set_player_location();
}

void initialize_inventory() {
    inventory.items = calloc(max_inventory_count, sizeof(struct item));
    inventory.item_count = 0;
}

int getcharacter() {
    int input;
    struct termios old, new;
    tcgetattr(STDIN_FILENO, &old);
    new = old;
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
    system("stty -echo");
    input = getchar();
    system("stty echo");
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    return input;
}

void* get_input(void* unused) {
     do {
        pthread_mutex_lock(&input_lock);
        input = getcharacter();
        pthread_mutex_unlock(&input_lock);
        if (input == '\\') {
            gamemode = !gamemode;
        } else {}
    } while (input != '/');
    hault = true;
    return unused;
}

void do_gravity() {
    if (player_location.first > 0) {
        if (space[player_location.first - 1][player_location.second].type == air) {
            player_location.first--;
        }
    }
}


static void clip_player_location() {
    if (player_location.first >= height) {
        player_location.first = height - 1;
    }
    if (player_location.first < 0) {
        player_location.first = 0;
    }
    if (player_location.second >= width) {
        player_location.second = width - 1;
    }
    if (player_location.second < 0) {
        player_location.second = 0;
    }
}

void process_player_actions() {

    /// Player Movement
    
    if (input == 'w') {
        /// first++
        if (player_location.first < height - 3
            && ((player_location.first > 0 && space[player_location.first - 1][player_location.second].type != air) || (player_location.first == 0))
            && space[player_location.first + 1][player_location.second].type == air
            && space[player_location.first + 2][player_location.second].type == air
            && space[player_location.first + 3][player_location.second].type == air
            && gamemode == survival
            
            ) {
            player_location.first += 3;
        }
        if (player_location.first < height - 2
            && ((player_location.first > 0 && space[player_location.first - 1][player_location.second].type != air) || (player_location.first == 0))
            && space[player_location.first + 1][player_location.second].type == air
            && space[player_location.first + 2][player_location.second].type == air
            && gamemode == survival
            
        ) {
            player_location.first += 2;
        }
        else if (player_location.first < height - 1
            && (gamemode == creative || ((player_location.first > 0 && space[player_location.first - 1][player_location.second].type != air) || (player_location.first == 0)))
            && space[player_location.first + 1][player_location.second].type == air
        ) {
            player_location.first++;
        }
        
    } else if (input == 's') {
        /// first--
        if (player_location.first > 0
            && space[player_location.first - 1][player_location.second].type == air
        ) {
            player_location.first--;
        }
        
    } else if (input == 'a') {
        /// second++
        
        if (player_location.second < width - 1
            && space[player_location.first][player_location.second + 1].type == air
        ) {
            player_location.second++;
        }
    
        
    } else if (input == 'd') {
        /// second--
        
        if (player_location.second > 0
            && space[player_location.first][player_location.second - 1].type == air
            ) {
            player_location.second--;
        }

        
    /// Player block actions:
        
    } else if (input == 'i') {
        /// first++
        if (block_mode == block_removal) {
            if (player_location.first < height - 1 && space[player_location.first + 1][player_location.second].type != air) {
                inventory.items[selected_item].item = space[player_location.first + 1][player_location.second].type;
                inventory.items[selected_item].count++;
                space[player_location.first + 1][player_location.second].type = air;
            }
        } else if (block_mode == block_placement) {
            if (player_location.first < height - 1 && inventory.items[selected_item].item != air && space[player_location.first + 1][player_location.second].type == air) {
                space[player_location.first + 1][player_location.second].type = inventory.items[selected_item].item;
                if (gamemode == survival) {
                    inventory.items[selected_item].count--;
                    if (inventory.items[selected_item].count == 0) inventory.items[selected_item].item = air;
                }
            }
        }
        
        
    } else if (input == 'k') {
        /// first--
       
        if (block_mode == block_removal) {
            if (player_location.first > 0 && space[player_location.first - 1][player_location.second].type != air) {
                inventory.items[selected_item].item = space[player_location.first - 1][player_location.second].type;
                inventory.items[selected_item].count++;
                space[player_location.first - 1][player_location.second].type = air;
            }
        } else if (block_mode == block_placement) {
            if (player_location.first > 0 && inventory.items[selected_item].item != air && space[player_location.first - 1][player_location.second].type == air) {
                space[player_location.first - 1][player_location.second].type = inventory.items[selected_item].item;
                if (gamemode == survival) {
                    inventory.items[selected_item].count--;
                    if (inventory.items[selected_item].count == 0) inventory.items[selected_item].item = air;
                }
            }
        }
        
    } else if (input == 'j') {
        /// second++
        
        if (block_mode == block_removal) {
            if (player_location.second < width - 1 && space[player_location.first][player_location.second + 1].type != air) {
                inventory.items[selected_item].item = space[player_location.first][player_location.second + 1].type;
                inventory.items[selected_item].count++;
                space[player_location.first][player_location.second + 1].type = air;
            }
        } else if (block_mode == block_placement) {
            if (player_location.second < width - 1 && inventory.items[selected_item].item != air && space[player_location.first][player_location.second + 1].type == air) {
                space[player_location.first][player_location.second + 1].type = inventory.items[selected_item].item;
                if (gamemode == survival) {
                    inventory.items[selected_item].count--;
                    if (inventory.items[selected_item].count == 0) inventory.items[selected_item].item = air;
                }
            }
        }
    
        
    } else if (input == 'l') {
        /// second--
        
        if (block_mode == block_removal) {
            if (player_location.second > 0 && space[player_location.first][player_location.second - 1].type != air) {
                inventory.items[selected_item].item = space[player_location.first][player_location.second - 1].type;
                inventory.items[selected_item].count++;
                space[player_location.first][player_location.second - 1].type = air;
            }
        } else if (block_mode == block_placement) {
            if (player_location.second > 0 && inventory.items[selected_item].item != air && space[player_location.first][player_location.second - 1].type == air) {
                space[player_location.first][player_location.second - 1].type = inventory.items[selected_item].item;
                if (gamemode == survival) {
                    inventory.items[selected_item].count--;
                    if (inventory.items[selected_item].count == 0) inventory.items[selected_item].item = air;
                }
            }
        }
    
    } else if (input == '<') {
        if (selected_item > 0) selected_item--;
    } else if (input == '>') {
        if (selected_item < max_inventory_count - 1) selected_item++;
    
    } else if (input == ' ') {
        block_mode = !block_mode;
    }
    input = 0;
}

static void do_game_cycle() {
    process_player_actions();
    if (gamemode == survival) do_gravity();
    clip_player_location();
    set_player_location();
}

static void draw() {
    printf("\n\n\n\n\n\n\n\n");
    // print space:
    for (integer i = height; i--;) {
        for (integer j = width; j--;) {
            register integer type = space[i][j].type;
            if (type == air) printf("   ");
            else if (type == dirt) printf("[#]");
            else if (type == player) printf(" x ");
            else printf("[?]");
        } printf("\n");
    }
    // print divider:
    for (integer j = 0; j < width; j++) {
        printf("===");
    } printf("\n");
    
    // print block mode:
    printf("(%c)(%c) : ", block_mode ? 'P' : 'D', gamemode? 'C':'S');
    
    // print inventory:
    for (integer i = 0; i < max_inventory_count; i++) {
        register integer item = inventory.items[i].item;
        if (item == air) printf("{ } ");
        else if (item == dirt) printf("{#} ");
        else printf("{?} ");
    } printf("\n");
    for (integer i = 9; i--;) printf(" ");
    for (integer i = 0; i < max_inventory_count; i++) {
        if (i == selected_item) printf(" ^  ");
        else printf("    ");
    } printf("\n");
    for (integer i = 9; i--;) printf(" ");
    for (integer i = 0; i < selected_item; i++)
        printf("    ");
    printf(" %d\n", inventory.items[selected_item].count);
    fflush(stdout);
}


int main(int argc, const char * argv[]) {
    printf("A game made by daniel rehman.\n");
    printf("controls:\n\n");
    printf("\t. WASD to move, IJKL to destroy/place\n");
    printf("\t. spacebar to change destroy or place mode\n");
    printf("\t. \\ to change gamemode\n");
    printf("\t. / to quit the game\n\n");
    printf("ready? press any key, then enter to start the game.\n");
    getc(stdin);
    
    initialize_space();
    generate();
    spawn_player();
    initialize_inventory();
    
    pthread_mutex_init(&input_lock, NULL);
    pthread_create(&input_thread, NULL, get_input, NULL);
    
    while (!hault) {
        do_game_cycle();
        draw();
        usleep(50000);
    }
    pthread_join(input_thread, NULL);
    pthread_mutex_destroy(&input_lock);
    return 0;
}
