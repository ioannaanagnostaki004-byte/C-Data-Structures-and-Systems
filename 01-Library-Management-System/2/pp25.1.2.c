#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct book {
    char *title;
    int release_date;
    float price;
    char *temp_surname; 
} book;

typedef struct author {
    int writer_id;
    char *surname;
    char *name;
    int num_of_books;
} author;

typedef struct writes {
    char *title;
    int writer_id;
} writes;

// Node Structures
struct BookNode {
    struct book data;       
    struct BookNode *next;  
};

struct AuthorNode {
    struct author data;     
    struct AuthorNode *next;
};

struct WritesNode {
    struct writes data;     
    struct WritesNode *next;
};

struct BookNode *books_head = NULL;
struct AuthorNode *authors_head = NULL;
struct WritesNode *writes_head = NULL;

int bookcounter = 0;
int authorcounter = 0;
int writescounter = 0;

int next_writer_id = 1;

void load_data();
void save_data();

// Συναρτήσεις εισαγωγής (δημιουργούν Nodes)
void insert_author_sorted(struct author new_author_data);
void insert_book_sorted(struct book new_book_data);
void insert_writes_sorted(struct writes new_write_data);

// Συναρτήσεις αναζήτησης (Επιστρέφουν δείκτη στον ΚΟΜΒΟ - Node*)
struct AuthorNode* find_author_by_id(int id);
struct BookNode* find_book_by_title(const char *title);
struct AuthorNode* find_author_by_surname(const char *surname);

void delete_author_node(int id);
void delete_book_node(const char *title);
void delete_writes_entry(const char *title, int writer_id); 

int main(void) {
    load_data(); 

    if (authors_head != NULL) {
        struct AuthorNode *curr = authors_head;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        next_writer_id = curr->data.writer_id + 1;
    }

    int choice;
    do {
        printf("\nMENU (Linked List Structure)\n");
        printf("1. Insert new writer record\n");
        printf("2. Insert new book record\n");
        printf("3. Search a writer record\n");
        printf("4. Search a book record\n");
        printf("5. Delete a writer record\n");
        printf("6. Delete a book record\n");
        printf("7. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: { // Insert Author
                struct author new_a; 
                char buf1[100], buf2[100];
                
                printf("Surname: "); scanf("%99s", buf1);
                printf("Name: "); scanf("%99s", buf2);
                
                new_a.surname = strdup(buf1);
                new_a.name = strdup(buf2);
                new_a.writer_id = next_writer_id++;
                new_a.num_of_books = 0;
                
                insert_author_sorted(new_a); // Περνάμε το data struct στη λίστα
                printf("Author added with ID: %d\n", new_a.writer_id);
                break;
            }
            case 2: { // Insert Book
                struct book new_b;
                char titleBuf[200], surnameBuf[100];
                
                printf("Book Title: "); scanf("%199s", titleBuf);
                printf("Release Date: "); scanf("%d", &new_b.release_date);
                printf("Price: "); scanf("%f", &new_b.price);
                printf("Author Surname: "); scanf("%99s", surnameBuf);
                
                new_b.title = strdup(titleBuf);
                new_b.temp_surname = NULL; 

                // Έλεγχος αν υπάρχει ο συγγραφέας
                struct AuthorNode *found_node = find_author_by_surname(surnameBuf);
                int current_writer_id;

                if (found_node == NULL) {
                    printf("Author not found. Creating new...\n");
                    struct author new_a;
                    new_a.surname = strdup(surnameBuf);
                    
                    printf("Give Author Name: "); 
                    char nameBuf[100]; scanf("%99s", nameBuf);
                    new_a.name = strdup(nameBuf);
                    
                    new_a.writer_id = next_writer_id++;
                    new_a.num_of_books = 1; 
                    
                    insert_author_sorted(new_a);
                    current_writer_id = new_a.writer_id;
                    printf("New author created (ID: %d)\n", current_writer_id);
                } else {
                    current_writer_id = found_node->data.writer_id;
                    found_node->data.num_of_books++;
                }

                insert_book_sorted(new_b);

                // Ενημέρωση writes
                struct writes new_w;
                new_w.title = strdup(new_b.title);
                new_w.writer_id = current_writer_id;
                insert_writes_sorted(new_w);
                
                printf("Book recorded successfully.\n");
                break;
            }
            case 3: { // Search Author
                char sname[100];
                printf("Enter Surname: "); scanf("%99s", sname);
                
                int found = 0;
                // Διάσχιση λίστας AuthorNode
                struct AuthorNode *curr_a = authors_head;
                while (curr_a != NULL) {
                    if (strcmp(curr_a->data.surname, sname) == 0) {
                        found = 1;
                        printf("\nFound: %s %s (ID: %d)\n", curr_a->data.name, curr_a->data.surname, curr_a->data.writer_id);
                        printf("Books:\n");
                        
                        int has_books = 0;
                        // Διάσχιση λίστας WritesNode
                        struct WritesNode *curr_w = writes_head;
                        while (curr_w != NULL) {
                            if (curr_w->data.writer_id == curr_a->data.writer_id) {
                                struct BookNode *b_node = find_book_by_title(curr_w->data.title);
                                if (b_node != NULL) {
                                    printf(" - %s (Year: %d, Price: %.2f)\n", 
                                           b_node->data.title, b_node->data.release_date, b_node->data.price);
                                    has_books = 1;
                                }
                            }
                            curr_w = curr_w->next;
                        }
                        if (!has_books) printf(" - No books found.\n");
                    }
                    curr_a = curr_a->next;
                }
                if (!found) printf("No author found with surname '%s'.\n", sname);
                break;
            }
            case 4: { // Search Book
                char title[200];
                printf("Enter Title: "); scanf("%199s", title);
                
                struct BookNode *b_node = find_book_by_title(title);
                if (b_node != NULL) {
                    printf("\nBook: %s\nYear: %d\nPrice: %.2f\nAuthors:\n", 
                           b_node->data.title, b_node->data.release_date, b_node->data.price);
                    
                    int found_author = 0;
                    struct WritesNode *curr_w = writes_head;
                    while (curr_w != NULL) {
                        if (strcmp(curr_w->data.title, title) == 0) {
                            struct AuthorNode *a_node = find_author_by_id(curr_w->data.writer_id);
                            if (a_node != NULL) {
                                printf(" - %s %s\n", a_node->data.name, a_node->data.surname);
                                found_author = 1;
                            }
                        }
                        curr_w = curr_w->next;
                    }
                    if (!found_author) printf(" - Unknown author.\n");
                } else {
                    printf("Book not found.\n");
                }
                break;
            }
            case 5: { // Delete Author
                int id;
                printf("Enter Writer ID to delete: "); scanf("%d", &id);
                struct AuthorNode *a_node = find_author_by_id(id);
                
                if (a_node == NULL) {
                    printf("Author ID not found.\n");
                } else {
                    printf("Processing deletion for Author ID: %d...\n", id);
                    
                    // Έλεγχος βιβλίων μέσω της λίστας WritesNode
                    struct WritesNode *curr_w = writes_head;
                    struct WritesNode *next_w; 

                    while (curr_w != NULL) {
                        next_w = curr_w->next; 

                        if (curr_w->data.writer_id == id) {
                            char *curr_title = strdup(curr_w->data.title);
                            
                            int count_authors = 0;
                            struct WritesNode *check_w = writes_head;
                            while (check_w != NULL) {
                                if (strcmp(check_w->data.title, curr_title) == 0) count_authors++;
                                check_w = check_w->next;
                            }
                            
                            if (count_authors == 1) { 
                                printf("Deleting monograph: %s\n", curr_title);
                                delete_book_node(curr_title);
                            } else {
                                printf(" - Removing author from co-authored book: %s\n", curr_title);
                            }
                            
                            // Διαγραφή της εγγραφής writes
                            delete_writes_entry(curr_title, id);
                            free(curr_title);
                        }
                        curr_w = next_w;
                    }
                    
                    delete_author_node(id);
                    printf("Author record deleted successfully.\n");
                }
                break;
            }
            case 6: { // Delete Book
                char title[200];
                printf("Enter Title to delete: "); scanf("%199s", title);
                struct BookNode *b_node = find_book_by_title(title);
                
                if (b_node == NULL) {
                    printf("Book not found.\n");
                } else {
                    printf("Deleting book: %s\n", title);
                    
                    // Ενημέρωση Writes και Author count
                    struct WritesNode *curr_w = writes_head;
                    struct WritesNode *next_w;

                    while (curr_w != NULL) {
                        next_w = curr_w->next;
                        if (strcmp(curr_w->data.title, title) == 0) {
                            int w_id = curr_w->data.writer_id;
                            struct AuthorNode *a_node = find_author_by_id(w_id);
                            
                            if (a_node != NULL && a_node->data.num_of_books > 0) {
                                a_node->data.num_of_books--;
                            }
                            
                            delete_writes_entry(title, w_id);
                        }
                        curr_w = next_w;
                    }
                    
                    delete_book_node(title);
                    printf("Book record deleted successfully.\n");
                }
                break;
            }
            case 7: 
                save_data();
                printf("Data saved. Exiting...\n");
                break;
        }
    } while (choice != 7);

    return 0;
}

// ΥΛΟΠΟΙΗΣΗ ΣΥΝΑΡΤΗΣΕΩΝ

void insert_author_sorted(struct author new_author_data) {
    // Δημιουργία Node 
    struct AuthorNode *new_node = (struct AuthorNode*)malloc(sizeof(struct AuthorNode));
    new_node->data = new_author_data; // Αντιγραφή δεδομένων στο Node
    new_node->next = NULL;

    authorcounter++;

    // Εισαγωγή στη λίστα 
    if (authors_head == NULL || authors_head->data.writer_id > new_node->data.writer_id) {
        new_node->next = authors_head;
        authors_head = new_node;
    } else {
        struct AuthorNode *curr = authors_head;
        while (curr->next != NULL && curr->next->data.writer_id < new_node->data.writer_id) {
            curr = curr->next;
        }
        new_node->next = curr->next;
        curr->next = new_node;
    }
}

void insert_book_sorted(struct book new_book_data) {
    struct BookNode *new_node = (struct BookNode*)malloc(sizeof(struct BookNode));
    new_node->data = new_book_data;
    new_node->next = NULL;

    bookcounter++;

    if (books_head == NULL || strcmp(books_head->data.title, new_node->data.title) > 0) {
        new_node->next = books_head;
        books_head = new_node;
    } else {
        struct BookNode *curr = books_head;
        while (curr->next != NULL && strcmp(curr->next->data.title, new_node->data.title) < 0) {
            curr = curr->next;
        }
        new_node->next = curr->next;
        curr->next = new_node;
    }
}

void insert_writes_sorted(struct writes new_write_data) {
    struct WritesNode *new_node = (struct WritesNode*)malloc(sizeof(struct WritesNode));
    new_node->data = new_write_data;
    new_node->next = NULL;

    writescounter++;

    int head_check = 0;
    if (writes_head == NULL) {
        writes_head = new_node;
        return;
    }

    if (writes_head->data.writer_id > new_node->data.writer_id) { 
        head_check = 1;
    } else if (writes_head->data.writer_id == new_node->data.writer_id && strcmp(writes_head->data.title, new_node->data.title) > 0){ 
        head_check = 1;
    }

    if (head_check) { //αν head_check ειναι 1
        new_node->next = writes_head;
        writes_head = new_node;
    } else {
        struct WritesNode *curr = writes_head;
        while (curr->next != NULL) {
            if (curr->next->data.writer_id > new_node->data.writer_id) {
                break;
            }
            if (curr->next->data.writer_id == new_node->data.writer_id && strcmp(curr->next->data.title, new_node->data.title) > 0) {
                    break;
                }
            curr = curr->next;
        }
        new_node->next = curr->next;
        curr->next = new_node;
    }
}

// Linear Search 
struct AuthorNode* find_author_by_id(int id) {
    struct AuthorNode *curr = authors_head;
    while (curr != NULL) {
        if (curr->data.writer_id == id) return curr;
        curr = curr->next;
    }
    return NULL;
}

struct AuthorNode* find_author_by_surname(const char *surname) {
    struct AuthorNode *curr = authors_head;
    while (curr != NULL) {
        if (strcmp(curr->data.surname, surname) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}

struct BookNode* find_book_by_title(const char *title) {
    struct BookNode *curr = books_head;
    while (curr != NULL) {
        if (strcmp(curr->data.title, title) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}

// Διαγραφή Κόμβων
void delete_author_node(int id) {
    struct AuthorNode *temp = authors_head, *prev = NULL;

    if (temp != NULL && temp->data.writer_id == id) {
        authors_head = temp->next;
        free(temp->data.name);
        free(temp->data.surname);
        free(temp);
        authorcounter--;
        return;
    }

    while (temp != NULL && temp->data.writer_id != id) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) return;

    prev->next = temp->next;
    free(temp->data.name);
    free(temp->data.surname);
    free(temp);
    authorcounter--;
}

void delete_book_node(const char *title) {
    struct BookNode *temp = books_head, *prev = NULL;

    if (temp != NULL && strcmp(temp->data.title, title) == 0) {
        books_head = temp->next;
        free(temp->data.title);
        free(temp);
        bookcounter--;
        return;
    }

    while (temp != NULL && strcmp(temp->data.title, title) != 0) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) return;

    prev->next = temp->next;
    free(temp->data.title);
    free(temp);
    bookcounter--;
}

void delete_writes_entry(const char *title, int writer_id) {
    struct WritesNode *temp = writes_head, *prev = NULL;

    if (temp != NULL && temp->data.writer_id == writer_id && strcmp(temp->data.title, title) == 0) {
        writes_head = temp->next;
        free(temp->data.title);
        free(temp);
        writescounter--;
        return;
    }

    while (temp != NULL) {
        if (temp->data.writer_id == writer_id && strcmp(temp->data.title, title) == 0) break;
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) return;

    prev->next = temp->next;
    free(temp->data.title);
    free(temp);
    writescounter--;
}

void load_data() {
    FILE *fa = fopen("authors.txt", "r");
    if (fa) {
        int count;
        if (fscanf(fa, "%d", &count) == 1) {
            for (int i = 0; i < count; i++) {
                struct author a; 
                char buff1[100], buff2[100];
                fscanf(fa, "%d %s %s %d", &a.writer_id, buff1, buff2, &a.num_of_books);
                a.surname = strdup(buff1);
                a.name = strdup(buff2);
                insert_author_sorted(a); 
            }
        }
        fclose(fa);
    }

    FILE *fb = fopen("books.txt", "r");
    if (fb) {
        int count;
        if (fscanf(fb, "%d", &count) == 1) {
            for (int i = 0; i < count; i++) {
                struct book b;
                char buff[200];
                fscanf(fb, "%s %d %f", buff, &b.release_date, &b.price);
                b.title = strdup(buff);
                b.temp_surname = NULL;
                insert_book_sorted(b);
            }
        }
        fclose(fb);
    }

    FILE *fw = fopen("writes.txt", "r");
    if (fw) {
        int count;
        if (fscanf(fw, "%d", &count) == 1) {
            for (int i = 0; i < count; i++) {
                struct writes w;
                char buff[200];
                fscanf(fw, "%s %d", buff, &w.writer_id);
                w.title = strdup(buff);
                insert_writes_sorted(w);
            }
        }
        fclose(fw);
    }
}

void save_data() {
    FILE *fa = fopen("authors.txt", "w");
    if (fa) {
        fprintf(fa, "%d\n", authorcounter);
        struct AuthorNode *curr = authors_head;
        while (curr != NULL) {
            fprintf(fa, "%d %s %s %d\n", curr->data.writer_id, curr->data.surname, curr->data.name, curr->data.num_of_books);
            curr = curr->next;
        }
        fclose(fa);
    }

    FILE *fb = fopen("books.txt", "w");
    if (fb) {
        fprintf(fb, "%d\n", bookcounter);
        struct BookNode *curr = books_head;
        while (curr != NULL) {
            fprintf(fb, "%s %d %.2f\n", curr->data.title, curr->data.release_date, curr->data.price);
            curr = curr->next;
        }
        fclose(fb);
    }

    FILE *fw = fopen("writes.txt", "w");
    if (fw) {
        fprintf(fw, "%d\n", writescounter);
        struct WritesNode *curr = writes_head;
        while (curr != NULL) {
            fprintf(fw, "%s %d\n", curr->data.title, curr->data.writer_id);
            curr = curr->next;
        }
        fclose(fw);
    }
}