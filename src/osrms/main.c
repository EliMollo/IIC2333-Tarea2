#include "../osrms_API/osrms_API.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE* memory_file;

int main(int argc, char const *argv[])
{
    if (argc < 2) {
        printf("Uso: %s  <memory_file>\n", argv[0]);
        return 1;
    }

    // montar la memoria
    os_mount((char *)argv[1]);

    //resto de instrucciones

    // os_ls_processes
    os_ls_processes();

    // os_exists
    int process_id = 91;
    char* file_name = "knowledg.jpg";

    if (os_exists(process_id, file_name)) {
        printf("\nExiste! Archivo: %s | ID: %d.\n", file_name, process_id);
    } else {
        printf("\nNo existe el archivo.\n");
    }

    // os_ls_files
    os_ls_files(139);

    // os_frame_bitmap
    os_frame_bitmap();

    // os_tp_bitmap
    os_tp_bitmap();

    // os_start_process
    // os_start_process(115, "hola");
    // os_start_process(45, "chao");

    // os_finish_process
    // os_finish_process(45);

    // os_write_file
    osrmsFile* file_to_write = os_open(139, "newfile.txt", 'w');
    if (file_to_write != NULL) {
        char* data = "archivo de prueba";
        int bytes_written = os_write_file(file_to_write, data);
        printf("\nBytes escritos: %d\n", bytes_written);

        os_close(file_to_write);
    } else {
        printf("Error abriendo el archivo\n");
    }

    // os_open
    osrmsFile* file = os_open(139, "aaaaa.gif", 'r');

    // os_read_file
    char* dest = (char*) calloc(1, 1024);
    os_read_file(file, dest);

    free(memory_file);

    

    return 0;

}