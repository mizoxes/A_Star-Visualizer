#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <math.h>
#include "file_prioritaire.h"

#undef min
#define min(a, b) (a < b ? a : b)

typedef struct {
    int xParent, yParent;
    double g, h;
    double heuristique;
} Case;

TTF_Font *font;
SDL_Window *window;
SDL_Surface *screenSurface;
SDL_Surface *colorSurface, *boxSurface, *grassSurface, *stoneSurface;
int carte[15][15];
Case details[15][15];
file_prioritaire fp;
int d[8][2] = {{1, 0}, {0, -1}, {-1, 0}, {0, 1}, {1, -1}, {-1, -1}, {-1, 1}, {1, 1}};
int chemin_trouve;
int lx, ly;
int emplace = 0, supprime = 0;

enum { VIDE, VIDE_VISITE, MUR, DEBUT, DEBUT_VISITE, FIN, VOISIN, CHEMIN };
int couleurs[] = { 0xAFFFAF, 0x00FFFF3F, 0x654321, 0x0000FF, 0x000FF, 0xFF0000, 0xFFFF003F, 0xa006d4 };

void Initializer() {
    int i, j;
    for (i = 0; i < 15; i++) {
        for (j = 0; j < 15; j++) {
            if (carte[i][j] != MUR) carte[i][j] = VIDE;
            details[i][j].heuristique = details[i][j].xParent = -1;
        }
    }
    carte[14][0] = DEBUT;
    carte[0][14] = FIN;
    details[14][0].heuristique = details[14][0].g = 0;
    chemin_trouve = 0;
    lx = ly = 0;
    fp.premier = NULL;
    enfiler(&fp, 0, 14, 0);
}

void Dessiner() {
    int i, j;
    char s[4];
    SDL_Surface *text;
    for (i = 0; i < 15; i++) {
        for (j = 0; j < 15; j++) {
            SDL_Rect dest = { 41 * j, 41 * i, 40, 40 };
            if (carte[i][j] == MUR) {
                SDL_BlitSurface(boxSurface, NULL, screenSurface, &dest);
            }
            else if (carte[i][j] == VIDE) {
                SDL_BlitSurface(grassSurface, NULL, screenSurface, &dest);
            }
            else if (carte[i][j] == CHEMIN) {
                SDL_BlitSurface(stoneSurface, NULL, screenSurface, &dest);
            }
            else if (carte[i][j] == VOISIN || carte[i][j] == VIDE_VISITE) {
                SDL_BlitSurface(grassSurface, NULL, screenSurface, &dest);
                SDL_FillRect(colorSurface, NULL, couleurs[carte[i][j]]);
                SDL_BlitSurface(colorSurface, NULL, screenSurface, &dest);
                if (carte[i][j] == VOISIN) {
                    sprintf(s, "%.2lf", details[i][j].g);
                    text = TTF_RenderText_Solid(font, s, (SDL_Color){ 0, 0, 255 });
                    dest = (SDL_Rect){ 41 * j + 2, 41 * i, 40, 40 };
                    SDL_BlitSurface(text, NULL, screenSurface, &dest);
                    SDL_FreeSurface(text);

                    sprintf(s, "%.2lf", details[i][j].h);
                    text = TTF_RenderText_Solid(font, s, (SDL_Color){ 255, 0, 0 });
                    dest = (SDL_Rect){ 41 * j + 25, 41 * i, 40, 40 };
                    SDL_BlitSurface(text, NULL, screenSurface, &dest);
                    SDL_FreeSurface(text);

                    sprintf(s, "%.2lf", details[i][j].heuristique);
                    text = TTF_RenderText_Solid(font, s, (SDL_Color){ 255, 255, 255 });
                    dest = (SDL_Rect){ 41 * j + 10, 41 * i + 20, 40, 40 };
                    SDL_BlitSurface(text, NULL, screenSurface, &dest);
                    SDL_FreeSurface(text);
                }
            }
            else {
                SDL_FillRect(screenSurface, &dest, couleurs[carte[i][j]]);
            }
        }
    }
}

double Distance(int ax, int ay, int bx, int by) {
    return sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay));
}

void A_Star() {
    element *e;
    int dx, dy, x, y, x_voisin, y_voisin, i, j, tmp;
    double g, h, heuristique;

    if (lx == -1) {
        supprimer(&fp);
        Initializer();
        return;
    }

    if (est_vide(&fp) || (lx == 0 && ly == 14)) {
        if (est_vide(&fp)) {
            SDL_ShowSimpleMessageBox(0, "A*", "Aucun chemin a ete trouve", window);
            supprimer(&fp);
            Initializer();
        }
        else {
            for (i = 0; i < 15; i++)
                for (j = 0; j < 15; j++)
                    if (carte[i][j] == VIDE_VISITE || carte[i][j] == VOISIN)
                        carte[i][j] = VIDE;
            SDL_ShowSimpleMessageBox(0, "A*", "Chemin trouve", window);
            lx = -1;
        }
        return;
    }

    if (chemin_trouve) {
        if (lx != 14 || ly != 0)
            carte[ly][lx] = CHEMIN;
        tmp = lx;
        lx = details[ly][lx].xParent;
        ly = details[ly][tmp].yParent;
        return;
    }
    
    e = defiler(&fp);
    x = e->x;
    y = e->y;
    free(e);

    int dirs[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (i = 0; i < 8; i++) {
        if (i == 4 && (dirs[0] == 0 && dirs[1] == 0)) continue;
        else if (i == 5 && (dirs[1] == 0 && dirs[2] == 0)) continue;
        else if (i == 6 && (dirs[2] == 0 && dirs[3] == 0)) continue;
        else if (i == 7 && (dirs[3] == 0 && dirs[0] == 0)) continue;
        x_voisin = x + d[i][0];
        y_voisin = y + d[i][1];
        if (0 <= x_voisin && x_voisin < 15 && 0 <= y_voisin && y_voisin < 15 && (carte[y_voisin][x_voisin] == VIDE || carte[y_voisin][x_voisin] == VOISIN || carte[y_voisin][x_voisin] == FIN)) {
            dirs[i] = 1;
            if (x_voisin == 14 && y_voisin == 0) {
                carte[y][x] = VIDE_VISITE;
                details[y_voisin][x_voisin].xParent = x;
                details[y_voisin][x_voisin].yParent = y;
                chemin_trouve = 1;
                lx = 14;
                ly = 0;
                return;
            }
            g = details[y][x].g + Distance(x, y, x_voisin, y_voisin);
            h = Distance(x_voisin, y_voisin, 14, 0);
            heuristique = g + h;
            if (details[y_voisin][x_voisin].heuristique == -1 || heuristique < details[y_voisin][x_voisin].heuristique) {
                details[y_voisin][x_voisin].xParent = x;
                details[y_voisin][x_voisin].yParent = y;
                details[y_voisin][x_voisin].g = g;
                details[y_voisin][x_voisin].h = h;
                details[y_voisin][x_voisin].heuristique = heuristique;
                carte[y_voisin][x_voisin] = VOISIN;
                enfiler(&fp, x_voisin, y_voisin, heuristique);
            }
        }
    }
    if (x == 0 && y == 14) carte[y][x] = DEBUT_VISITE;
    else carte[y][x] = VIDE_VISITE;
}

int main(int argc, char *argv[]) {
    SDL_Event e;
    int quitter = 0;
    int i, j;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return -1;
    }

    if (!(window = SDL_CreateWindow("A* (Hamza EL KAICHE)", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 614, 614, SDL_WINDOW_SHOWN))) {
        SDL_Quit();
        return -1;
    }

    screenSurface = SDL_GetWindowSurface(window);

    colorSurface = SDL_CreateRGBSurface(0 , 40, 40 , 32 , 0xff000000 , 0x00ff0000 , 0x0000ff00 , 0x000000ff) ;
    boxSurface = SDL_LoadBMP("box.bmp");
    grassSurface = SDL_LoadBMP("grass.bmp");
    stoneSurface = SDL_LoadBMP("stone.bmp");
    if (boxSurface == NULL || grassSurface == NULL || stoneSurface == NULL) {
        SDL_Quit();
        return -1;
    }

    if (TTF_Init() < 0) {
        SDL_Quit();
        return -1;
    }
    
    font = TTF_OpenFont("arial.ttf", 10);
    if (font == NULL) {
        SDL_Quit();
        return -1;
    }

    Initializer();

    while (!quitter) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quitter = 1;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                i = e.button.y / 41;
                j = e.button.x / 41;
                if (carte[i][j] == VIDE) { emplace = 1; carte[i][j] = MUR; }
                else if (carte[i][j] == MUR) { supprime = 1; carte[i][j] = VIDE; }
            }
            else if (e.type == SDL_MOUSEBUTTONUP) {
                emplace = supprime = 0;
            }
            else if (e.type == SDL_MOUSEMOTION) {
                if (emplace) {
                    i = e.button.y / 41;
                    j = e.button.x / 41;
                    if (carte[i][j] == VIDE) carte[i][j] = MUR;
                }
                else if (supprime) {
                    i = e.button.y / 41;
                    j = e.button.x / 41;
                    if (carte[i][j] == MUR) carte[i][j] = VIDE;
                }
            }
            else if (e.type == SDL_KEYDOWN) {
                A_Star();
            }
        }

        SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0, 0, 0));

        Dessiner();

        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }

    TTF_CloseFont(font);
    TTF_Quit();
    SDL_FreeSurface(stoneSurface);
    SDL_FreeSurface(grassSurface);
    SDL_FreeSurface(boxSurface);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}