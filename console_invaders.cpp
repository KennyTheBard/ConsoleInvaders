#include <ncurses.h>
#include <string.h>

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
    bool active;
};

struct player {
    int x, y;
    int w, h;
};

float get_time_millies(clock_t t) {
    return ((float) t) / CLOCKS_PER_SEC * 1000;
}

void move_player(struct player *p, int dir, int screen_width) {
    if (p->x + dir >= 0 && p->x + dir + p->w <= screen_width) {
        p->x += dir;
    }
}

void player_fire(struct player p, list<struct bullet> *bullets) {
    struct bullet * b = (struct bullet *) malloc (sizeof (struct bullet));
    b->x = p.x + p.w/2;
    b->y = p.y;
    b->going_up = TRUE;
    b->ticks = clock();
    b->active = TRUE;
    bullets->push_back(*b);
}

void alien_fire(struct alien a, list<struct bullet> *bullets) {
    struct bullet * b = (struct bullet *) malloc (sizeof (struct bullet));
    b->x = a.x + a.w/2;
    b->y = a.y;
    b->going_up = FALSE;
    b->ticks = clock();
    b->active = TRUE;
    bullets->push_back(*b);
}

bool in_rectangle(int h_min, int w_min, int h_max, int w_max, int y, int x) {
    return (x >= w_min && x < w_max && y >= h_min && y < h_max);
}

void destroy_alien(struct alien *a, int *score, float *action) {
    a->active = FALSE;
    (*score) += 70 - (*action) / FRAME_DURATION_MILLIES;
    (*action) -= 3.5 * FRAME_DURATION_MILLIES;
}

void lose() {

}

void win() {

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
    int game_width = 60;
    int game_height = 30;
    clock_t ticks;
    srand(time(NULL));

    vector<struct alien> aliens;
    list<bullet> bullets;
    float bullet_speed_millis = 120;

    struct player player_1;
    int score = 0;
    clock_t cd = clock();

    char game_name[] = "> CONSOLE INVADERS <\n";
    char score_text[] = ":SCORE\n";

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

        for (int i = 0; i < aliens.size(); i ++) {
            if (aliens[i].y + aliens[i].h >= player_1.y) {
                if (aliens[i].y + aliens[i].h >= game_height) {
                    lose();
                }

                if (in_rectangle(player_1.y, player_1.x, player_1.y + player_1.h,
                    player_1.x + player_1.w, aliens[i].y + aliens[i].h, aliens[i].x)
                    || in_rectangle(player_1.y, player_1.x, player_1.y + player_1.h,
                    player_1.x + player_1.w, aliens[i].y + aliens[i].h, aliens[i].x + aliens[i].w - 1)) {
                        lose();
                    }
            }
        }

        // bullets update
        for (list<struct bullet>::iterator it = bullets.begin(); it != bullets.end(); it++) {
            if (!(*it).active) {
                continue;
            }

            if (get_time_millies(clock() - (*it).ticks) > bullet_speed_millis) {
                (*it).ticks = clock();
                if ((*it).going_up) {
                    (*it).y += -1;
                } else {
                    (*it).y += 1;
                }
            }

            if ((*it).y < 0 || (*it).y >= game_height) {
                (*it).active = FALSE;
                continue;
            }

            if ((*it).going_up) {
                // alien collision
                int n = aliens.size() - 1;
                if (in_rectangle(aliens[0].y, aliens[0].x, aliens[n].y + aliens[n].h, aliens[n].x + aliens[n].w, (*it).y, (*it).x)) {
                    for (int i = 0; i < aliens.size(); i ++) {
                        if (aliens[i].active) {
                            if (in_rectangle(aliens[i].y, aliens[i].x, aliens[i].y + aliens[i].h, aliens[i].x + aliens[i].w, (*it).y, (*it).x)) {
                                destroy_alien(&aliens[i], &score, &action);
                                (*it).active = FALSE;
                            }
                        }
                    }
                }
            } else {
                // ship collision
                if (in_rectangle(player_1.y, player_1.x, player_1.y + player_1.h, player_1.x + player_1.w, (*it).y, (*it).x)) {
                    lose();
                }
            }
        }

        // clean the screen
        werase(stdscr);

        //draw the borders
        {
            for (int i = delta_x - 1; i < scr_width - delta_x + 1; i++) {
                mvaddch(delta_y - 1, i, '-');
                mvaddch(delta_y + game_height, i, '-');
            }

            for (int i = delta_y; i < scr_height - delta_y; i++) {
                mvaddch(i, delta_x - 1, '|');
                mvaddch(i, delta_x + game_width, '|');
            }

            mvaddch(delta_y - 1, delta_x - 1, '+');
            mvaddch(delta_y - 1, delta_x + game_width, '+');
            mvaddch(delta_y + game_height, delta_x - 1, '+');
            mvaddch(delta_y + game_height, delta_x + game_width, '+');
        }

        // update scene variables
        {
            getmaxyx(stdscr, scr_height, scr_width);
            delta_y = (scr_height - game_height) / 2;
            delta_x = (scr_width - game_width) / 2;
        }

        // draw title
        {
            int game_name_start = delta_x + game_width / 2 - strlen(game_name) / 2;
            for (int i = 0; i < strlen(game_name); i++) {
                mvaddch(delta_y - 2, game_name_start + i, game_name[i]);
            }
        }

        // draw score bar
        int score_text_start = delta_x + game_width - 20;
        for (int i = 0; i < strlen(score_text); i++) {
            mvaddch(delta_y + game_height + 1, score_text_start + i, score_text[i]);
        }
        for (int i = 1; i < 6; i ++) {
            int num = score;
            for (int k = 1; k < i; k ++) {
                num /= 10;
            }
            char c = num % 10 + '0';
            mvaddch(delta_y + game_height + 1, score_text_start - i , c);
        }

        // draw the aliens
        for (int i = 0; i < aliens.size(); i ++) {
            if (aliens[i].active) {
                for (int x = 0; x < aliens[i].w; x ++) {
                    for (int y = 0; y < aliens[i].h; y ++) {
                        if (in_rectangle(0, 0, scr_height, scr_width, delta_y + aliens[i].y + y, delta_x + aliens[i].x + x)) {
                            mvaddch(delta_y + aliens[i].y + y, delta_x + aliens[i].x + x, alien_sp[y][x]);
                        }
                    }
                }
            }
        }

        // draw the bullets
        for (list<struct bullet>::iterator it = bullets.begin(); it != bullets.end(); it++) {
            if ((*it).active) {
                if ((*it).going_up) {
                    mvaddch(delta_y + (*it).y, delta_x + (*it).x, bullet_sp);
                } else {
                    mvaddch(delta_y + (*it).y, delta_x + (*it).x, enemy_bullet_sp);
                }
            }
        }

        // draw the player
        for (int x = 0; x < player_1.w; x ++) {
            for (int y = 0; y < player_1.h; y ++) {
                mvaddch(delta_y + player_1.y + y, delta_x + player_1.x + x, player_sp[y][x]);
            }
        }

        // get input
        if ( (ch = getch()) != ERR) {
            switch (ch) {
                case KEY_LEFT:
                case 'A':
                case 'a':
                    move_player(&player_1, -1, game_width);
                    break;
                case KEY_RIGHT:
                case 'D':
                case 'd':
                    move_player(&player_1, 1, game_width);
                    break;

                case ' ':
                    if (get_time_millies(clock() - cd) > 500) {
                        cd = clock();
                        player_fire(player_1, &bullets);
                    }
                    break;
            }
        }
    }

}
