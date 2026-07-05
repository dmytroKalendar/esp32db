#include <Arduino.h>
#include "esp_heap_caps.h"
#include "esp_system.h"

// This is a small database, which runs entirely on your esp32.
// For now this database supports:
//      - Adding new rows
//      - Reading existing data
// TODO: 
//      - Add partitioning of memory (define struct Partition with a pointer, define max number of partitions and min size of a partition)
//      - Deletion of rows
//      - Rows of custom sizes should be supported

/*
    Define number of columns including 'id'.
    'id' is always the first column of type INT'
    Default = 1
*/
#define NUM_COLUMNS 1

/*
    Lists all of the supported types
    //#DO_NOT_MODIFY
*/
enum  TYPE {
    SHORT,
    INT,
    DOUBLE,
    LONG,
    CHAR,
    STRING
};

/*
    Set the types of data stored in each column. 
    Available types: INT, DOUBLE, LONG, STRING[STRING_LEN]
    'id' is always the first column of type INT'
    Default: {INT}
*/
const enum TYPE COLUMN_TYPES[NUM_COLUMNS] = {INT};

/*
    Set the size of a STRING parameter in number of CHARs.
    Default: 32
    Note: each column of type STRING has the same size
*/
#define STRING_LEN 32

void printMemoryStats() {
    Serial.println("=== Memory Stats ===");

    // Heap memory info
    Serial.printf("Free Heap: %u bytes\n", esp_get_free_heap_size());
    Serial.printf("Largest Free Block: %u bytes\n", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    Serial.printf("Minimum Free Heap Ever: %u bytes\n", esp_get_minimum_free_heap_size());

    // Stack high water mark for the current task
    Serial.printf("Main Task Stack High Water Mark: %u bytes\n", uxTaskGetStackHighWaterMark(NULL));

    // PSRAM stats (if available)
    #if CONFIG_SPIRAM_USE_MALLOC
        Serial.printf("Free PSRAM: %u bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        Serial.printf("Largest Free PSRAM Block: %u bytes\n", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    #else
        Serial.println("PSRAM not available.");
    #endif

    Serial.println("====================\n");
}


unsigned char ROW_SIZE;
unsigned char COLUMN_SIZES[NUM_COLUMNS];


void* table;
short num_rows = 0;
int table_size = 0;

/*
    Prints entire table
*/
void print_table(){

}

/*
    Adds new row to the table
*/
void add_row(int n){
    if (table_size > (num_rows + 1)*ROW_SIZE){ // table has place for one more row
        // add new row
        void* new_row = table + num_rows*ROW_SIZE;
        *(int*) new_row = n;
        Serial.printf("Added row: %d \n", *(int*) new_row);
        num_rows++;
    } else{
        // do nothing
        return;
    }
}

/*
    Calculates the size of individual rows (in bytes) from the defined COLUMN_TYPES
*/
unsigned char calculate_row_size(){
    unsigned char row = 0;
    for (unsigned char i = 0; i < NUM_COLUMNS; i++){
        if (COLUMN_TYPES[i] == INT){
            row += 2;
            COLUMN_SIZES[i] = 2;
        } else if (COLUMN_TYPES[i] == DOUBLE){
            row += 8;            COLUMN_SIZES[i] = 8;
        } else if (COLUMN_TYPES[i] == LONG){
            row += 8;
            COLUMN_SIZES[i] = 8;
        } else if (COLUMN_TYPES[i] == CHAR){
            row += 1;
            COLUMN_SIZES[i] = 1;
        } else if (COLUMN_TYPES[i] == STRING){
            row += STRING_LEN * sizeof(char);
            COLUMN_SIZES[i] = STRING_LEN * sizeof(char);
        }
    }
    return row;
}

void setup() {
    Serial.begin(115200);
    delay(1000); // Allow time for serial monitor to start

    heap_caps_print_heap_info(MALLOC_CAP_8BIT);

    ROW_SIZE = calculate_row_size();

    unsigned int free_memory = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

    if (ROW_SIZE * 100 < free_memory){ // should be able to store at least 100 rows
        table = malloc(free_memory);
        if (table == NULL) {
            Serial.println("malloc failed!");
        }
        table_size = free_memory / 2;
    } else{
        pinMode(2,OUTPUT);
        digitalWrite(2,HIGH);
        delay(1000);
        digitalWrite(2,LOW);
        delay(1000000);
    }

    printMemoryStats();

}

int n = 0;
void loop() {

    add_row(n);
    n++;
    delay(1000);

}

/*
    RULES:
    1) >10% of memory should be always free to ensure that DB doesn't crash
*/
