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
    char *tittle; 
    int writer_id;
} writes;

//  ΚΑΘΟΛΙΚΕΣ ΜΕΤΑΒΛΗΤΕΣ 
struct book *books_arr = NULL;
int bookcounter = 0;
struct author *authors_arr = NULL;
int authorcounter = 0;
struct writes *writes_arr = NULL;
int writescounter = 0;

int next_writer_id = 1; 

void load_data();
void save_data();
//χρησιμοποιούν τις καθολικές μεταβλητές ως pointers
void insert_author_sorted(struct author new_author);
void insert_book_sorted(struct book new_book);
void insert_writes_sorted(struct writes new_write);
// Συναρτήσεις αναζήτησης
int binary_search_author_id(struct author *arr, int n, int id);
int binary_search_book_title(struct book *arr, int n, const char *title);
int find_author_by_surname(struct author *arr, int n, const char *surname); // Linear search (πίνακας ταξινομημένος κατά ID)

int main(void) {
    load_data();  

    if (authorcounter > 0) {
        next_writer_id = authors_arr[authorcounter - 1].writer_id + 1;
    }

    int choice;
    do {
        printf("\nMENU\n");
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

                
                insert_author_sorted(new_a);
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
                int author_idx = find_author_by_surname(authors_arr, authorcounter, surnameBuf);
                int current_writer_id;

                if (author_idx == -1) {
                    printf("Author not found. Creating new...\n");
                    struct author new_a;
                    new_a.surname = strdup(surnameBuf);
                    
                    printf("Give Author Name: "); 
                    char nameBuf[100]; scanf("%99s", nameBuf);
                    new_a.name = strdup(nameBuf);
                    
                    new_a.writer_id = next_writer_id++;
                    new_a.num_of_books = 1; // Θα έχει το βιβλίο που μόλις βάλαμε
                    
                    insert_author_sorted(new_a);
                    current_writer_id = new_a.writer_id;
                    printf("New author created (ID: %d)\n", current_writer_id);
                } else {
                    current_writer_id = authors_arr[author_idx].writer_id;
                    authors_arr[author_idx].num_of_books++;
                }

                insert_book_sorted(new_b);

                // Ενημέρωση writes_arr
                struct writes new_w;
                new_w.tittle = strdup(new_b.title);
                new_w.writer_id = current_writer_id;
                insert_writes_sorted(new_w);
                
                printf("Book recorded successfully.\n");
                break;
            }
            case 3: { // Search Author
                char sname[100];
                printf("Enter Surname: "); scanf("%99s", sname);
                
                int found = 0;
                for(int i=0; i<authorcounter; i++){
                    if(strcmp(authors_arr[i].surname, sname) == 0){
                        found = 1;
                        printf("\nFound: %s %s (ID: %d)\n", authors_arr[i].name, authors_arr[i].surname, authors_arr[i].writer_id);
                        printf("Books:\n");
                        int has_books = 0;
                        for(int j=0; j<writescounter; j++){
                            if(writes_arr[j].writer_id == authors_arr[i].writer_id){
                                // Βρίσκουμε τις λεπτομέρειες από τον book_arr
                                int book_idx = binary_search_book_title(books_arr, bookcounter, writes_arr[j].tittle);
                                if(book_idx != -1) {
                                    printf(" - %s (Year: %d, Price: %.2f)\n", 
                                           books_arr[book_idx].title, books_arr[book_idx].release_date, books_arr[book_idx].price);
                                    has_books = 1;
                                }
                            }
                        }
                        if(!has_books) printf(" - No books found.\n");
                    }
                }
                if(!found) printf("No author found with surname '%s'.\n", sname);
                break;
            }
            case 4: { // Search Book
                char title[200];
                printf("Enter Title: "); scanf("%199s", title);
                int idx = binary_search_book_title(books_arr, bookcounter, title);
                if(idx != -1){
                    printf("\nBook: %s\nYear: %d\nPrice: %.2f\nAuthors:\n", books_arr[idx].title, books_arr[idx].release_date, books_arr[idx].price);
                    
                    int found_author = 0;
                    for(int i=0; i<writescounter; i++){
                        if(strcmp(writes_arr[i].tittle, title) == 0){
                            int a_idx = binary_search_author_id(authors_arr, authorcounter, writes_arr[i].writer_id);
                            if(a_idx != -1){
                                printf(" - %s %s\n", authors_arr[a_idx].name, authors_arr[a_idx].surname);
                                found_author = 1;
                            }
                        }
                    }
                    if(!found_author) printf(" - Unknown author.\n");
                } else {
                    printf("Book not found.\n");
                }
                break;
            }
            case 5: { // Delete Author
                int id;
                printf("Enter Writer ID to delete: "); scanf("%d", &id);
                int a_idx = binary_search_author_id(authors_arr, authorcounter, id);
                
                if(a_idx == -1) {
                    printf("Author ID not found.\n");
                } else {
                    printf("Processing deletion for Author ID: %d...\n", id);
                    // Πρέπει να σβήσουμε βιβλία ΑΝ είναι μονογραφίες 
                    for(int i=0; i<writescounter; ) {
                        if(writes_arr[i].writer_id == id) {
                            char *curr_title = writes_arr[i].tittle;
                            
                            // Έλεγχος αν είναι μονογραφία
                            int count_authors = 0;
                            for(int j=0; j<writescounter; j++) {
                                if(strcmp(writes_arr[j].tittle, curr_title) == 0) count_authors++;
                            }
                            
                            if(count_authors == 1) { 
                                printf("Deleting monograph: %s\n", curr_title);
                                int book_idx = binary_search_book_title(books_arr, bookcounter, curr_title);
                                if(book_idx != -1) {
                                   free(books_arr[book_idx].title); 
                                    // Μετακίνηση στοιχείων προς τα αριστερά
                                    for(int k=book_idx; k<bookcounter-1; k++) {
                                        books_arr[k] = books_arr[k+1];
                                    }
                                    bookcounter--; 

                                    // Δυναμική αλλαγή μεγέθους ή free αν άδειασε
                                    if (bookcounter == 0) {
                                        free(books_arr);
                                        books_arr = NULL;
                                    } else {
                                        struct book *temp = realloc(books_arr, bookcounter * sizeof(struct book));
                                        if(temp != NULL) books_arr = temp;
                                    }
                                }
                            } else {
                                printf(" - Removing author from co-authored book: %s\n", curr_title);
                            }
                            
                            // Διαγραφή από writes_arr
                            free(writes_arr[i].tittle);
                            for(int k=i; k<writescounter-1; k++) {
                            writes_arr[k] = writes_arr[k+1];
                            }writescounter--;
                           if (writescounter == 0) {
                                free(writes_arr);
                                writes_arr = NULL;
                            } else {
                                struct writes *temp = realloc(writes_arr, writescounter * sizeof(struct writes));
                                if(temp != NULL) writes_arr = temp;
                            }
                            // Δεν κάνουμε i++, γιατί η επόμενη εγγραφή ήρθε στη θέση i
                        } else {
                            i++; // Προχωράμε μόνο αν δεν σβήσαμε
                        }
                    }
                    
                    // Διαγραφή από authors_arr
                    free(authors_arr[a_idx].name);
                    free(authors_arr[a_idx].surname);
                    for(int k=a_idx; k<authorcounter-1; k++) {
                        authors_arr[k] = authors_arr[k+1];
                    }
                    authorcounter--;

                    // Δυναμική αλλαγή μεγέθους ή free αν άδειασε
                    if (authorcounter == 0) {
                        free(authors_arr);
                        authors_arr = NULL;
                    } else {
                        struct author *temp = realloc(authors_arr, authorcounter * sizeof(struct author));
                        if(temp != NULL) authors_arr = temp;
                    }

                    printf("Author record deleted successfully.\n");
                }
                break;
            }
            case 6: { // delete book
                char title[200];
                printf("Enter Title to delete: "); scanf("%199s", title);
                int book_idx = binary_search_book_title(books_arr, bookcounter, title);
                
                if(book_idx == -1){
                    printf("Book not found.\n");
                } else {
                    printf("Deleting book: %s\n", title);
                    // Ενημέρωση writes_arr και num_of_books
                    for(int i=0; i<writescounter; ){
                        if(strcmp(writes_arr[i].tittle, title) == 0){
                            int w_id = writes_arr[i].writer_id;
                            // Βρίσκουμε τον συγγραφέα για να μειώσουμε τα βιβλία του
                            int a_idx = binary_search_author_id(authors_arr, authorcounter, w_id);
                            if(a_idx != -1) {
                                if(authors_arr[a_idx].num_of_books > 0)
                                    authors_arr[a_idx].num_of_books--;
                            }                
                            // Διαγραφή από writes_arr            
                            free(writes_arr[i].tittle);
                            for(int k=i; k<writescounter-1; k++) {
                                writes_arr[k] = writes_arr[k+1];
                            }
                            writescounter--;
                           // Δυναμική αλλαγή μεγέθους ή free αν άδειασε
                            if (writescounter == 0) {
                                free(writes_arr);
                                writes_arr = NULL;
                            } else {
                                struct writes *temp = realloc(writes_arr, writescounter * sizeof(struct writes));
                                if(temp != NULL) writes_arr = temp;
                            }
                            } else {
                            i++;
                            }
                        }
                    // Διαγραφή από book_arr
                    free(books_arr[book_idx].title);
                    for(int k=book_idx; k<bookcounter-1; k++) {
                        books_arr[k] = books_arr[k+1];
                    }
                    bookcounter--;
                    // Δυναμική αλλαγή μεγέθους ή free αν άδειασε
                    if (bookcounter == 0) {
                        free(books_arr);
                        books_arr = NULL;
                    } else {
                        struct book *temp = realloc(books_arr, bookcounter * sizeof(struct book));
                        if(temp != NULL) books_arr = temp;
                    }
                    
                    printf("Book record deleted successfully.\n");
                }
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

// Ταξινόμηση κατά ID 
void insert_author_sorted(struct author new_author) {
    if (authors_arr == NULL || authorcounter == 0) {
        // Δημιουργία πίνακα malloc 
        authors_arr = (struct author *)malloc(sizeof(struct author));
    } else {
        // Αλλαγή μεγέθους realloc 
        authors_arr = (struct author *)realloc(authors_arr, (authorcounter + 1) * sizeof(struct author));
    }

    if (authors_arr == NULL) { printf("Memory Error\n"); exit(1); } // Έλεγχος ασφαλείας

    int i = authorcounter - 1;
    while (i >= 0 && authors_arr[i].writer_id > new_author.writer_id) {
        authors_arr[i + 1] = authors_arr[i];
        i--;
    }
    authors_arr[i + 1] = new_author;
    authorcounter++;
}

// Ταξινόμηση κατά Title 
void insert_book_sorted(struct book new_book) {
    if (books_arr == NULL || bookcounter == 0) {
        // Δημιουργία πίνακα  malloc 
        books_arr = (struct book *)malloc(sizeof(struct book));
    } else {
        // Αλλαγή μεγέθους  realloc 
        books_arr = (struct book *)realloc(books_arr, (bookcounter + 1) * sizeof(struct book));
    }

    if (books_arr == NULL) { printf("Memory Error\n"); exit(1); }

    int i = bookcounter - 1;
    while (i >= 0 && strcmp(books_arr[i].title, new_book.title) > 0) {
        books_arr[i + 1] = books_arr[i];
        i--;
    }
    books_arr[i + 1] = new_book;
    bookcounter++;
}

// Ταξινόμηση κατά ID, μετά κατά Title 
void insert_writes_sorted(struct writes new_write) {
   if (writes_arr == NULL || writescounter == 0) {
        // Δημιουργία πίνακα malloc 
        writes_arr = (struct writes *)malloc(sizeof(struct writes));
    } else {
        // Αλλαγή μεγέθους realloc 
        writes_arr = (struct writes *)realloc(writes_arr, (writescounter + 1) * sizeof(struct writes));
    }

    if (writes_arr == NULL) {
         printf("Memory Error\n"); exit(1); 
        }

    int i = writescounter - 1;
    while (i >= 0) {
        if (writes_arr[i].writer_id > new_write.writer_id) {
            writes_arr[i + 1] = writes_arr[i];
        } else if (writes_arr[i].writer_id == new_write.writer_id && 
                   strcmp(writes_arr[i].tittle, new_write.tittle) > 0) {
            writes_arr[i + 1] = writes_arr[i];
        } else {
            break; 
        }
        i--;
    }
    writes_arr[i + 1] = new_write;
    writescounter++;
}

int binary_search_author_id(struct author *arr, int n, int id) {
    int left = 0, right = n - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (arr[mid].writer_id == id) return mid;
        if (arr[mid].writer_id < id) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

// Binary Search (Title) 
int binary_search_book_title(struct book *arr, int n, const char *title) {
    int left = 0, right = n - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = strcmp(arr[mid].title, title);
        if (cmp == 0) return mid;
        if (cmp < 0) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

// Linear Search 
int find_author_by_surname(struct author *arr, int n, const char *surname) {
    for (int i = 0; i < n; i++) {
        if (strcmp(arr[i].surname, surname) == 0) return i;
    }
    return -1;
}


void load_data() {
    FILE *fa = fopen("authors.txt", "r");
    if(fa) {
        int count;
        if(fscanf(fa, "%d", &count) == 1) { // Διάβασμα πλήθους στην αρχή
            for(int i=0; i<count; i++){
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
    if(fb) {
        int count;
        if(fscanf(fb, "%d", &count) == 1) {
            for(int i=0; i<count; i++){
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
    if(fw) {
        int count;
        if(fscanf(fw, "%d", &count) == 1) {
            for(int i=0; i<count; i++){
                struct writes w;
                char buff[200];
                fscanf(fw, "%s %d", buff, &w.writer_id);
                w.tittle = strdup(buff);
                insert_writes_sorted(w);
            }
        }
        fclose(fw);
    }
}

void save_data() {
    FILE *fa = fopen("authors.txt", "w"); 
    if(fa) {
        fprintf(fa, "%d\n", authorcounter); 
        for(int i=0; i<authorcounter; i++)
            fprintf(fa, "%d %s %s %d\n", authors_arr[i].writer_id, authors_arr[i].surname, authors_arr[i].name, authors_arr[i].num_of_books);
        fclose(fa);
    }

    FILE *fb = fopen("books.txt", "w");
    if(fb) {
        fprintf(fb, "%d\n", bookcounter);
        for(int i=0; i<bookcounter; i++)
            fprintf(fb, "%s %d %.2f\n", books_arr[i].title, books_arr[i].release_date, books_arr[i].price);
        fclose(fb);
    }

    FILE *fw = fopen("writes.txt", "w");
    if(fw) {
        fprintf(fw, "%d\n", writescounter);
        for(int i=0; i<writescounter; i++)
            fprintf(fw, "%s %d\n", writes_arr[i].tittle, writes_arr[i].writer_id);
        fclose(fw);
    }
}