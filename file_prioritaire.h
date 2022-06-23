#include <stdlib.h>

typedef struct element {
    int x, y;
    double heuristique;
    struct element *suivant;
} element;

typedef struct {
    element *premier;
} file_prioritaire;

int est_vide(file_prioritaire *fp) {
    return fp->premier ? 0 : 1;
}

void enfiler(file_prioritaire *fp, int x, int y, double heuristique) {
    element *tmp, *e, *n = (element *)malloc(sizeof(element));
    n->x = x;
    n->y = y;
    n->heuristique = heuristique;
    n->suivant = NULL;

    if (fp->premier) {
        if (fp->premier->x == x && fp->premier->y == y) {
            if (heuristique >= fp->premier->heuristique)
                return;
            else {
                tmp = fp->premier;
                fp->premier = fp->premier->suivant;
                free(tmp);
            }
        }
        else {
            e = fp->premier;
            while (e->suivant) {
                if (e->suivant->x == x && e->suivant->y == y) {
                    if (heuristique >= e->suivant->heuristique)
                        return;
                    else {
                        tmp = e->suivant;
                        e->suivant = e->suivant->suivant;
                        free(tmp);
                        break;
                    }
                }
                e = e->suivant;
            }
        }
    }

    if (!fp->premier || fp->premier->heuristique >= heuristique) {
        n->suivant = fp->premier;
        fp->premier = n;
    }
    else {
        e = fp->premier;
        while (e->suivant && e->suivant->heuristique < heuristique) {
            e = e->suivant;
        }
        n->suivant = e->suivant;
        e->suivant = n;
    }
}

element *defiler(file_prioritaire *fp) {
    element *e = fp->premier;
    fp->premier = fp->premier->suivant;
    return e;
}

void supprimer(file_prioritaire *fp) {
    element *tmp, *e = fp->premier;
    while (e) {
        tmp = e;
        e = e->suivant;
        free(tmp);
    }
    fp->premier = NULL;
}