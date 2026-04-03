#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GENERIC ΥΛΟΠΟΙΗΣΗ ΔΥΝΑΜΙΚΟΙ ΠΙΝΑΚΕΣ

#define INITIAL_CAPACITY 10

typedef struct {
    void **items;  
    int size;      
    int capacity;  
} GenericArray;

typedef int (*CompareFunc)(const void *a, const void *b); 
typedef int (*MatchFunc)(const void *data, const void *key); 
typedef void (*PrintFunc)(const void *data); 
typedef void (*FreeDataFunc)(void *data); 


// Αρχικοποίηση πίνακα
void array_init(GenericArray *arr) {
    arr->size = 0;
    arr->capacity = INITIAL_CAPACITY;
    arr->items = malloc(arr->capacity * sizeof(void *));
}

void array_insert_sorted(GenericArray *arr, void *data, CompareFunc cmp) {
    // Έλεγχος χωρητικότητας
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->items = realloc(arr->items, arr->capacity * sizeof(void *));
    }

    // εισαγωγή
    int i = 0;
    while (i < arr->size && cmp(arr->items[i], data) < 0) {
        i++;
    }

    // Μετακίνηση στοιχείων 
    for (int j = arr->size; j > i; j--) {
        arr->items[j] = arr->items[j-1];
    }

    // τελικη εισαγωγή
    arr->items[i] = data;
    arr->size++;
}

// Δυαδική Αναζήτηση 
void* array_binary_search(GenericArray *arr, const void *key_wrapper, CompareFunc cmp) {
    int left = 0;
    int right = arr->size - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        int res = cmp(arr->items[mid], key_wrapper);

        if (res == 0) return arr->items[mid]; 
        if (res < 0) left = mid + 1;          
        else right = mid - 1;                 
    }
    return NULL; 
}

// Linear Search
void* array_search(GenericArray *arr, const void *key, MatchFunc match) {
    for (int i = 0; i < arr->size; i++) {
        if (match(arr->items[i], key)) {
            return arr->items[i];
        }
    }
    return NULL;
}

void array_display(GenericArray *arr, PrintFunc print) {
    if (arr->size == 0) printf(" (Array is empty)\n");
    for (int i = 0; i < arr->size; i++) {
        print(arr->items[i]);
    }
}

int array_delete(GenericArray *arr, const void *key, MatchFunc match, FreeDataFunc free_func) {
    int idx = -1;
    for (int i = 0; i < arr->size; i++) {
        if (match(arr->items[i], key)) {
            idx = i;
            break;
        }
    }

    if (idx == -1) return 0;

    if (free_func) free_func(arr->items[idx]);
    free(arr->items[idx]); 

    for (int j = idx; j < arr->size - 1; j++) {
        arr->items[j] = arr->items[j+1];
    }

    arr->size--;
    return 1;
}

// Απελευθέρωση μνήμης πίνακα
void array_free_all(GenericArray *arr, FreeDataFunc free_func) {
    for (int i = 0; i < arr->size; i++) {
        if (free_func) free_func(arr->items[i]);
        free(arr->items[i]);
    }
    free(arr->items);
    arr->items = NULL;
    arr->size = 0;
    arr->capacity = 0;
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

// CALLBACKS 

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

GenericArray authors_arr;
GenericArray books_arr;
GenericArray writes_arr;

int next_writer_id = 1;

void load_data(); 
void save_data();

int main(void) {
    array_init(&authors_arr);
    array_init(&books_arr);
    array_init(&writes_arr);

    load_data(); 

    if (authors_arr.size > 0) {
        author *last = (author*)authors_arr.items[authors_arr.size - 1];
        next_writer_id = last->writer_id + 1;
    }

    int choice;
    do {
        printf("\nMENU (Generic Dynamic Arrays)\n");
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
                
                array_insert_sorted(&authors_arr, new_a, cmp_author_id);
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

                // Linear Search
                author *found_a = (author*)array_search(&authors_arr, surnameBuf, match_author_surname);
                int curr_id;

                if (!found_a) {
                    printf("Author not found. Creating new...\n");
                    author *new_a = (author*)malloc(sizeof(author));
                    new_a->surname = strdup(surnameBuf); 
                    char nameBuf[100]; printf("Name: "); scanf("%99s", nameBuf);
                    new_a->name = strdup(nameBuf);       
                    new_a->writer_id = next_writer_id++;
                    new_a->num_of_books = 1;
                    array_insert_sorted(&authors_arr, new_a, cmp_author_id);
                    curr_id = new_a->writer_id;
                } else {
                    curr_id = found_a->writer_id;
                    found_a->num_of_books++;
                }

                array_insert_sorted(&books_arr, new_b, cmp_book_title);

                writes *new_w = (writes*)malloc(sizeof(writes));
                new_w->tittle = strdup(new_b->title); 
                new_w->writer_id = curr_id;
                array_insert_sorted(&writes_arr, new_w, cmp_writes);
                printf("Book recorded.\n");
                break;
            }
            case 3: { // Search Writer 
                char sname[100];
                printf("Enter Surname: "); scanf("%99s", sname);
                
                // Linear search
                author *a = (author*)array_search(&authors_arr, sname, match_author_surname);
                
                if (a) {
                    print_author(a);
                    printf("Books:\n");
                    for (int i = 0; i < writes_arr.size; i++) {
                        writes *w = (writes*)writes_arr.items[i];
                        if (w->writer_id == a->writer_id) {
                            // Binary Search για τα βιβλία (Title) 
                            book b_dummy; b_dummy.title = w->tittle;
                            book *b = (book*)array_binary_search(&books_arr, &b_dummy, cmp_book_title);
                            if(b) printf(" - %s (Year: %d)\n", b->title, b->release_date);
                        }
                    }
                } else printf("Not found.\n");
                break;
            }
            case 4: { // Search Book 
                char title[200];
                printf("Title: "); scanf("%199s", title);
                
                book key_dummy;
                key_dummy.title = title;

                book *b = (book*)array_binary_search(&books_arr, &key_dummy, cmp_book_title);
                if(b) {
                    print_book(b);
                    for (int i = 0; i < writes_arr.size; i++) {
                        writes *w = (writes*)writes_arr.items[i];
                        if(strcmp(w->tittle, title) == 0) {
                            author a_dummy; a_dummy.writer_id = w->writer_id;
                            //Binary Search για Author ID
                            author *a = (author*)array_binary_search(&authors_arr, &a_dummy, cmp_author_id);
                            if(a) printf(" - Author: %s %s\n", a->name, a->surname);
                        }
                    }
                } else printf("Book not found.\n");
                break;
            }
            case 5: { // Delete Author
                int id;
                printf("ID to delete: "); scanf("%d", &id);
                
                // Εύρεση με Binary Search 
                author key_dummy; key_dummy.writer_id = id;
                author *a = (author*)array_binary_search(&authors_arr, &key_dummy, cmp_author_id);
                
                if(a) {
                    printf("Processing deletion...\n");
                    for (int i = 0; i < writes_arr.size; ) {
                        writes *w = (writes*)writes_arr.items[i];
                        
                        if (w->writer_id == id) {
                            char *book_title = strdup(w->tittle); 
                            int count = 0;
                            for(int j=0; j<writes_arr.size; j++) {
                                if(strcmp(((writes*)writes_arr.items[j])->tittle, book_title) == 0) count++;
                            }
                            if(count == 1) {
                                printf(" - Deleting monograph: %s\n", book_title);
                                array_delete(&books_arr, book_title, match_book_title, free_book_data);
                            } else {
                                printf(" - Removed from co-authored book: %s\n", book_title);
                            }
                            writes key = { .tittle = book_title, .writer_id = id };
                            array_delete(&writes_arr, &key, match_writes_entry, free_writes_data);
                            free(book_title);
                        } else {
                            i++;
                        }
                    }
                    array_delete(&authors_arr, &id, match_author_id, free_author_data);
                    printf("Author deleted successfully.\n");
                } else {
                    printf("ID not found.\n");
                }
                break;
            }
            case 6: { // Delete Book
                char title[200];
                printf("Title to delete: "); scanf("%199s", title);
                
                book key_dummy; key_dummy.title = title;
                if (array_binary_search(&books_arr, &key_dummy, cmp_book_title)) {
                    for (int i = 0; i < writes_arr.size; ) {
                        writes *w = (writes*)writes_arr.items[i];
                        if(strcmp(w->tittle, title) == 0) {
                            author a_dummy; a_dummy.writer_id = w->writer_id;
                            author *a = (author*)array_binary_search(&authors_arr, &a_dummy, cmp_author_id);
                            if(a && a->num_of_books > 0) a->num_of_books--;
                            
                            writes key = { .tittle = title, .writer_id = w->writer_id };
                            array_delete(&writes_arr, &key, match_writes_entry, free_writes_data);
                        } else {
                            i++;
                        }
                    }
                    array_delete(&books_arr, title, match_book_title, free_book_data);
                    printf("Book deleted.\n");
                } else printf("Book not found.\n");
                break;
            }
            case 7: // Exit & Save
                save_data();
                printf("Saved. Bye!\n");
                array_free_all(&authors_arr, free_author_data);
                array_free_all(&books_arr, free_book_data);
                array_free_all(&writes_arr, free_writes_data);
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
                array_insert_sorted(&authors_arr, a, cmp_author_id);
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
                array_insert_sorted(&books_arr, b, cmp_book_title);
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
                array_insert_sorted(&writes_arr, w, cmp_writes);
            }
        }
        fclose(fw);
    }
}

void save_data() {
    FILE *fa = fopen("authors.txt", "w");
    if(fa) {
        fprintf(fa, "%d\n", authors_arr.size);
        for(int i=0; i<authors_arr.size; i++) {
            author *a = (author*)authors_arr.items[i];
            fprintf(fa, "%d %s %s %d\n", a->writer_id, a->surname, a->name, a->num_of_books);
        }
        fclose(fa);
    }
    FILE *fb = fopen("books.txt", "w");
    if(fb) {
        fprintf(fb, "%d\n", books_arr.size);
        for(int i=0; i<books_arr.size; i++) {
            book *b = (book*)books_arr.items[i];
            fprintf(fb, "%s %d %.2f\n", b->title, b->release_date, b->price);
        }
        fclose(fb);
    }
    FILE *fw = fopen("writes.txt", "w");
    if(fw) {
        fprintf(fw, "%d\n", writes_arr.size);
        for(int i=0; i<writes_arr.size; i++) {
            writes *w = (writes*)writes_arr.items[i];
            fprintf(fw, "%s %d\n", w->tittle, w->writer_id);
        }
        fclose(fw);
    }
}