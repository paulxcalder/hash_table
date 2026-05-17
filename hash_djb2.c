#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

typedef struct Item {
    char *key;
    int value;
    char deleted;
} Item;

unsigned int capacity = 32;
unsigned int table_size = 0;
float load_factor = 0.6f;

unsigned int rehash_count = 0;
unsigned int collision_count = 0;

Item *rehash_table(Item *hash_table);

unsigned int djb2(const char* str, unsigned int length) {
    unsigned int hash = 5381;

    for (unsigned int i = 0; i < length; str++, i++)
    {
        hash = ((hash << 5) + hash) + (*str);
    }

    return hash;
}

void free_table(Item *hash_table, unsigned int cap){
    if(!hash_table){
        return;
    }

    for(int i = 0; i < cap; i++){
        if(hash_table[i].key){
            free(hash_table[i].key);
        }
    }
    free(hash_table);
}

Item *put(Item *hash_table, const char *key, const int value){
    char collision = 0;
    int first_deleted = -1;
    unsigned int count = 0;
    unsigned int idx = djb2(key, strlen(key)) & (capacity - 1);

    while(hash_table[idx].key){
        if(count++ >= capacity){
            break;
        }       
        else if(hash_table[idx].deleted && first_deleted == -1){
            first_deleted = idx;
        }
        else if(strcmp(hash_table[idx].key, key) == 0){
            hash_table[idx].value = value;
            //printf("OK\n");
            return hash_table;
        }
        
        idx = (idx + 1) & (capacity - 1);
        collision = 1;
    }

    if(collision){
        collision_count++;
    }
    
    if(first_deleted != -1){
        idx = first_deleted;
    }

    if(hash_table[idx].key){
        free(hash_table[idx].key);
    }

    hash_table[idx].key = (char*)malloc((strlen(key) + 1) * sizeof(char));

    if(!hash_table[idx].key){
        printf("Memory allocation error!\n");
        return hash_table;
    }

    strcpy(hash_table[idx].key, key);
    hash_table[idx].value = value;
    hash_table[idx].deleted = 0;
    table_size++;

    //printf("OK\n");

    if(table_size > load_factor * capacity){
        return rehash_table(hash_table);
    }
    return hash_table;
}

int get(Item *hash_table, const char *key, char *found){
    unsigned int count = 0;
    unsigned int idx = djb2(key, strlen(key)) & (capacity - 1);

    while(hash_table[idx].key){
        if((strcmp(hash_table[idx].key, key) == 0) && !hash_table[idx].deleted){
            *found = 1;
            return hash_table[idx].value;
        }
        else if(count++ >= capacity){
            break;
        }

        idx = (idx + 1) & (capacity - 1);
    }

    *found = 0;
    return 0;
}

int delete(Item *hash_table, const char *key, char *found){
    unsigned int count = 0;
    unsigned int idx = djb2(key, strlen(key)) & (capacity - 1);

    while(hash_table[idx].key){
        if((strcmp(hash_table[idx].key, key) == 0) && !hash_table[idx].deleted){
            hash_table[idx].deleted = 1;
            table_size--;
            *found = 1;
            return hash_table[idx].value;
        }
        else if(count++ >= capacity){
            break;
        }

        idx = (idx + 1) & (capacity - 1);
    }

    *found = 0;
    return 0;
}

Item *rehash_table(Item *hash_table){
    rehash_count++;
    unsigned int old_cap = capacity;
    capacity <<= 1;
    
    Item *temp = (Item*)calloc(capacity, sizeof(Item));
    if(!temp){
        capacity = old_cap;
        return hash_table;
    }
    
    table_size = 0;
    for(int i = 0; i < old_cap; i++){
        if(hash_table[i].key && !hash_table[i].deleted){
            temp = put(temp, hash_table[i].key, hash_table[i].value);
        }
    }

    free_table(hash_table, old_cap);
    return temp;
}

/*void test(unsigned int n){
    srand(time(NULL));

    Item *hash_table = NULL;
    char **keys = (char**)malloc(n * sizeof(char*));

    if(!keys){
        printf("Memory allocation error!\n");
        return;
    }

    char found;
    int value = 67521488;
    double t1, t2;
    clock_t start, end;
    load_factor = 0.1;
    unsigned long long memory_size = 0;

    double t_put_best = 10000000000.0;
    double t_get_best = 10000000000.0;
    unsigned int col_count_best = -1;
    unsigned long long best_mem_size = -1;
    double best_balance = 100000000000000000000.0;
    float load_put_best = 0.0;
    float load_get_best = 0.0;
    float load_col_best = 0.0;
    float load_mem_best = 0.0;
    float load_bal_best = 0.0;

    for(int i = 0; i < n; i++){
        keys[i] = (char*)malloc(8 * sizeof(char));

        if(!keys[i]){
            printf("Memory allocation error!\n");
            goto clean;
        }

        for(int j = 0; j < 7; j++){
            keys[i][j] = 33 + (rand() & 223);
        }
        keys[i][7] = '\0';
    }

    for(; load_factor <= 0.91; load_factor += 0.05){
        rehash_count = 0;
        collision_count = 0;
        free_table(hash_table, capacity);
        table_size = 0;
        capacity = 32;
        hash_table = (Item*)calloc(capacity, sizeof(Item));

        if(!hash_table){
            printf("Memory allocation error!\n");
            break;
        }   

        start = clock();
        for(int i = 0; i < n; i++){
            hash_table = put(hash_table, keys[i], value);
        }
        end = clock();
        t1 = (double)(end - start) / CLOCKS_PER_SEC;

        memory_size = capacity * sizeof(Item) + n * 8 * sizeof(char);

        start = clock();
        for(int i = 0; i < n; i++){
            get(hash_table, keys[i], &found);        
        }
        end = clock();
        t2 = (double)(end - start) / CLOCKS_PER_SEC;

        if(t1 <= t_put_best){
            t_put_best = t1;
            load_put_best = load_factor;
        }
        if(t2 <= t_get_best){
            t_get_best = t2;
            load_get_best = load_factor;
        }
        if(collision_count <= col_count_best){
            col_count_best = collision_count;
            load_col_best = load_factor;
        }
        if(memory_size < best_mem_size){
            best_mem_size = memory_size;
            load_mem_best = load_factor;
        }
        if(best_balance >= t1 + t2 + memory_size){
            best_balance = t1 + t2 + memory_size;
            load_bal_best = load_factor;
        }

        printf("load_factor=%0.2f\n", load_factor);
        printf("Put %d items time(sec): %.4f\n", n, t1);
        printf("Collision count: %u\n", collision_count);
        printf("Rehash count: %u\n", rehash_count);
        printf("Get %d items time(sec): %.4f\n", n, t2);
        printf("Memory size: ");
        if(memory_size >= 1024 * 1024 * 1024){
            printf("%.2f Gb\n", memory_size / (1024.0 * 1024.0 * 1024.0));
        }
        else if(memory_size >= 1024 * 1024){
            printf("%.2f Mb\n", memory_size / (1024.0 * 1024.0));
        }
        else if(memory_size >= 1024){
            printf("%.2f Kb\n", memory_size / 1024.0);
        }
        else{
            printf("%llu B\n", memory_size);
        }
        printf("\n");
    }

    printf("Best put time(sec): %.4f\n", t_put_best);
    printf("load_factor=%0.2f\n\n", load_put_best);
    
    printf("Best get time(sec): %.4f\n", t_get_best);
    printf("load_factor=%0.2f\n\n", load_get_best);

    printf("Best collision count: %u\n", col_count_best);
    printf("load_factor=%0.2f\n\n", load_col_best);

    printf("Best memory size: ");
    if(best_mem_size >= 1024 * 1024 * 1024){
        printf("%.2f Gb\n", best_mem_size / (1024.0 * 1024.0 * 1024.0));
    }
    else if(best_mem_size >= 1024 * 1024){
        printf("%.2f Mb\n", best_mem_size / (1024.0 * 1024.0));
    }
    else if(best_mem_size >= 1024){
        printf("%.2f Kb\n", best_mem_size / 1024.0);
    }
    else{
        printf("%llu B\n", best_mem_size);
    }
    printf("load_factor=%0.2f\n\n", load_mem_best);

    printf("Best balance (sum of get time, put time and memory): %.4f\n", best_balance);
    printf("load_factor=%0.2f\n\n", load_bal_best);

clean:
    for(int i = 0; i < n; i++){
        free(keys[i]);
    }
    free(keys);
    free_table(hash_table, capacity);
}*/

int main(){
    char operation[5];
    char key[100];
    int value;
    char found;
    Item *hash_table = (Item*)calloc(capacity, sizeof(Item));

    if(!hash_table){
        printf("Memory allocation error!\n");
        return 0;
    }

    while(1){
        printf("?>");
        scanf("%4s", operation);

        if(strcmp(operation, "put") == 0){
            if(scanf(" %99[^,], %d", key, &value) == 2){
                hash_table = put(hash_table, key, value);
            }
            else{
                printf("Incorrect argument!\n");
            }
        }

        else if(strcmp(operation, "get") == 0){
            if(scanf("%99s", key) == 1){
                value = get(hash_table, key, &found);
                if(found){
                    printf("%d\n", value);
                }
                else{
                    printf("Not found!\n");
                }
            }
            else{
                printf("Incorrect argument!\n");
            }
        }

        else if(strcmp(operation, "del") == 0){
            if(scanf("%99s", key) == 1){
                value = delete(hash_table, key, &found);
                if(found){
                    printf("%d\n", value);
                }
                else{
                    printf("Not found!\n");
                }
            }
            else{
                printf("Incorrect argument!\n");
            }
        }

        else if(strcmp(operation, "exit") == 0){
            break;
        }

        else{
            printf("Unknown operation!\n");
        }
    }

    //test(10000000);

    free_table(hash_table, capacity);
    return 0;
}