#include <ncurses.h>

#include <cstdlib>
#include <ctime>

#include <vector>
#include <list>

using namespace std;

#define FRAME_DURATION_MILLIES 1000.0 / 60;

char player_sp[3][5] = {
  {' ', ' ', 'A', ' ', ' '},
  {'_', '/', ' ', '\\', '_'},
  {'S', 'P', 'A', 'C', 'E'}
};

char alien_sp[2][4] = {
  {'d', '0', '0', 'b'},
  {'/', '"', '"', '\\'}
};

char bullet_sp = '^';

char enemy_bullet_sp = '*';

struct alien {
  int x, y;
  int w, h;
  bool active;
};

struct bullet {
    int x, y;
    bool going_up;
    float ticks;
};

struct player {
    int x, y;
    int w, h;
};

float get_time_millies(clock_t t) {
    return ((float) t) / CLOCKS_PER_SEC * 1000;
}

void move_player(struct player *p, int dir) {
    p->x += dir;
}

void player_fire(struct player p, list<struct bullet> *bullets) {
    struct bullet * b = (struct bullet *) malloc (sizeof (struct bullet));
    b->x = p.x + p.w/2 + 1;
    b->y = p.y;
    b->going_up = TRUE;
    b->ticks = clock();
    bullets->push_back(*b);
}

void alien_fire(struct alien a, list<struct bullet> *bullets) {
    struct bullet * b = (struct bullet *) malloc (sizeof (struct bullet));
    b->x = a.x + a.w/2;
    b->y = a.y;
    b->going_up = FALSE;
    b->ticks = clock();
    bullets->push_back(*b);
}

bool onScreen(int h, int w, int y, int x) {
    return (x >= 0 && x < w && y >= 0 && y < h);
}

int main() {

    // initialize ncurses screen
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    // initialize scene variables
    int scr_height, scr_width;
    int delta_y, delta_x;
    int game_width = 45;
    int game_height = 30;
    clock_t ticks;
    srand(time(NULL));

    vector<struct alien> aliens;
    list<bullet> bullets;
    struct player player_1;
    float bullet_speed_millis = 200;

    // add player
    {
        player_1.w = 5;
        player_1.h = 3;
        player_1.x = game_width / 2 - player_1.w / 2 - 1;
        player_1.y = game_height - player_1.h - 1;
    }

    // add aliens
    int aliens_dir = 1;
    int aliens_moves = 0;
    float action = 60 * FRAME_DURATION_MILLIES;
    {
        for (int i = 0; i < 3; i ++) {
            for (int j = 0; j < 5; j ++) {
                struct alien a;
                a.h = 2;
                a.w = 4;
                a.x = (a.w + 3) * j + game_width / 9;
                a.y = (a.h + 2) * i + 2;
                a.active = true;
                aliens.push_back(a);
            }
        }
    }

    // game itself
    int ch;
    ticks = clock();
    for (;;) {
        // get input
        if ( (ch = getch()) != ERR) {
            switch (ch) {
                case KEY_LEFT:
                case 'A':
                case 'a':
                    move_player(&player_1, -1);
                    break;
                case KEY_RIGHT:
                case 'D':
                case 'd':
                    move_player(&player_1, 1);
                    break;

                case ' ':
                    player_fire(player_1, &bullets);
                    break;
            }
        }

        // aliens update
        if (get_time_millies(clock() - ticks) >= action) {
            ticks = clock();

            // alien movement
            if (aliens_moves >= 5) {
                aliens_moves %= 5;
                aliens_dir = -aliens_dir;

                // vertical movement
                for (int i = 0; i < aliens.size(); i ++) {
                    aliens[i].y += 2;
                }
            } else {
                // horizontal movement
                aliens_moves++;
                for (int i = 0; i < aliens.size(); i ++) {
                    aliens[i].x += aliens_dir * 2;
                }
            }

            // alien fire
            int alien_to_fire = rand() % aliens.size();
            if (aliens[alien_to_fire].active) {
                alien_fire(aliens[alien_to_fire], &bullets);
            }
        }

        // bullets update
        for (list<struct bullet>::iterator it = bullets.begin(); it != bullets.end(); it++) {
            if (get_time_millies(clock() - (*it).ticks) > bullet_speed_millis) {
                (*it).ticks = clock();
                if ((*it).going_up) {
                    (*it).y += -1;
                } else {
                    (*it).y += 1;
                }
            }
        }

        // clean the screen
        werase(stdscr);

        // update scene variables
        getmaxyx(stdscr, scr_height, scr_width);
        delta_y = scr_height / 2 - game_height / 2;
        delta_x = scr_width / 2 - game_width / 2;

        // draw the aliens
        for (int i = 0; i < aliens.size(); i ++) {
            if (aliens[i].active) {
                for (int x = 0; x < aliens[i].w; x ++) {
                    for (int y = 0; y < aliens[i].h; y ++) {
                        if (onScreen(scr_height, scr_width, delta_y + aliens[i].y + y, delta_x + aliens[i].x + x)) {
                            mvaddch(delta_y + aliens[i].y + y, delta_x + aliens[i].x + x, alien_sp[y][x]);
                        }
                    }
                }
            }
        }

        // draw the bullets
        for (list<struct bullet>::iterator it = bullets.begin(); it != bullets.end(); it++) {
            if ((*it).going_up) {
                mvaddch(delta_y + (*it).y, delta_x + (*it).x, bullet_sp);
            } else {
                mvaddch(delta_y + (*it).y, delta_x + (*it).x, enemy_bullet_sp);
            }
        }

        // draw the player
        for (int x = 0; x < player_1.w; x ++) {
            for (int y = 0; y < player_1.h; y ++) {
                mvaddch(delta_y + player_1.y + y, delta_x + player_1.x + x, player_sp[y][x]);
            }
        }
    }

}
