#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define READ_MAX 1000
#define URL_MAX 100

struct node {
    // contains the name of the current url
    char url[READ_MAX];

    // pointer to next node
    struct node *next;

    // contains the url that it is pointing to in collection.txt
    char url_data[READ_MAX];

    // contains a double page rank
    double page_rank;

    // this is new page rank that is used when calculating the page rank
    double new_page_rank;

    // contains the number of urls from url_data
    int url_num;

    // pointer to the next node of the incoming nodes
    struct node *incoming;

    // contains the number of the index of the adjList from struct graph
    int adj_num;
};

struct graph {
    // adjacency list 
    struct node *adjList[URL_MAX];

    // contains the number of total vertices of the graph
    int num_vertices;
};


////////////////////////////////////////////////////////////////////////////////
void read_collection(char tokens[][READ_MAX], int *num_tokens);
struct graph *initialize_graph(int num);
void print_graph(struct graph *graph);
struct graph *update_graph(struct graph *curr, int num_vertices,
    char tokens[][READ_MAX]);
void read_text(struct graph *my_graph,int num_tokens);
void free_graph(struct graph *g);
struct graph* create_incoming_graph(struct graph *g);
void calculator(struct graph *g, struct graph *in, int num_vertices,
    double dampingFactor, double diffPR, int maxIterations);
void print_incoming_graph(struct graph *incoming_graph);
void insert_at_start(struct graph *g, struct graph *in);
void free_incoming(struct graph *g);
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
    // if less than 2 inputs then returns 0
    if (argc < 2) {
        printf("enter more than 2 argument");
        return 0;
    }
    
    // stores the urls
    char tokens[READ_MAX][READ_MAX];

    // countes the number of the urls that has been read
    int num_tokens = 0;

    // inputs used for calculating pagerank
    double dampingFactor = atof(argv[1]);
    double diffPR = atof(argv[2]);
    int maxIterations = atoi(argv[3]);

    // this function reads the txt file then puts the strings 
    read_collection(tokens, &num_tokens);

    // this function returns a graph that is initialized
    struct graph *my_graph = initialize_graph(num_tokens);

    // this function updates the graph with the urls 
    my_graph = update_graph(my_graph, num_tokens, tokens);

    // this function reads all of the url.txt files then puts into the graph
    read_text(my_graph, num_tokens);

    // this function creates the the incoming graph of incoming urls
    struct graph *incoming_graph = create_incoming_graph(my_graph);

    // inserts the current url at the start of the linked list
    insert_at_start(my_graph, incoming_graph);

    // function calculates the page rank
    calculator((my_graph, incoming_graph, num_tokens, dampingFactor, 
        diffPR, maxIterations));
   
    // used for printing both incoming grpah and graph
    // print_incoming_graph(incoming_graph);
    // print_graph(my_graph);
    
    // freeing 
    free_incoming(incoming_graph); 
    free_graph(my_graph);
    free(my_graph);
    free(incoming_graph);

    return 0;
}

void read_collection(char tokens[][READ_MAX], int *num_tokens) {

    // reading collection.txt file 
    FILE *fp;
    fp = fopen("collection.txt", "r");

    char buffer[READ_MAX];
    *num_tokens = 0;

    while (fscanf(fp, "%s", buffer) != EOF) {
        const char s[] = " \n";
        char *current_token = strtok(buffer, s);

        while (current_token != NULL) {
            
            
            size_t len = strcspn(current_token, "\n");  
            current_token[len] = '\0';  
            strcpy(tokens[*num_tokens], current_token);
            (*num_tokens)++;
            current_token = strtok(NULL, s);
        }
    }

    fclose(fp);
}

struct graph* initialize_graph(int num) {
    struct graph *temp = malloc(sizeof(struct graph));
    temp->num_vertices = num;

    // initialises the graph adjList to NULL
    for (int i = 0; i < num; i++) {
        temp->adjList[i] = NULL;  
    }

    // returns the graph
    return temp;
}


void print_graph(struct graph *graph) {

    // prints the graph
    for (int i = 0; i < graph->num_vertices; i++) {
        struct node *current = graph->adjList[i];
        printf("%s -> ", current->url);

        while (current->next != NULL) {
            current = current->next;
            printf("%s -> ", current->url);
        }

        printf("NULL\n");
    }
}


struct graph *update_graph((struct graph *curr, int num_vertices, 
    char tokens[][READ_MAX])) {

    for (int i = 0; i < num_vertices; i++) {
        struct node *new_node = malloc(sizeof(struct node));

        // puts in the url into every node and initialises the node
        strcpy(new_node->url, tokens[i]);
        new_node->url_num = 0;  
        new_node->adj_num = i;
        new_node->page_rank = 1.0 / num_vertices;  
        new_node->new_page_rank = 0.0;

        new_node->next = curr->adjList[i];
        curr->adjList[i] = new_node;
    }

    return curr;
}


void read_text(struct graph *my_graph, int num_tokens) {
    char buffer[READ_MAX];

    // reads all the url.txt files
    for (int i = 0; i < num_tokens; i++) {
        char *file_ext = ".txt";
        char filename[READ_MAX];
        strcpy(filename, my_graph->adjList[i]->url);
        strcat(filename, file_ext);

        // if file is NULL then exits
        FILE *fp = fopen(filename, "r");
        if (fp == NULL) {
            exit(EXIT_FAILURE);
        }

        int reading_section = 0;
        char *accumulated_text = malloc(1);  
        accumulated_text[0] = '\0';  

        // reads everything between "#start Section-1" and "#end Section-1"
        while (fgets(buffer, READ_MAX, fp) != NULL) {
            if (strncmp(buffer, "#start Section-1", 16) == 0) {
                reading_section = 1;
                continue;
            }

            if (reading_section) {
               
                if (strstr(buffer, "#end Section-1") != NULL) {
                    reading_section = 0;
                }

                // tokenises the urls
                else {
                    char *word = strtok(buffer, " \n");
                    
                    // while loops until NULL, inserts all the urls that is 
                    // contained in the txt file of the url
                    while (word != NULL) {
                        for (int j = 0; j < num_tokens; j++) {
                            if(strcmp(my_graph->adjList[j]->url, word) == 0) {
                                struct node *curr = malloc(sizeof(struct node));
                                my_graph->adjList[i]->url_num += 1;
                                curr->adj_num = j;
                                strcpy(curr->url, word);
                                curr->next = my_graph->adjList[i]->next;
                                my_graph->adjList[i]->next = curr;
                            }
                        }
                        word = strtok(NULL, " \n");
                    }
                }
            }
        }

        strcpy(my_graph->adjList[i]->url_data, accumulated_text);

        free(accumulated_text);

        fclose(fp);
    }
}

struct graph *create_incoming_graph(struct graph *g) {
    struct graph *incoming_graph = initialize_graph(g->num_vertices);

    for (int i = 0; i < g->num_vertices; i++) {
        struct node *current_node = g->adjList[i]->next;  

        while (current_node != NULL) {
            int incoming_vertex_index = current_node->adj_num;

            // puts in the incoming urls into the incoming graph and 
            // other variables
            if ((incoming_vertex_index >= 0 && 
                incoming_vertex_index < incoming_graph->num_vertices)) {

                struct node *incoming_node = malloc(sizeof(struct node));
                strcpy(incoming_node->url, g->adjList[i]->url);  
                incoming_node->url_num = g->adjList[i]->url_num;  
                incoming_node->adj_num = g->adjList[i]->adj_num;  
                ((incoming_node->next = 
                    incoming_graph->adjList[incoming_vertex_index]));
                incoming_graph->adjList[incoming_vertex_index] = incoming_node;
            }

            current_node = current_node->next;
        }
    }

    return incoming_graph;
}


void insert_at_start(struct graph *g, struct graph *in) {
    for (int i = 0; i < g->num_vertices; i++) {
        struct node *curr_g = g->adjList[i];
        struct node *curr_in = malloc(sizeof(struct node));

        // puts in the current url at the start of the incoming graph node
        strcpy(curr_in->url, curr_g->url);
        curr_in->url_num = curr_g->url_num;
        curr_in->adj_num = curr_g->adj_num;
        curr_in->next = in->adjList[i];

        in->adjList[i] = curr_in;
    }
}


void print_incoming_graph(struct graph *incoming_graph) {

    // prints the incoming graph
    for (int i = 0; i < incoming_graph->num_vertices; i++) {
        struct node *current = incoming_graph->adjList[i];

        if (current != NULL) {
            printf("%s -> ", current->url);

            while (current->next != NULL) {
                current = current->next;
                printf("%s -> ", current->url);
            }
        }
        printf("NULL\n");
    }
}

void free_incoming(struct graph *g) {

    // frees the incoming graph
    for (int i = 0;i < g->num_vertices; i++) {
        struct node *curr = g->adjList[i];
        while (curr != NULL) {
            struct node *temp = curr;
            curr = curr->next;
            free(temp);
        }
    }
}

void free_graph(struct graph *g) {

    // frees the graph
    for (int i = 0; i < g->num_vertices; i++) {
        struct node *current = g->adjList[i]; 
        while (current != NULL) {
            struct node *temp = current;
            current = current->next;
            free(temp);
        }

    }
}


void calculator(struct graph *g, struct graph *in, int num_vertices,
                double dampingFactor, double diffPR, int maxIterations) {
    double *pagerank = malloc(num_vertices * sizeof(double));
    int *sorted_indices = malloc(num_vertices * sizeof(int));

    for (int i = 0; i < num_vertices; i++) {
        pagerank[i] = 1 / num_vertices;
        sorted_indices[i] = i;
    }

    int iterations = 0;
    double diff = diffPR;
    
    while (iterations < maxIterations && diff >= diffPR) {
        double *new_pagerank = malloc(num_vertices * sizeof(double));
        
        // for loop based on i < number of vertices
        for (int i = 0; i < num_vertices; i++) {
            double sum = 0;
            struct node *incoming_node = in->adjList[i]->next;

            // while loops until incoming_node is NULL and calculates the sum
            while (incoming_node != NULL) {
                int j = incoming_node->adj_num;
                if (j >= 0 && j < num_vertices && g->adjList[j] != NULL) {
                    int out_degree = g->adjList[j]->url_num;
                    sum += pagerank[j] / out_degree;  
                }
                incoming_node = incoming_node->next;
            }

            // calculates the page rank for a url
            new_pagerank[i] = (((1- dampingFactor) / num_vertices) + 
                (dampingFactor * sum));
        }

        // sets new pagerank based on new page rank
        diff = diffPR;
        for (int i = 0; i < num_vertices; i++) {
            diff += fabs(new_pagerank[i] - pagerank[i]);
            pagerank[i] = new_pagerank[i];

            if (diff < diffPR) {
                break;
            }
        }

        free(new_pagerank);
        iterations++;
    }

    // orders the urls based on pagerank
    for (int i = 0; i < num_vertices - 1; i++) {
        for (int j = i + 1; j < num_vertices; j++) {
            if (pagerank[sorted_indices[i]] < pagerank[sorted_indices[j]]) {
                int temp = sorted_indices[i];
                sorted_indices[i] = sorted_indices[j];
                sorted_indices[j] = temp;
            }
        }
    }

    // creates a file that prints the url, url contains, pagerank
    FILE *fp = fopen("pagerankList.txt", "w");
    for (int i = 0; i < num_vertices; i++) {
        struct node *current_node = g->adjList[sorted_indices[i]];
        fprintf((fp, "%s, %d, %.7f\n", current_node->url, current_node->url_num, 
            pagerank[sorted_indices[i]]));
    }
    fclose(fp);

    free(sorted_indices);
    free(pagerank);
}