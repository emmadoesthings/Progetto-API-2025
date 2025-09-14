/*
PROVA FINALE ALGORITMI E PRINCIPI DELL'INFORMATICA A.A 24/25
Emma Roscioli

Versione finale, consegna 28/08/2025
Versione con commenti corretti, consegna 07/09/2025
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define max_piastr (2147483647 / 2)
#define BUFFER_SIZE 1024
#define maxrotte 5
#define maxvicini 6

/**
 * Strutture dati necessarie 
 */
typedef struct Esagono Esagono;
typedef struct Rotta Rotta;

struct Rotta {
    Esagono* es_dest;    
    int c_rotta;         
};

struct Esagono {
    int x, y;            
    int c_attrav;        
    Rotta rotte_aeree[maxrotte];
    int num_rotte; 
};

typedef struct {
    int x_vic;
    int y_vic;
} vicino;

typedef struct {
    int num_vicini;
    vicino vicini[maxvicini];
} ViciniCache;

typedef struct Grafo {
    Esagono** nodiAdiac;       
    int rows;
    int cols; 
    ViciniCache **vicini_cache;      
} Grafo;


const int vicini_xEVEN[6] = {+1, 0, -1, -1, -1, 0};
const int vicini_yEVEN[6] = {0, +1, +1, 0, -1, -1};
const int vicini_xODD[6]  = {+1, +1, 0, -1, 0, +1};
const int vicini_yODD[6]  = {0, +1, +1, 0, -1, -1};


/**
 * Inizializza il grafo che rappresenta la piastrellatura
 * @param rows è il numero di righe da allocare
 * @param cols è il numero di colonne da allocare
 * @param vicini_cache è la struttura dati dove memorizzare i vicini validi di ogni esagono
 */
Grafo* setGraphInit(int rows, int cols, ViciniCache *vicini_cache) {
    
     Grafo* graf = (Grafo*)malloc(sizeof(Grafo));
    graf->nodiAdiac = (Esagono**)malloc(rows * sizeof(Esagono*));
    graf->rows = rows;
    graf->cols = cols;

    graf->vicini_cache = (ViciniCache**)malloc(rows * sizeof(ViciniCache*));
    if (!graf->vicini_cache) {
        free(graf->nodiAdiac);
        free(graf);
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        graf->nodiAdiac[i] = (Esagono*)malloc(cols * sizeof(Esagono));
        graf->vicini_cache[i] = (ViciniCache*)malloc(cols * sizeof(ViciniCache));

        for (int j = 0; j < cols; j++) {
            graf->nodiAdiac[i][j].x = j;
            graf->nodiAdiac[i][j].y = i; 
            graf->nodiAdiac[i][j].c_attrav = 1;
            for(int h=0; h<5;h++){
                graf->nodiAdiac[i][j].rotte_aeree[h].c_rotta=-999;
                graf->nodiAdiac[i][j].rotte_aeree[h].es_dest=NULL;
            }
            graf->nodiAdiac[i][j].num_rotte = 0;

            int count = 0;
            for (int k = 0; k < 6; k++) {
                int x=0;
                int y=0;
                if (j % 2 == 0) {
                    x = i + vicini_xEVEN[k];
                    y = j + vicini_yEVEN[k];
                } else {
                    x = i + vicini_xODD[k];
                    y = j + vicini_yODD[k];
                }

                if (x >= 0 && y >= 0 && x < rows && y < cols) {
                    graf->vicini_cache[i][j].vicini[count].x_vic = x;
                    graf->vicini_cache[i][j].vicini[count].y_vic = y;
                    count++;
                }
            }
            graf->vicini_cache[i][j].num_vicini = count;
        }
    }

    return graf;
}

/**
 * Approssima un numero float all'intero più basso
 * @param x è il numero float da approssimare
 */
int floor_custom(float x) {
    if (x >= 0) {
        return (int)x;  
    } else if (x<0){
        int i = (int)x;
        if(i==x){
            return i;
        }
        else{
            i=i-1;
            return i;
        }
        
    }
    printf("qualche errore in floor\n");
    return -999;
}

/**
 * Controlla l'esistenza di una determinata rotta data la sua partenza e il suo arrivo
 * @param g è il grafo
 * @param x1 è la coordinata x dell'esagono di partenza
 * @param y1 è la coordinata y dell'esagono di partenza
 * @param x2 è la coordinata x dell'esagono di arrivo
 * @param y2 è la coordinata y dell'esagono di arrivo
 */
int checkRotta(Grafo* g, int x1, int y1, int x2, int y2){
    Esagono* part= &g->nodiAdiac[x1][y1];

    for (int h=0; h<part->num_rotte; h++){
        if(part->rotte_aeree[h].es_dest->x==x2 && part->rotte_aeree[h].es_dest->y==y2 ){
            return 1;
        }
    }
    return -1;

}

/**
 * Implementa il comando travel_cost usando un algoritmo con coda FIFO modificato per gestire anche costi variabili
 * @param g è il grafo
 * @param x1 è la coordinata x dell'esagono di partenza
 * @param y1 è la coordinata y dell'esagono di partenza
 * @param x2 è la coordinata x dell'esagono di arrivo
 * @param y2 è la coordinata y dell'esagono di arrivo
 * @param numRow è il numero totale di righe della piastrellatura
 * @param numCol è il numero totale di colonne della piastrellatura
 */
int travel_cost(Grafo* g, int x1, int y1, int x2, int y2, int numRow, int numCol) {

    int** c_percorso = (int**)malloc(numRow * sizeof(int*));
    if (!c_percorso){
        return -1;
    }
    for (int i = 0; i < numRow; i++) {
        c_percorso[i] = (int*)malloc(numCol * sizeof(int));
        for (int j = 0; j < numCol; j++) {
            c_percorso[i][j] = max_piastr;
        }
    }
    c_percorso[x1][y1] = 0;

    int numEs = numRow * numCol;
    int* codaX = (int*)malloc(20* numEs * sizeof(int));
    if (!codaX) return -1;
    int* codaY = (int*)malloc(20* numEs * sizeof(int));
    if (!codaY) return -1;
    int front = 0;
    int fine = 0;
    codaX[fine] = x1;
    codaY[fine] = y1;
    int nuova_distanza=0;
    fine++;
    int dimensione_coda=20*numEs;

    while (front < fine) {
            if (fine >= 3 * numEs) {
                printf("finito spazio nella coda :(\n");
                break;
            }
            if (fine >= dimensione_coda - 1) {
                dimensione_coda *= 2;
                codaX = realloc(codaX, dimensione_coda * sizeof(int));
                if (!codaX) {
                    
                    free(codaY);
                    for (int i = 0; i < numRow; i++) free(c_percorso[i]);
                    free(c_percorso);
                    return -1;
                }
                codaY = realloc(codaY, dimensione_coda * sizeof(int));
                
            }

        int x_nodo = codaX[front];
        int y_nodo = codaY[front++];
        ViciniCache *vicini = &g->vicini_cache[x_nodo][y_nodo];
        for (int i = 0; i < vicini->num_vicini; i++) {
            int x_vicino = vicini->vicini[i].x_vic;
            int y_vicino = vicini->vicini[i].y_vic;

            if ((g->nodiAdiac[x_vicino][y_vicino].c_attrav != 0 || (x_vicino == x2 && y_vicino == y2))) {
                    
                    if ((x_nodo == x1 && y_nodo == y1) || g->nodiAdiac[x_nodo][y_nodo].c_attrav > 0) {
                        if ((x_vicino == x2 && y_vicino == y2) || g->nodiAdiac[x_vicino][y_vicino].c_attrav > 0){

                        nuova_distanza = c_percorso[x_nodo][y_nodo] + g->nodiAdiac[x_nodo][y_nodo].c_attrav;

                        if (nuova_distanza < c_percorso[x_vicino][y_vicino]) {
                            c_percorso[x_vicino][y_vicino] = nuova_distanza;
                            codaX[fine] = x_vicino;
                            codaY[fine] = y_vicino;
                            fine++;
                        }

                    }
                    }
                    else{
                        
                    }
                
            }
            else{
               
            }
        }

        Esagono* es = &g->nodiAdiac[x_nodo][y_nodo];
        for (int i = 0; i < es->num_rotte; i++) {
            if (es->rotte_aeree[i].es_dest == NULL) continue;

            int x_nodoDest = es->rotte_aeree[i].es_dest->x;
            int y_nodoDest = es->rotte_aeree[i].es_dest->y;
            

            int new_dist_con_rotta = c_percorso[x_nodo][y_nodo] + es->rotte_aeree[i].c_rotta;

            if (x_nodoDest == x2 && y_nodoDest == y2) { 
                if (new_dist_con_rotta < c_percorso[x2][y2]) {
                    
                    c_percorso[x2][y2] = new_dist_con_rotta;
                }
                
            }
            else{
                if ((g->nodiAdiac[x_nodoDest][y_nodoDest].c_attrav != 0 || g->nodiAdiac[x_nodoDest][y_nodoDest].num_rotte!=0)
                && new_dist_con_rotta < c_percorso[x_nodoDest][y_nodoDest]) { 
                c_percorso[x_nodoDest][y_nodoDest] = new_dist_con_rotta;
                codaX[fine] = x_nodoDest;
                codaY[fine++] = y_nodoDest;
            }
            }

            
        }
    }


    int costoMin = 0;
    
    if (c_percorso[x2][y2] == max_piastr) {
       
        costoMin = -1;
    } else {
        costoMin = c_percorso[x2][y2];
       
        if(costoMin==0){
            costoMin=-1;
        }
    }

    
    free(codaX);
    free(codaY);
    for (int i = 0; i < numRow; i++){
        free(c_percorso[i]); 
    }
    free(c_percorso);

    
    return costoMin;
}


/**
 * Libera lo spazio in memoria allocato per il grafo della piastrellatura
 * @param graf è il grafo allocato in memoria
 */
void freeGrafo(Grafo* graf) {
    if (graf == NULL) {
        return;
    }
    if (graf->nodiAdiac != NULL && graf->vicini_cache != NULL) {
        for (int i = 0; i < graf->rows; i++) {
            if (graf->nodiAdiac[i] != NULL) {
                free(graf->nodiAdiac[i]);
                graf->nodiAdiac[i]=NULL;
            }
            if (graf->vicini_cache[i] != NULL) {
                free(graf->vicini_cache[i]);
                graf->vicini_cache[i]=NULL;
            }
        }
    }
    if (graf->vicini_cache != NULL) {
        free(graf->vicini_cache);
        graf->vicini_cache = NULL;
    }
    if (graf->nodiAdiac != NULL) {
        free(graf->nodiAdiac);
        graf->nodiAdiac = NULL;
    }
    free(graf);
    return;
}
 

/**
 * Implementa il comando change_cost. Cerca il percorso più breve tra due esagoni usando un algoritmo BFS
 * e cambia i costi secondo la formula richiesta
 * @param piastrellatura è il grafo
 * @param x è la coordinata x dell'esagono di partenza
 * @param y è la coordinata y dell'esagono di partenza
 * @param v è un dato del problema
 * @param raggio è il raggio dato dal comando
 * @param numRow è il numero di righe della piastrellatura
 * @param numCol è il numero di colonne della piastrellatura
 */
void change_costCONBFS(Grafo* piastrellatura, int x, int y, int v, int raggio, int numRow, int numCol) {
    int** nodi_visitati = (int**)malloc(numRow * sizeof(int*));
    for (int i = 0; i < numRow; i++) {
        nodi_visitati[i] = (int*)calloc(numCol, sizeof(int));
    }

    int* queue_x = (int*)malloc(numRow * numCol * sizeof(int));
    int* queue_y = (int*)malloc(numRow * numCol * sizeof(int));
    int* queue_dist = (int*)malloc(numRow * numCol * sizeof(int));
    int inizio = 0;
    int fine = 0;

    queue_x[fine] = x;
    queue_y[fine] = y;
    queue_dist[fine] = 0;
    fine++;
    nodi_visitati[x][y] = 1;

    while (inizio< fine) {
        int curr_x = queue_x[inizio];
        int curr_y = queue_y[inizio];
        int dist = queue_dist[inizio];
        inizio++;

        
        if (dist < raggio) { 
            
            float fraction = (float)(raggio - dist) / raggio;
            if(fraction<=0){
                fraction=0;
            }
            int delta = (int)floor_custom(v * fraction); 
            
            
                int new_cost = piastrellatura->nodiAdiac[curr_x][curr_y].c_attrav + delta;
                if(new_cost<0){new_cost=0;}
                if(new_cost>100){new_cost=100;}

                piastrellatura->nodiAdiac[curr_x][curr_y].c_attrav = new_cost;
            

            if(piastrellatura->nodiAdiac[curr_x][curr_y].num_rotte>0){
            int new_costo_rotta=0;
            for (int i = 0; i < piastrellatura->nodiAdiac[curr_x][curr_y].num_rotte; i++) { ///scorro le rotte
                new_costo_rotta=new_costo_rotta+piastrellatura->nodiAdiac[curr_x][curr_y].rotte_aeree[i].c_rotta;
            }
            new_costo_rotta=new_costo_rotta+new_cost;
            float costo_float=0;
            costo_float=new_costo_rotta/piastrellatura->nodiAdiac[curr_x][curr_y].num_rotte;
            new_costo_rotta=(int)floor_custom(costo_float);
            for (int i = 0; i < piastrellatura->nodiAdiac[curr_x][curr_y].num_rotte; i++) { ///scorro le rotte
                piastrellatura->nodiAdiac[curr_x][curr_y].rotte_aeree[i].c_rotta=new_costo_rotta;
            }
        }

        }


        if (dist + 1 < raggio){

        ViciniCache *vicini = &piastrellatura->vicini_cache[curr_x][curr_y]; 
        for (int i = 0; i < vicini->num_vicini; i++) {
            int vicino_x = vicini->vicini[i].x_vic;
            int vicino_y = vicini->vicini[i].y_vic;

            
            if (nodi_visitati[vicino_x][vicino_y]==0) { 
                nodi_visitati[vicino_x][vicino_y] = 1;
                queue_x[fine] = vicino_x;
                queue_y[fine] = vicino_y;
                queue_dist[fine] = dist + 1;
                fine++;
            }
        }
        }

    }

    for (int i = 0; i < numRow; i++) free(nodi_visitati[i]);
    free(nodi_visitati);
    free(queue_x);
    free(queue_y);
    free(queue_dist);
}


/**
 * Funzione main per la gestione dei comandi
 */
int main() {
    
    char command_type[20] = {0};
    char command[BUFFER_SIZE] = {0};
    Grafo* piastrellatura = NULL;
    ViciniCache *vicini_cache = NULL;
    int numRow=0;
    int numCol=0;

    /** Lettura del comando */
    while (fgets(command, BUFFER_SIZE, stdin) != NULL){ 
        command[strcspn(command, "\n")] = 0;  

        
        if (sscanf(command, "%19s", command_type) != 1) {
            
            exit(EXIT_FAILURE);
        }
    

    if (strcmp(command_type, "init")==0){
        /** Gestione del comando init */
            if (sscanf(command, "%*s %d %d", &numRow, &numCol) == 2){
                
                if(piastrellatura==NULL){
                    /** Caso in cui la piastrellatura non è mai stata inizializzata */
                    piastrellatura=setGraphInit(numRow,numCol, vicini_cache);
                }
                else{
                    /** Caso in cui c'è già una piastrellatura esistente */
                    freeGrafo(piastrellatura);
                    piastrellatura=NULL;
                    piastrellatura=setGraphInit(numRow,numCol, vicini_cache);
                }
                
                
            if (piastrellatura !=NULL){
                
                printf("OK\n");
            }
            }
            

    }
    else if (strcmp(command_type, "change_cost")==0){
        /** Gestione del comando change_cost */
        if (piastrellatura == NULL || numRow <= 0 || numCol <= 0) {
            printf("KO\n");  
            continue;
        }
        
        int raggio=-1;
        int v=-1;
        int x=-1;
        int y=-1;;
        if (sscanf(command, "%*s %d %d %d %d", &x, &y, &v, &raggio) != 4){
            /** Caso sintassi comando errata */
            printf("KO\n");
            continue; 
        }
        if (raggio <= 0 || x < 0 || y < 0 || x >= numRow || y >= numCol || v < -10 || v > 10) {
            /** Caso dati non validi */
            printf("KO\n");
            continue;
        }

        
        change_costCONBFS(piastrellatura, x, y, v, raggio, numRow, numCol);
        printf("OK\n");
    }
    else if (strcmp(command_type, "travel_cost")==0){
        /** Gestione del comando travel_cost */
        if (piastrellatura == NULL || numRow <= 0 || numCol <= 0) {
            /** Caso dati non validi */
            printf("-1\n");  
            continue;
        }
        
        
        
        int x1=-1;
        int y1=-1;
        int x2=-1;
        int y2=-1;
        if (sscanf(command, "%*s %d %d %d %d", &x1, &y1, &x2, &y2) != 4){
            /** Caso sintassi comando errata */
            return 1;
        }


       
        
    
        if(x1 < 0 || x1 >= numRow || y1 < 0 || y1 >= numCol || x2 < 0 || x2 >= numRow || y2 < 0 || y2 >= numCol || piastrellatura == NULL){
            /** Caso dati non validi */
            printf("-1\n");
        }
        else if (x1==x2 && y1==y2){
            /** Caso destinazione coincidente con partenza */
            printf("0\n");
        }
        else{
            
            int costo=0;
            
            costo=travel_cost(piastrellatura,x1,y1,x2,y2, numRow, numCol);
            
            printf("%d\n", costo);
            

        }
        
    }
    else if (strcmp(command_type, "toggle_air_route")==0){
        /** Gestione comando toggle_air_route */
        if (piastrellatura == NULL || numRow <= 0 || numCol <= 0) {
            /** Caso dati non validi */
            printf("KO\n"); 
            continue;
        }
        int x1=-1;
        int y1=-1;
        int x2=-1;
        int y2=-1;
        int esiste=0;
        if (sscanf(command, "%*s %d %d %d %d", &x1, &y1, &x2, &y2) != 4){
           /** Caso sintassi comando errata */
            return 1;
        }


        if (x1<0 || y1<0 || x1>=numRow || y1>=numCol || x2<0 || y2<0 || x2>=numRow || y2>=numCol){
            /** Caso dati comando non validi */
            printf("KO\n");
        } else if(x1==x2 && y1==y2){
            /** Caso destinazione coincidente con partenza */
            printf("KO\n");
        }
        else{
            
            esiste = checkRotta(piastrellatura, x1,y1,x2,y2);
            if (esiste==1){
                
                Esagono* es = &piastrellatura->nodiAdiac[x1][y1];
               for (int i = 0; i < es->num_rotte; i++) {

                if (es->rotte_aeree[i].es_dest == &piastrellatura->nodiAdiac[x2][y2]) {

                    es->rotte_aeree[i] = es->rotte_aeree[es->num_rotte - 1];
                    es->rotte_aeree[es->num_rotte - 1].es_dest = NULL;
                    es->rotte_aeree[es->num_rotte - 1].c_rotta = -999;
                    es->num_rotte--;
                    printf("OK\n");
                    break;
                }

            }
               

               
                
            }
            else if(esiste==-1){
                
                
               
                Esagono* es= &piastrellatura->nodiAdiac[x1][y1];
                
                if (es->num_rotte >= 5) {
                    /** Caso numero massimo di rotte raggiunto */
                    printf("KO\n");
                } else {
                    
                    int costo_tot = 0;
                    int costo_medio = es->c_attrav;
                    Rotta* rotta = es->rotte_aeree;
                    if (es->num_rotte==0){
                        
                        costo_medio = es->c_attrav;
                    }
                    else if (es->num_rotte > 0) { 
                        
                        for(int g=0;g<es->num_rotte;g++) {
                            costo_tot += rotta[g].c_rotta;
                        }
                        
                        costo_medio = (costo_tot+costo_medio) / (es->num_rotte + 1);
                    }
                    es->rotte_aeree[es->num_rotte].es_dest = &piastrellatura->nodiAdiac[x2][y2];
                    es->rotte_aeree[es->num_rotte].es_dest->x = x2;
                    es->rotte_aeree[es->num_rotte].es_dest->y = y2;
                    es->rotte_aeree[es->num_rotte].c_rotta = costo_medio;
                    es->num_rotte++;
                    
                    printf("OK\n");
                }
                

            }
            else{ printf("errore\n");}

        
        }
        
    }
    else{
        printf("comando invalido!");
    }
    
}

/** Libera lo spazio allocato per la piastrellatura */
freeGrafo(piastrellatura);

return 0;
   
}