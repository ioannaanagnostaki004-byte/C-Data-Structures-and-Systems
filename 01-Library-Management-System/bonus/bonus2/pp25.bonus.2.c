#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

//GENERIC ΥΛΟΠΟΙΗΣΗ ΛΙΣΤΕΣ

typedef struct GenericNode {
    void *data;                
    struct GenericNode *next; 
} GenericNode;

// Callbacks
typedef int (*CompareFunc)(const void *a, const void *b); 
typedef int (*MatchFunc)(const void *data, const void *key); 
typedef void (*PrintFunc)(const void *data); 
typedef void (*FreeDataFunc)(void *data); 


void list_insert_sorted(GenericNode **head, void *data, CompareFunc cmp) {
    GenericNode *new_node = (GenericNode *)malloc(sizeof(GenericNode));
    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL || cmp((*head)->data, data) > 0) {
        new_node->next = *head;
        *head = new_node;
    } else {
        GenericNode *curr = *head;
        while (curr->next != NULL && cmp(curr->next->data, data) < 0) {
            curr = curr->next;
        }
        new_node->next = curr->next;
        curr->next = new_node;
    }
}

void* list_search(GenericNode *head, const void *key, MatchFunc match) {
    GenericNode *curr = head;
    while (curr != NULL) {
        if (match(curr->data, key)) return curr->data;
        curr = curr->next;
    }
    return NULL;
}

void list_display(GenericNode *head, PrintFunc print) {
    GenericNode *curr = head;
    if (curr == NULL) printf(" (List is empty)\n");
    while (curr != NULL) {
        print(curr->data);
        curr = curr->next;
    }
}

int list_delete(GenericNode **head, const void *key, MatchFunc match, FreeDataFunc free_func) {
    GenericNode *temp = *head, *prev = NULL;

    if (temp != NULL && match(temp->data, key)) {
        *head = temp->next;
        if (free_func) free_func(temp->data);
        free(temp->data);
        free(temp);
        return 1;
    }

    while (temp != NULL && !match(temp->data, key)) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) return 0;

    prev->next = temp->next;
    if (free_func) free_func(temp->data);
    free(temp->data);
    free(temp);
    return 1;
}

typedef struct book {
    char *title;
    int release_date;
    float price;
} book;

typedef struct author {
    int writer_id;
    char *surname;
    char *name;
    int num_of_books;
} author;

typedef struct writes {
    char *tittle;
    int writer_id;
} writes;

//CALLBACKS 
int cmp_author_id(const void *a, const void *b) {
    return ((author*)a)->writer_id - ((author*)b)->writer_id;
}

int cmp_book_title(const void *a, const void *b) {
    return strcmp(((book*)a)->title, ((book*)b)->title);
}

int cmp_writes(const void *a, const void *b) {
    writes *w1 = (writes*)a;
    writes *w2 = (writes*)b;
    if (w1->writer_id != w2->writer_id) 
        return w1->writer_id - w2->writer_id;
    return strcmp(w1->tittle, w2->tittle);
}

int match_author_id(const void *data, const void *key) {
    return ((author*)data)->writer_id == *(int*)key;
}
int match_author_surname(const void *data, const void *key) {
    return strcmp(((author*)data)->surname, (char*)key) == 0;
}
int match_book_title(const void *data, const void *key) {
    return strcmp(((book*)data)->title, (char*)key) == 0;
}
int match_writes_entry(const void *data, const void *key) {
    writes *w_data = (writes*)data;
    writes *w_key = (writes*)key;
    return (w_data->writer_id == w_key->writer_id && strcmp(w_data->tittle, w_key->tittle) == 0);
}

void print_author(const void *data) {
    author *a = (author*)data;
    printf("ID: %d | %s %s | Books: %d\n", a->writer_id, a->name, a->surname, a->num_of_books);
}
void print_book(const void *data) {
    book *b = (book*)data;
    printf("Title: %s | Year: %d | Price: %.2f\n", b->title, b->release_date, b->price);
}

void free_author_data(void *data) {
    author *a = (author*)data;
    free(a->name); free(a->surname);
}
void free_book_data(void *data) {
    book *b = (book*)data;
    free(b->title);
}
void free_writes_data(void *data) {
    writes *w = (writes*)data;
    free(w->tittle);
}

GenericNode *authors_head = NULL;
GenericNode *books_head = NULL;
GenericNode *writes_head = NULL;

int next_writer_id = 1;

void load_data(); 
void save_data();

int main(void) {
    load_data(); 

    // Υπολογισμός επόμενου ID
    if (authors_head != NULL) {
        GenericNode *curr = authors_head;
        while (curr->next != NULL) curr = curr->next;
        next_writer_id = ((author*)curr->data)->writer_id + 1;
    }

    int choice;
    do {
        printf("\n MENU (Generic Lists) \n");
        printf("1. Insert new writer\n");
        printf("2. Insert new book\n");
        printf("3. Search writer\n");
        printf("4. Search book\n");
        printf("5. Delete writer\n");
        printf("6. Delete book\n");
        printf("7. Exit & Save\n"); 
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: { // Insert Author
                author *new_a = (author*)malloc(sizeof(author));
                char buf1[100], buf2[100];
                printf("Surname: "); scanf("%99s", buf1);
                printf("Name: "); scanf("%99s", buf2);
                new_a->surname = strdup(buf1);
                new_a->name = strdup(buf2);
                new_a->writer_id = next_writer_id++;
                new_a->num_of_books = 0;
                
                list_insert_sorted(&authors_head, new_a, cmp_author_id);
                printf("Author added with ID: %d\n", new_a->writer_id);
                break;
            }
            case 2: { // Insert Book
                book *new_b = (book*)malloc(sizeof(book));
                char titleBuf[200], surnameBuf[100];
                printf("Title: "); scanf("%199s", titleBuf);
                printf("Date: "); scanf("%d", &new_b->release_date);
                printf("Price: "); scanf("%f", &new_b->price);
                printf("Author Surname: "); scanf("%99s", surnameBuf);
                new_b->title = strdup(titleBuf);

                // Αναζήτηση συγγραφέα
                author *found_a = (author*)list_search(authors_head, surnameBuf, match_author_surname);
                int curr_id;

                if (!found_a) {
                    printf("Author not found. Creating new...\n");
                    author *new_a = (author*)malloc(sizeof(author));
                    new_a->surname = strdup(surnameBuf);
                    char nameBuf[100]; printf("Name: "); scanf("%99s", nameBuf);
                    new_a->name = strdup(nameBuf);
                    new_a->writer_id = next_writer_id++;
                    new_a->num_of_books = 1;
                    list_insert_sorted(&authors_head, new_a, cmp_author_id);
                    curr_id = new_a->writer_id;
                } else {
                    curr_id = found_a->writer_id;
                    found_a->num_of_books++;
                }

                list_insert_sorted(&books_head, new_b, cmp_book_title);

                // Ενημέρωση writes
                writes *new_w = (writes*)malloc(sizeof(writes));
                new_w->tittle = strdup(new_b->title);
                new_w->writer_id = curr_id;
                list_insert_sorted(&writes_head, new_w, cmp_writes);
                printf("Book recorded.\n");
                break;
            }
            case 3: { // Search Author
                char sname[100];
                printf("Surname: "); scanf("%99s", sname);
                author *a = (author*)list_search(authors_head, sname, match_author_surname);
                
                if (a) {
                    print_author(a);
                    printf("Books:\n");
                    GenericNode *curr_w = writes_head;
                    while(curr_w) {
                        writes *w = (writes*)curr_w->data;
                        if (w->writer_id == a->writer_id) {
                            book *b = (book*)list_search(books_head, w->tittle, match_book_title);
                            if(b) printf(" - %s (Year: %d)\n", b->title, b->release_date);
                        }
                        curr_w = curr_w->next;
                    }
                } else printf("Not found.\n");
                break;
            }
            case 4: { // Search Book
                char title[200];
                printf("Title: "); scanf("%199s", title);
                book *b = (book*)list_search(books_head, title, match_book_title);
                if(b) {
                    print_book(b);
                    // Εύρεση συγγραφέων
                    GenericNode *curr_w = writes_head;
                    while(curr_w) {
                        writes *w = (writes*)curr_w->data;
                        if(strcmp(w->tittle, title) == 0) {
                            author *a = (author*)list_search(authors_head, &w->writer_id, match_author_id);
                            if(a) printf(" - Author: %s %s\n", a->name, a->surname);
                        }
                        curr_w = curr_w->next;
                    }
                } else printf("Book not found.\n");
                break;
            }
            case 5: { // Delete Author
                int id;
                printf("ID to delete: "); scanf("%d", &id);
                author *a = (author*)list_search(authors_head, &id, match_author_id);
                
                if(a) {
                    printf("Processing deletion...\n");
                    GenericNode *curr_w = writes_head; 
                    GenericNode *next_w; 

                    while(curr_w) {
                        next_w = curr_w->next;
                        writes *w = (writes*)curr_w->data;

                        if(w->writer_id == id) {
                            char *book_title = strdup(w->tittle);
                            
                            // Έλεγχος αν είναι μονογραφία
                            int count = 0;
                            GenericNode *check = writes_head;
                            while(check) {
                                if(strcmp(((writes*)check->data)->tittle, book_title) == 0) count++;
                                check = check->next;
                            }

                            if(count == 1) {
                                printf(" - Deleting monograph: %s\n", book_title);
                                list_delete(&books_head, book_title, match_book_title, free_book_data);
                            } else {
                                printf(" - Removed from co-authored book: %s\n", book_title);
                            }

                            // Διαγραφή από writes
                            writes key = { .tittle = book_title, .writer_id = id };
                            list_delete(&writes_head, &key, match_writes_entry, free_writes_data);
                            
                            free(book_title);
                        }
                        curr_w = next_w;
                    }
                    list_delete(&authors_head, &id, match_author_id, free_author_data);
                    printf("Author deleted successfully.\n");
                } else {
                    printf("ID not found.\n");
                }
                break;
            }
            case 6: { // Delete Book
                char title[200];
                printf("Title to delete: "); scanf("%199s", title);
                if (list_search(books_head, title, match_book_title)) {
                    // Ενημέρωση writes & authors
                    GenericNode *curr_w = writes_head;
                    GenericNode *next_w;
                    while(curr_w) {
                        next_w = curr_w->next;
                        writes *w = (writes*)curr_w->data;
                        if(strcmp(w->tittle, title) == 0) {
                            author *a = (author*)list_search(authors_head, &w->writer_id, match_author_id);
                            if(a && a->num_of_books > 0) a->num_of_books--;
                            
                            writes key = { .tittle = title, .writer_id = w->writer_id };
                            list_delete(&writes_head, &key, match_writes_entry, free_writes_data);
                        }
                        curr_w = next_w;
                    }
                    list_delete(&books_head, title, match_book_title, free_book_data);
                    printf("Book deleted.\n");
                } else printf("Book not found.\n");
                break;
            }
            case 7: // Exit & Save
                save_data();
                printf("Data saved. Exiting...\n");
                break;
        }
    } while (choice != 7);
    return 0;
}

void load_data() {
    FILE *fa = fopen("authors.txt", "r");
    if (fa) {
        int c; 
        if(fscanf(fa, "%d", &c) == 1) {
            for(int i=0; i<c; i++){
                author *a = (author*)malloc(sizeof(author));
                char b1[100], b2[100];
                fscanf(fa, "%d %s %s %d", &a->writer_id, b1, b2, &a->num_of_books);
                a->surname = strdup(b1); a->name = strdup(b2);
                list_insert_sorted(&authors_head, a, cmp_author_id);
            }
        }
        fclose(fa);
    }

    FILE *fb = fopen("books.txt", "r");
    if (fb) {
        int c; 
        if(fscanf(fb, "%d", &c) == 1) {
            for(int i=0; i<c; i++){
                book *b = (book*)malloc(sizeof(book));
                char buff[200];
                fscanf(fb, "%s %d %f", buff, &b->release_date, &b->price);
                b->title = strdup(buff);
                list_insert_sorted(&books_head, b, cmp_book_title);
            }
        }
        fclose(fb);
    }

    FILE *fw = fopen("writes.txt", "r");
    if (fw) {
        int c; 
        if(fscanf(fw, "%d", &c) == 1) {
            for(int i=0; i<c; i++){
                writes *w = (writes*)malloc(sizeof(writes));
                char buff[200];
                fscanf(fw, "%s %d", buff, &w->writer_id);
                w->tittle = strdup(buff);
                list_insert_sorted(&writes_head, w, cmp_writes);
            }
        }
        fclose(fw);
    }
}

void save_data() {
    FILE *fa = fopen("authors.txt", "w");
    if(fa) {
        int count = 0; GenericNode *curr = authors_head;
        while(curr) { count++; curr = curr->next; }
        fprintf(fa, "%d\n", count);
        
        curr = authors_head;
        while(curr) {
            author *a = (author*)curr->data;
            fprintf(fa, "%d %s %s %d\n", a->writer_id, a->surname, a->name, a->num_of_books);
            curr = curr->next;
        }
        fclose(fa);
    }

    FILE *fb = fopen("books.txt", "w");
    if(fb) {
        int count = 0; GenericNode *curr = books_head;
        while(curr) { count++; curr = curr->next; }
        fprintf(fb, "%d\n", count);

        curr = books_head;
        while(curr) {
            book *b = (book*)curr->data;
            fprintf(fb, "%s %d %.2f\n", b->title, b->release_date, b->price);
            curr = curr->next;
        }
        fclose(fb);
    }

    FILE *fw = fopen("writes.txt", "w");
    if(fw) {
        int count = 0; GenericNode *curr = writes_head;
        while(curr) { count++; curr = curr->next; }
        fprintf(fw, "%d\n", count);

        curr = writes_head;
        while(curr) {
            writes *w = (writes*)curr->data;
            fprintf(fw, "%s %d\n", w->tittle, w->writer_id);
            curr = curr->next;
        }
        fclose(fw);
    }
}